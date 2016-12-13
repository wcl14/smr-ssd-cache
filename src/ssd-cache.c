#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "ssd_buf_table.h"
#include "strategy/clock.h"
#include "strategy/lru.h"
#include "strategy/lruofband.h"
#include "strategy/most.h"
#include "strategy/scan.h"
#include "strategy/WA.h"
static SSDBufferDesc *SSDBufferAlloc(SSDBufferTag ssd_buf_tag, bool * found);
static void    *initStrategySSDBuffer(SSDEvictionStrategy strategy);
static SSDBufferDesc *getSSDStrategyBuffer(SSDBufferTag ssd_buf_tag, SSDEvictionStrategy strategy);
static void    *hitInSSDBuffer(SSDBufferDesc * ssd_buf_hdr, SSDEvictionStrategy strategy);
/*
 * init buffer hash table, strategy_control, buffer, work_mem
 */
void 
initSSDBuffer()
{
	initStrategySSDBuffer(EvictStrategy);
	initSSDBufTable(NSSDBufTables);

	ssd_buffer_strategy_control = (SSDBufferStrategyControl *) malloc(sizeof(SSDBufferStrategyControl));
	ssd_buffer_strategy_control->n_usedssd = 0;
	//printf("usedssd: %ld\n", ssd_buffer_strategy_control->n_usedssd);
	ssd_buffer_strategy_control->first_freessd = 0;
	ssd_buffer_strategy_control->last_freessd = NSSDBuffers - 1;

	ssd_buffer_descriptors = (SSDBufferDesc *) malloc(sizeof(SSDBufferDesc) * NSSDBuffers);
	SSDBufferDesc  *ssd_buf_hdr;
	long		i;
	ssd_buf_hdr = ssd_buffer_descriptors;
	for (i = 0; i < NSSDBuffers; ssd_buf_hdr++, i++) {
		ssd_buf_hdr->ssd_buf_flag = 0;
		ssd_buf_hdr->ssd_buf_id = i;
		ssd_buf_hdr->next_freessd = i + 1;
	}
	ssd_buffer_descriptors[NSSDBuffers - 1].next_freessd = -1;
	//ssd_buffer_strategy_control->n_usedssd = 0;
	hit_num = 0;
	//miss_num = 0;
	flush_ssd_blocks = 0;
	//flush_fifo_times = 0;

	//initStrategySSDBuffer(EvictStrategy);
}

