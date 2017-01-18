#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>

#include "ssd-cache.h"
#include "smr-simulator.h"
#include "inner_ssd_buf_table.h"

static SSDDesc *getStrategySSD();
static void    *freeStrategySSD();
static volatile void *flushSSD(SSDDesc * ssd_hdr);

/*
 * init inner ssd buffer hash table, strategy_control, buffer, work_mem
 */
void
initSSD()
{
	pthread_t	freessd_tid;
	int		err;

	initSSDTable(NSSDTables);

	ssd_strategy_control = (SSDStrategyControl *) malloc(sizeof(SSDStrategyControl));
	ssd_strategy_control->first_usedssd = 0;
	ssd_strategy_control->last_usedssd = -1;
	ssd_strategy_control->n_usedssd = 0;

	ssd_descriptors = (SSDDesc *) malloc(sizeof(SSDDesc) * NSSDs);
	SSDDesc        *ssd_hdr;
	long		i;
	ssd_hdr = ssd_descriptors;
	for (i = 0; i < NSSDs; ssd_hdr++, i++) {
		ssd_hdr->ssd_flag = 0;
		ssd_hdr->ssd_id = i;
	}
	interval_time = 0;

	pthread_mutex_init(&free_ssd_mutex, NULL);

	err = pthread_create(&freessd_tid, NULL, freeStrategySSD, NULL);
	if (err != 0) {
		printf("[ERROR] initSSD: fail to create thread: %s\n", strerror(err));
	}
	flush_bands = 0;
	flush_band_size = 0;
	flush_fifo_blocks = 0;
}

int
smrread(int smr_fd, char *buffer, size_t size, off_t offset)
{
	SSDTag		ssd_tag;
	SSDDesc        *ssd_hdr;
	long		i;
	int		returnCode;
	long		ssd_hash;
	long		ssd_id;

	for (i = 0; i * BLCKSZ < size; i++) {
		ssd_tag.offset = offset + i * BLCKSZ;
		ssd_hash = ssdtableHashcode(&ssd_tag);
		ssd_id = ssdtableLookup(&ssd_tag, ssd_hash);

		if (ssd_id >= 0) {
			ssd_hdr = &ssd_descriptors[ssd_id];
			returnCode = pread(inner_ssd_fd, buffer, BLCKSZ, ssd_hdr->ssd_id * BLCKSZ);
			if (returnCode < 0) {
				printf("[ERROR] smrread():-------read from inner ssd: fd=%d, errorcode=%d, offset=%lu\n", inner_ssd_fd, returnCode, ssd_hdr->ssd_id * BLCKSZ);
				exit(-1);
			}
		} else {
			returnCode = pread(smr_fd, buffer, BLCKSZ, offset + i * BLCKSZ);
			if (returnCode < 0) {
				printf("[ERROR] smrread():-------read from smr disk: fd=%d, errorcode=%d, offset=%lu\n", inner_ssd_fd, returnCode, offset + i * BLCKSZ);
				exit(-1);
			}
		}
	}

	return 0;
}

int
smrwrite(int smr_fd, char *buffer, size_t size, off_t offset)
{
	SSDTag		ssd_tag;
	SSDDesc        *ssd_hdr;
	long		i;
	int		returnCode;
	long		ssd_hash;
	long		ssd_id;

	for (i = 0; i * BLCKSZ < size; i++) {
		ssd_tag.offset = offset + i * BLCKSZ;
		ssd_hash = ssdtableHashcode(&ssd_tag);
		ssd_id = ssdtableLookup(&ssd_tag, ssd_hash);
		if (ssd_id >= 0) {
			ssd_hdr = &ssd_descriptors[ssd_id];
		} else {
			ssd_hdr = getStrategySSD();
		}

		ssdtableInsert(&ssd_tag, ssd_hash, ssd_hdr->ssd_id);
		ssd_hdr->ssd_flag |= SSD_VALID | SSD_DIRTY;
		ssd_hdr->ssd_tag = ssd_tag;
		flush_fifo_blocks++;
		returnCode = pwrite(inner_ssd_fd, buffer, BLCKSZ, ssd_hdr->ssd_id * BLCKSZ);
		if (returnCode < 0) {
			printf("[ERROR] smrwrite():-------write to smr disk: fd=%d, errorcode=%d, offset=%lu\n", inner_ssd_fd, returnCode, offset + i * BLCKSZ);
			exit(-1);
		}
	}

}

