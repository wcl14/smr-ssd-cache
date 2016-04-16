#include <stdio.h>
#include <stdlib.h>
//#include <system.h>
#include "ssd-cache.h"
#include "smr-simulator.h"

static SSDDesc *getStrategySSD();
static void freeStrategySSD();
static volatile void flushSSD(SSDDesc *ssd_hdr);
static unsigned GetSMRBandNumFromSSD(SSDDesc *ssd_hdr);
static unsigned GetSMROffsetInBandFromSSD(SSDDesc *ssd_hdr);

/*
 * init inner ssd buffer hash table, strategy_control, buffer, work_mem
 */
void initSSD()
{
	initSSDTable(NSSDTables);

	ssd_strategy_control = (SSDStrategyControl *) malloc(sizeof(SSDStrategyControl));
	ssd_strategy_control->first_usedssd = 0;
	ssd_strategy_control->last_usedssd = -1;
	ssd_strategy_control->n_usedssd = 0;

	ssd_descriptors = (SSDDesc *) malloc(sizeof(SSDDesc)*NSSDs);
	SSDDesc *ssd_hdr;
	int i;
	ssd_hdr = ssd_descriptors;
	for (i = 0; i < NSSDs; ssd_hdr++, i++) {
		ssd_hdr->ssd_flag = 0;
		ssd_hdr->ssd_id = i;
//		ssd_hdr->usage_count = 0;
//		ssd_hdr->next_freessd = i + 1;
        }
//	ssd_descriptors[NSSDs - 1].next_freessd = -1;
	interval_time = 0;

	ssd_blocks = (char *) malloc(SSD_SIZE*NSSDs);
	memset(ssd_blocks, 0, SSD_SIZE*NSSDs);

}

int smrread(int smr_fd, char* buffer, size_t size, off_t offset)
{
	SSDTag ssd_tag;
	SSDDesc *ssd_hdr;
	int i, returnCode;
	unsigned ssd_hash;
	unsigned ssd_id;
	
	for (i = 0; i * BLCKSZ < size; i++) {
		ssd_tag.offset = offset + i * BLCKSZ;
		ssd_hash = ssdtableHashcode(&ssd_tag);
		ssd_id = ssdtableLookup(&ssd_tag, ssd_hash);

		if (ssd_id >= 0) {
			ssd_hdr = &ssd_descriptors[ssd_id];
			returnCode = pread(*inner_ssd_fd, buffer, BLCKSZ, ssd_hdr->ssd_id * BLCKSZ);
		        if(returnCode < 0) {
        		        printf("[ERROR] smrread():-------read from inner ssd: fd=%d, errorcode=%d, offset=%d\n", returnCode, ssd_hdr->ssd_id * BLCKSZ);
                		exit(-1);
	        	}
	
			return returnCode;
		} else {
			returnCode = pread(smr_fd, buffer, BLCKSZ, offset + i * BLCKSZ);
			if(returnCode < 0) {
        			printf("[ERROR] smrread():-------read from smr disk: fd=%d, errorcode=%d, offset=%d\n", returnCode, offset + i * BLCKSZ);
		                exit(-1);
        		}
		}
	}
	
	return 0;
}

int smrwrite(int smr_fd, char* buffer, size_t size, off_t offset)
{
	SSDTag ssd_tag;
	SSDDesc *ssd_hdr;
	int i, returnCode;
	unsigned ssd_hash;
	int ssd_id;

	for (i = 0; i * BLCKSZ < size; i++) {
		ssd_tag.offset = offset + i * BLCKSZ;
		ssd_hash = ssdtableHashcode(&ssd_tag);
		ssd_id = ssdtableLookup(&ssd_tag, ssd_hash);
		if (ssd_id >= 0) {
			ssd_hdr = &ssd_descriptors[ssd_id];
		}
		else {
			ssd_hdr = getStrategySSD();
			//releaselock
		}

		ssdtableInsert(&ssd_tag, ssd_hash, ssd_hdr->ssd_id);
		ssd_hdr->ssd_flag |= SSD_VALID | SSD_DIRTY;
		ssd_hdr->ssd_tag = ssd_tag;
		
		returnCode = pwrite(*inner_ssd_fd, buffer, BLCKSZ, ssd_hdr->ssd_id * BLCKSZ);
		if(returnCode < 0) {
        		printf("[ERROR] smrread():-------read from smr disk: fd=%d, errorcode=%d, offset=%d\n", returnCode, offset + i * BLCKSZ);
		               exit(-1);
        	}
		
	}

}