void           *
flushSSDBuffer(SSDBufferDesc * ssd_buf_hdr)
{
	char		*ssd_buffer;
	int		returnCode;

	returnCode = posix_memalign(&ssd_buffer,512,sizeof(char)*BLCKSZ);
        if(returnCode < 0){
                printf("[ERROR] flushSSDBuffer():--------posix memalign\n");
                free(ssd_buffer);
                exit(-1);
        }
	returnCode = pread(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE);
	if (returnCode < 0) {
		printf("[ERROR] flushSSDBuffer():-------read from ssd: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE);
		exit(-1);
	}
	returnCode = smrwrite(smr_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_tag.offset);
	//returnCode = pwrite(smr_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_tag.offset);
	if (returnCode < 0) {
		printf("[ERROR] flushSSDBuffer():-------write to smr: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, ssd_buf_hdr->ssd_buf_tag.offset);
		exit(-1);
	}
	free(ssd_buffer);
	return NULL;
}

static SSDBufferDesc *
SSDBufferAlloc(SSDBufferTag ssd_buf_tag, bool * found)
{
	//printf("ssdbufferalloc offset: %lu\n", ssd_buf_tag.offset);
	SSDBufferDesc  *ssd_buf_hdr;
	unsigned long	ssd_buf_hash = ssdbuftableHashcode(&ssd_buf_tag);
	long		ssd_buf_id = ssdbuftableLookup(&ssd_buf_tag, ssd_buf_hash);

	if (ssd_buf_id >= 0) {
		hit_num++;
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buf_id];
		*found = 1;
		hitInSSDBuffer(ssd_buf_hdr, EvictStrategy);
		return ssd_buf_hdr;
	}
	//printf("test3\n");
	//ssd_buf_hdr = (SSDBufferDesc *) malloc(sizeof(SSDBufferDesc));
	//ssd_buf_hdr->ssd_buf_tag = ssd_buf_tag;
	//printf("-----------offset: %lu\n", ssd_buf_hdr->ssd_buf_tag.offset);
	//printf("%ld\n", ssd_buffer_strategy_control->n_usedssd);
	//miss_num++;
	ssd_buf_hdr = getSSDStrategyBuffer(ssd_buf_tag, EvictStrategy);
	//printf("test2\n");
	/*
	 * unsigned char old_flag = ssd_buf_hdr->ssd_buf_flag; SSDBufferTag
	 * old_tag = ssd_buf_hdr->ssd_buf_tag; if (DEBUG) printf("[INFO]
	 * SSDBufferAlloc(): old_flag&SSD_BUF_DIRTY=%d\n", old_flag &
	 * SSD_BUF_DIRTY); if (old_flag & SSD_BUF_DIRTY != 0) {
	 * flushSSDBuffer(ssd_buf_hdr); } if (old_flag & SSD_BUF_VALID != 0)
	 * { unsigned long old_hash = ssdbuftableHashcode(&old_tag);
	 * ssdbuftableDelete(&old_tag, old_hash); }
	 */
	ssdbuftableInsert(&ssd_buf_tag, ssd_buf_hash, ssd_buf_hdr->ssd_buf_id);
	ssd_buf_hdr->ssd_buf_flag &= ~(SSD_BUF_VALID | SSD_BUF_DIRTY);
	ssd_buf_hdr->ssd_buf_tag = ssd_buf_tag;
	*found = 0;
	return ssd_buf_hdr;
}

static void    *
initStrategySSDBuffer(SSDEvictionStrategy strategy)
{
	if (strategy == CLOCK)
		initSSDBufferForClock();
	else if (strategy == LRU)
		initSSDBufferForLRU();
	else if (strategy == LRUofBand)
		initSSDBufferForLRUofBand();
	else if (strategy == Most || strategy == Most_Dirty)
		initSSDBufferForMost();
	else if (strategy == SCAN)
		initSSDBufferForSCAN();
	else if (strategy == WA)
		initSSDBufferForWA();
}

static SSDBufferDesc *
getSSDStrategyBuffer(SSDBufferTag ssd_buf_tag, SSDEvictionStrategy strategy)
{
	//printf("ssd_cache_usedssd : %d\n", ssd_buffer_strategy_control->n_usedssd);
	//flush_fifo_times++;
	if (strategy == CLOCK)
		return getCLOCKBuffer();
	else if (strategy == LRU)
		return getLRUBuffer();
	else if (strategy == LRUofBand)
		return getLRUofBandBuffer(ssd_buf_tag);
	else if (strategy == Most)
		return getMostBuffer(ssd_buf_tag);
	else if (strategy == SCAN)
		return getSCANBuffer(ssd_buf_tag);
	else if (strategy == WA)
		return getWABuffer(ssd_buf_tag);
}

static void    *
hitInSSDBuffer(SSDBufferDesc * ssd_buf_hdr, SSDEvictionStrategy strategy)
{
	if (strategy == CLOCK)
		hitInCLOCKBuffer(ssd_buf_hdr);
	else if (strategy == LRU)
		hitInLRUBuffer(ssd_buf_hdr);
	else if (strategy == LRUofBand)
		hitInLRUofBandBuffer(ssd_buf_hdr);
	else if (strategy == Most || strategy == Most_Dirty)
		hitInMostBuffer();
	else if (strategy == SCAN)
		hitInSCANBuffer(ssd_buf_hdr);
	else if (strategy == WA)
		hitInWABuffer(ssd_buf_hdr);
}

/*
 * read--return the buf_id of buffer according to buf_tag
 */
void 
read_block(off_t offset, char *ssd_buffer)
{
	void           *ssd_buf_block;
	bool		found = 0;
	int		returnCode;

	static SSDBufferTag ssd_buf_tag;
	static SSDBufferDesc *ssd_buf_hdr;

	ssd_buf_tag.offset = offset;
	if (DEBUG)
		printf("[INFO] read():-------offset=%lu\n", offset);
	ssd_buf_hdr = SSDBufferAlloc(ssd_buf_tag, &found);
	if (found) {
		returnCode = pread(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE);
		if (returnCode < 0) {
			printf("[ERROR] read():-------read from smr: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
	} else {
		returnCode = smrread(smr_fd, ssd_buffer, SSD_BUFFER_SIZE, offset);
		//returnCode = pread(smr_fd, ssd_buffer, SSD_BUFFER_SIZE, offset);
		if (returnCode < 0) {
			printf("[ERROR] read():-------read from smr: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
		flush_ssd_blocks++;
		//returnCode = pwrite(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE);
		if (returnCode < 0) {
			printf("[ERROR] read():-------write to ssd: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
	}
	ssd_buf_hdr->ssd_buf_flag &= ~SSD_BUF_VALID;
	ssd_buf_hdr->ssd_buf_flag |= SSD_BUF_VALID;
}

/*
 * write--return the buf_id of buffer according to buf_tag
 */
void 
write_block(off_t offset, char *ssd_buffer)
{
	void           *ssd_buf_block;
	bool		found;
	int		returnCode;

	static SSDBufferTag ssd_buf_tag;
	static SSDBufferDesc *ssd_buf_hdr;

	ssd_buf_tag.offset = offset;
	if (DEBUG)
		printf("[INFO] write():-------offset=%lu\n", offset);
	ssd_buf_hdr = SSDBufferAlloc(ssd_buf_tag, &found);
	flush_ssd_blocks++;
    if (flush_ssd_blocks % 10000 == 0)
		printf("hit num:%lu   flush_ssd_blocks:%lu flush_fifo_times:%lu flush_fifo_blocks:%lu  flusd_bands:%lu\n ", hit_num, flush_ssd_blocks, flush_fifo_times, flush_fifo_blocks, flush_bands);
	returnCode = pwrite(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE);
	if (returnCode < 0) {
		printf("[ERROR] write():-------write to ssd: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
		exit(-1);
	}
	ssd_buf_hdr->ssd_buf_flag |= SSD_BUF_VALID | SSD_BUF_DIRTY;
}
void 
read_band(off_t offset, char *ssd_buffer)
{
	printf("enter read_band\n");
	void           *ssd_buf_block;
	bool		found = 0;
	int		returnCode;

	static SSDBufferTag ssd_buf_tag;
	static SSDBufferDesc *ssd_buf_hdr;

	ssd_buf_tag.offset = offset;
	//Band
		static SSDBufferTag band_tag;
	band_tag.offset = (offset / BNDSZ);
	static SSDBufferTag hdr_tag;
	hdr_tag.offset = (band_tag.offset) * BNDSZ;
	size_t		new_offset = offset - hdr_tag.offset;
	char           *band_buffer;
	band_buffer = (char *)malloc(sizeof(char) * BNDSZ);
	printf("readband_tag%ld\n", band_tag.offset);
	if (DEBUG)
		printf("[INFO] read():-------offset=%lu\n", offset);
	ssd_buf_hdr = SSDBufferAlloc(hdr_tag, &found);
	if (found) {
		returnCode = pread(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE + new_offset);
		if (returnCode < 0) {
			printf("[ERROR] read():-------read from smr: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
	} else {
		returnCode = smrread(smr_fd, band_buffer, BNDSZ, hdr_tag.offset);
		//returnCode = pread(smr_fd, ssd_buffer, SSD_BUFFER_SIZE, offset);
		if (returnCode < 0) {
			printf("[ERROR] read():-------read from smr: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
		flush_ssd_blocks++;
		//returnCode = pwrite(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * SSD_BUFFER_SIZE);
		/*
		 * if(returnCode < 0) {            printf("[ERROR]
		 * read():-------write to ssd: fd=%d, errorcode=%d,
		 * offset=%lu\n", ssd_fd, returnCode, offset); exit(-1); }
		 */
		returnCode = pwrite(ssd_fd, band_buffer, BNDSZ, ssd_buf_hdr->ssd_buf_id * BNDSZ);
		if (returnCode < 0) {
			printf("[ERROR] read():-------write to ssd: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
	}
	ssd_buf_hdr->ssd_buf_flag &= ~SSD_BUF_VALID;
	ssd_buf_hdr->ssd_buf_flag |= SSD_BUF_VALID;
}
void 
write_band(off_t offset, char *ssd_buffer)
{
	void           *ssd_buf_block;
	bool		found;
	int		returnCode;

	static SSDBufferTag ssd_buf_tag;
	static SSDBufferDesc *ssd_buf_hdr;

	ssd_buf_tag.offset = offset;
	static SSDBufferTag band_tag;
	band_tag.offset = (offset / BNDSZ);
	static SSDBufferTag hdr_tag;
	hdr_tag.offset = (band_tag.offset) * BNDSZ;
	size_t		new_offset = offset - hdr_tag.offset;
	char           *band_buffer;
	if (DEBUG)
		printf("[INFO] write():-------offset=%lu\n", offset);

	band_buffer = (char *)malloc(sizeof(char) * BNDSZ);
	ssd_buf_hdr = SSDBufferAlloc(ssd_buf_tag, &found);
	flush_ssd_blocks++;
	if (flush_ssd_blocks % 10000 == 0)
		printf("hit num:%lu   flush_ssd_blocks:%lu flush_fifo_times:%lu flush_fifo_blocks:%lu  flusd_bands:%lu\n ", hit_num, flush_ssd_blocks, flush_fifo_times, flush_fifo_blocks, flush_bands);
	if (found) {
		returnCode = pwrite(ssd_fd, ssd_buffer, SSD_BUFFER_SIZE, ssd_buf_hdr->ssd_buf_id * BNDSZ + new_offset);
	} else {
		returnCode = smrread(smr_fd, band_buffer, BNDSZ, hdr_tag.offset);

		if (returnCode < 0) {
			printf("[ERROR] write():-------write to ssd: fd=%d, errorcode=%d, offset=%lu\n", ssd_fd, returnCode, offset);
			exit(-1);
		}
		memcpy(band_buffer + new_offset, ssd_buffer, BLCKSZ);
	}
	ssd_buf_hdr->ssd_buf_flag |= SSD_BUF_VALID | SSD_BUF_DIRTY;

}