static SSDDesc *
getStrategySSD()
{
	SSDDesc        *ssd_hdr;

	while (ssd_strategy_control->n_usedssd >= NSSDs) {
		usleep(1);
		if (DEBUG)
			printf("[INFO] getStrategySSD():--------ssd_strategy_control->n_usedssd=%ld\n", ssd_strategy_control->n_usedssd);
	}
	/* allocatelock */
	pthread_mutex_lock(&free_ssd_mutex);
	ssd_strategy_control->last_usedssd = (ssd_strategy_control->last_usedssd + 1) % NSSDs;
	ssd_strategy_control->n_usedssd++;
	/* releaselock */
	pthread_mutex_unlock(&free_ssd_mutex);

	return &ssd_descriptors[ssd_strategy_control->last_usedssd];
}

static void    *
freeStrategySSD()
{
	long		i;

	while (1) {
		usleep(100);
		interval_time++;
		if ((interval_time > INTERVALTIMELIMIT && ssd_strategy_control->n_usedssd >= NSSDCLEAN) || ssd_strategy_control->n_usedssd >= NSSDLIMIT) {
			if (DEBUG) {
				printf("[INFO] freeStrategySSD():--------interval_time=%ld\n", interval_time);
				printf("[INFO] freeStrategySSD():--------ssd_strategy_control->n_usedssd=%lu ssd_strategy_control->first_usedssd=%ld\n", ssd_strategy_control->n_usedssd, ssd_strategy_control->first_usedssd);
			}
			/* allocatelock */
			pthread_mutex_lock(&free_ssd_mutex);
			interval_time = 0;
			for (i = ssd_strategy_control->first_usedssd; i < ssd_strategy_control->first_usedssd + NSSDCLEAN; i++) {
				if ((ssd_descriptors[i % NSSDs].ssd_flag & SSD_VALID) && (ssd_descriptors[i % NSSDs].ssd_flag & SSD_DIRTY)) {
					flushSSD(&ssd_descriptors[i % NSSDs]);
				}
				if (ssd_descriptors[i % NSSDs].ssd_flag & SSD_VALID) {
					SSDTag         *ssd_tag = &ssd_descriptors[i % NSSDs].ssd_tag;
					unsigned long	hash_code = ssdtableHashcode(ssd_tag);
					ssd_descriptors[i % NSSDs].ssd_flag &= !SSD_VALID;
					ssdtableDelete(ssd_tag, hash_code);
				}
			}
			ssd_strategy_control->first_usedssd = (ssd_strategy_control->first_usedssd + NSSDCLEAN) % NSSDs;
			ssd_strategy_control->n_usedssd -= NSSDCLEAN;
			/* releaselock */
			pthread_mutex_unlock(&free_ssd_mutex);
			if (DEBUG)
				printf("[INFO] freeStrategySSD():--------after clean\n");
		}
	}
}