static SSDDesc *getStrategySSD()
{
	SSDDesc *ssd_hdr;

	while (ssd_strategy_control->n_usedssd >= NSSDs)
	{
		usleep(1);
		if (DEBUG) printf("[INFO] getStrategySSD():--------ssd_strategy_control->n_usedssd=%d\n", ssd_strategy_control->n_usedssd);	
	}
	//allocatelock
	ssd_strategy_control->last_usedssd = (ssd_strategy_control->last_usedssd + 1) % NSSDs;
	ssd_strategy_control->n_usedssd++;
	
	return &ssd_descriptors[ssd_strategy_control->last_usedssd];
}

static void freeStrategySSD()
{
	int i;

	while (1) {
		interval_time++;
		if (interval_time > INTERVALTIMELIMIT || ssd_strategy_control->n_usedssd > NSSDLIMIT) {
			//allocatelock
			interval_time = 0;
			for (i = ssd_strategy_control->first_usedssd; i < ssd_strategy_control->first_usedssd + NSSDCLEAN; i++) {
				if (ssd_descriptors[i%NSSDs].ssd_flag & SSD_VALID) {
					flushSSD(&ssd_descriptors[i%NSSDs]);
				} 
				ssdtableDelete(&ssd_descriptors[i%NSSDs].ssd_tag, ssd_descriptors[i%NSSDs].ssd_id);
		                ssd_descriptors[i%NSSDs].ssd_flag &= ~(SSD_VALID | SSD_DIRTY);	
			}
			ssd_strategy_control->first_usedssd = (ssd_strategy_control->first_usedssd + NSSDCLEAN) % NSSDs;
			ssd_strategy_control->n_usedssd -= NSSDCLEAN;
			//releaselock
		}
	}
}

static volatile void flushSSD(SSDDesc *ssd_hdr)
{
	int i, returnCode;
	char* buffer;
	char* band[BNDSZ/BLCKSZ];
	bool bandused[BNDSZ/BLCKSZ];
	unsigned BandNum = GetSMRBandNumFromSSD(ssd_hdr);
	unsigned Offset;

	memset(bandused, 0, BNDSZ/BLCKSZ*sizeof(bool));
	for (i = 0; i < BNDSZ/BLCKSZ; i++)
		band[i] = (char *) malloc(sizeof(char)*BLCKSZ);
	returnCode = pread(*inner_ssd_fd, band[GetSMROffsetInBandFromSSD(ssd_hdr)], BLCKSZ, ssd_hdr->ssd_id * BLCKSZ);
	if(returnCode < 0) {
		printf("[ERROR] flushSSD():-------read from inner ssd: fd=%d, errorcode=%d, offset=%d\n", returnCode, ssd_hdr->ssd_id * BLCKSZ);
                exit(-1);
	}

	for (i = ssd_strategy_control->first_usedssd; i < ssd_strategy_control->n_usedssd; i++)
	{
		if (GetSMRBandNumFromSSD(&ssd_descriptors[i]) == BandNum) {
			Offset = GetSMROffsetInBandFromSSD(&ssd_descriptors[i]);
			returnCode = pread(*inner_ssd_fd, band[Offset], BLCKSZ, ssd_descriptors[i].ssd_id * BLCKSZ);
			if(returnCode < 0) {
				printf("[ERROR] flushSSD():-------read from inner ssd: fd=%d, errorcode=%d, offset=%d\n", returnCode, ssd_descriptors[i].ssd_id * BLCKSZ);
		                exit(-1);
			}
			bandused[Offset] = 1;
			ssd_descriptors[i].ssd_flag = 0;
		}
	}
	
	for (i = 0; i < BNDSZ/BLCKSZ; i++)
	{
		if (bandused[i] == 0) {
			returnCode = pread(*smr_fd, band[i], BLCKSZ, (BandNum * BNDSZ / BLCKSZ + i) * BLCKSZ);
			if(returnCode < 0) {
				printf("[ERROR] flushSSD():-------read from smr: fd=%d, errorcode=%d, offset=%d\n", returnCode, (BandNum * BNDSZ / BLCKSZ + i) * BLCKSZ);
		                exit(-1);
			}
		}
	}

	returnCode = pwrite(*smr_fd, band, BNDSZ, BandNum * BNDSZ);
	if(returnCode < 0) {
		printf("[ERROR] flushSSD():-------write to smr: fd=%d, errorcode=%d, offset=%d\n", returnCode, BandNum * BNDSZ);
                exit(-1);
	}
}


static unsigned GetSMRBandNumFromSSD(SSDDesc *ssd_hdr)
{
	return ssd_hdr->ssd_tag.offset / BNDSZ;
}

static unsigned GetSMROffsetInBandFromSSD(SSDDesc *ssd_hdr)
{
	return (ssd_hdr->ssd_tag.offset / BLCKSZ) % (BNDSZ / BLCKSZ);
}