static volatile void *
flushSSD(SSDDesc * ssd_hdr)
{
	long		i;
	unsigned long	actual_band_size;
	int		returnCode;
	char		buffer    [BLCKSZ];
	char           *band;
	unsigned long	BandNum = GetSMRBandNumFromSSD(ssd_hdr->ssd_tag.offset);
	off_t		Offset;

	long		band_size = GetSMRActualBandSizeFromSSD(ssd_hdr->ssd_tag.offset);
	off_t		band_offset = ssd_hdr->ssd_tag.offset - GetSMROffsetInBandFromSSD(ssd_hdr) * BLCKSZ;
	returnCode = posix_memalign(&band, 512, sizeof(char) * band_size);
	if (returnCode < 0) {
		printf("[ERROR] flushSSD():-------posix_memalign\n");
		exit(-1);
	}
	/* read whole band from smr */
	returnCode = pread(smr_fd, band, band_size, band_offset);
	if (returnCode < 0) {
		printf("[ERROR] flushSSD():---------read from smr: fd=%d, errorcode=%d, offset=%lu\n", smr_fd, returnCode, band_offset);
		exit(-1);
	}
	/* read cached pages from FIFO */
	for (i = ssd_strategy_control->first_usedssd; i < ssd_strategy_control->first_usedssd + ssd_strategy_control->n_usedssd; i++) {
		if (ssd_descriptors[i % NSSDs].ssd_flag & SSD_VALID && GetSMRBandNumFromSSD((&ssd_descriptors[i % NSSDs])->ssd_tag.offset) == BandNum) {
			ssd_descriptors[i % NSSDs].ssd_flag &= !SSD_DIRTY;
			Offset = GetSMROffsetInBandFromSSD(&ssd_descriptors[i % NSSDs]);
			returnCode = pread(inner_ssd_fd, band + Offset * BLCKSZ, BLCKSZ, ssd_descriptors[i % NSSDs].ssd_id * BLCKSZ);
			if (returnCode < 0) {
				printf("[ERROR] flushSSD():-------read from inner ssd: fd=%d, errorcode=%d, offset=%lu\n", inner_ssd_fd, returnCode, ssd_descriptors[i % NSSDs].ssd_id * BLCKSZ);
				exit(-1);
			}
		}
	}

	/* write whole band to smr */
	flush_bands++;
	flush_band_size += band_size;
	returnCode = pwrite(smr_fd, band, band_size, band_offset);
	if (returnCode < 0) {
		printf("[ERROR] flushSSD():-------write to smr: fd=%d, errorcode=%d, offset=%lu\n", inner_ssd_fd, returnCode, band_offset);
		exit(-1);
	}
	free(band);
}

unsigned long
GetSMRActualBandSizeFromSSD(unsigned long offset)
{
	long		band_size_num = BNDSZ / 1024 / 1024 / 2 + 1;
	long		num_each_size = NSMRBands / band_size_num;
	long		i        , size, total_size = 0;
	for (i = 0; i < band_size_num; i++) {
		size = BNDSZ / 2 + i * 1024 * 1024;
		if (total_size + size * num_each_size >= offset)
			return size;
		total_size += size * num_each_size;
	}

	return 0;
}

unsigned long
GetSMRBandNumFromSSD(unsigned long offset)
{
	long		band_size_num = BNDSZ / 1024 / 1024 / 2 + 1;
	long		num_each_size = NSMRBands / band_size_num;
	long		i        , size, total_size = 0;
	for (i = 0; i < band_size_num; i++) {
		size = BNDSZ / 2 + i * 1024 * 1024;
		if (total_size + size * num_each_size > offset)
			return num_each_size * i + (offset - total_size) / size;
		total_size += size * num_each_size;
	}

	return 0;
}

off_t
GetSMROffsetInBandFromSSD(SSDDesc * ssd_hdr)
{
	long		band_size_num = BNDSZ / 1024 / 1024 / 2 + 1;
	long		num_each_size = NSMRBands / band_size_num;
	long		i        , size, total_size = 0;
	unsigned long	offset = ssd_hdr->ssd_tag.offset;

	for (i = 0; i < band_size_num; i++) {
		size = BNDSZ / 2 + i * 1024 * 1024;
		if (total_size + size * num_each_size > offset)
			return (offset - total_size - (offset - total_size) / size * size) / BLCKSZ;
		total_size += size * num_each_size;
	}

	return 0;
}
