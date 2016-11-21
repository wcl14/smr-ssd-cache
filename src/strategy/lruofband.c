#include <stdio.h>
#include <stdlib.h>
#include "../ssd-cache.h"
#include "../smr-simulator/smr-simulator.h"
#include "lruofband.h"
#include "band_table.h"

static volatile void* addToLRUofBandHead(SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband);
static volatile void* deleteFromLRUofBand(SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband);
static volatile void* moveToLRUofBandHead(SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband);
static volatile void* addToBand(SSDBufferTag ssd_buf_tag,long freessd);
static volatile void* getSSDBufferofBand(SSDBufferTag ssd_buf_tag);
//extern unsigned long NSMRBand

void initSSDBufferForLRUofBand()
{
	initBandTable(NBANDTables);	

	ssd_buffer_strategy_control_for_lruofband = (SSDBufferStrategyControlForLRUofBand *) malloc(sizeof(SSDBufferStrategyControlForLRUofBand));
	ssd_buffer_strategy_control_for_lruofband->first_lru = -1;
	ssd_buffer_strategy_control_for_lruofband->last_lru = -1;	
	
	SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband;
	ssd_buffer_descriptors_for_lruofband = (SSDBufferDescForLRUofBand *) malloc(sizeof(SSDBufferDescForLRUofBand)*NSSDBuffers);
	long i;
	ssd_buf_hdr_for_lruofband = ssd_buffer_descriptors_for_lruofband;
	for(i = 0;i < NSSDBuffers; ssd_buf_hdr_for_lruofband++,i++){
		ssd_buf_hdr_for_lruofband->ssd_buf_id = i;
		ssd_buf_hdr_for_lruofband->next_lru = -1;
		ssd_buf_hdr_for_lruofband->last_lru = -1;
		ssd_buf_hdr_for_lruofband->next_ssd_buf = -1;
	}
	flush_fifo_times = 0;
}

static volatile void* addToLRUofBandHead(SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband)
{
	//printf("enter fun: addtolruofbandhead\n");
	if(ssd_buffer_strategy_control->n_usedssd ==0){
		ssd_buffer_strategy_control_for_lruofband->first_lru = ssd_buf_hdr_for_lruofband->ssd_buf_id;
		ssd_buffer_strategy_control_for_lruofband->last_lru = ssd_buf_hdr_for_lruofband->ssd_buf_id;
	} else {
		ssd_buf_hdr_for_lruofband->next_lru = ssd_buffer_descriptors_for_lruofband[ssd_buffer_strategy_control_for_lruofband->first_lru].ssd_buf_id;
		ssd_buf_hdr_for_lruofband->last_lru = -1;
		ssd_buffer_descriptors_for_lruofband[ssd_buffer_strategy_control_for_lruofband->first_lru].last_lru = ssd_buf_hdr_for_lruofband->ssd_buf_id;
		ssd_buffer_strategy_control_for_lruofband->first_lru = ssd_buf_hdr_for_lruofband->ssd_buf_id;	
	}
	//printf("exit fun addtolruofbandhead\n");
	return NULL;
}

static volatile void* deleteFromLRUofBand(SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband)
{
//	printf("%ld\n",ssd_buf_hdr_for_lruofband->next_lru);

	//printf("enter fun: deletefromlruofband\n");
//	printf("%ld %ld\n",ssd_buf_hdr_for_lruofband->last_lru,ssd_buf_hdr_for_lruofband->next_lru);
	if(ssd_buf_hdr_for_lruofband->last_lru >= 0){
		//printf("%ld %ld\n",ssd_buf_hdr_for_lruofband->last_lru,ssd_buf_hdr_for_lruofband->next_lru);
		ssd_buffer_descriptors_for_lruofband[ssd_buf_hdr_for_lruofband->last_lru].next_lru = ssd_buf_hdr_for_lruofband->next_lru;
	} else {
//	printf("%ld\n",ssd_buf_hdr_for_lruofband->next_lru);
		ssd_buffer_strategy_control_for_lruofband->first_lru = ssd_buf_hdr_for_lruofband->next_lru;
	}
//	printf("test1\n");
	if(ssd_buf_hdr_for_lruofband->next_lru >= 0){
		ssd_buffer_descriptors_for_lruofband[ssd_buf_hdr_for_lruofband->next_lru].last_lru = ssd_buf_hdr_for_lruofband->last_lru;
	} else {
		ssd_buffer_strategy_control_for_lruofband->last_lru = ssd_buf_hdr_for_lruofband->last_lru;
	}

	return NULL;
}

static volatile void* addToBand(SSDBufferTag ssd_buf_tag,long first_freessd)
{
	//printf("enter fun: addTOband\n");
	long band_num = GetSMRBandNumFromSSD(ssd_buf_tag.offset);
	//printf("getsmrbandnumfromssd  band_num : %ld first_freessd:%ld\n",band_num,first_freessd);
	unsigned long band_hash = bandtableHashcode(band_num);
	long first_page = bandtableLookup(band_num,band_hash);
	//printf("first_page in hashtable:%ld\n",first_page);

	SSDBufferDescForLRUofBand *ssd_buf_for_lruofband;
	if(first_page >= 0){
		ssd_buf_for_lruofband = &ssd_buffer_descriptors_for_lruofband[first_page];
		SSDBufferDescForLRUofBand *new_ssd_buf_for_lruofband;
		new_ssd_buf_for_lruofband = &ssd_buffer_descriptors_for_lruofband[first_freessd];
		
		new_ssd_buf_for_lruofband->next_ssd_buf = ssd_buf_for_lruofband->next_ssd_buf;
		ssd_buf_for_lruofband->next_ssd_buf = first_freessd;
		//printf("ssd_buf_for_lruofband->next_ssd_buf : %ld\n",ssd_buf_for_lruofband->next_ssd_buf);
	}
	else {
		bandtableInsert(band_num,band_hash,first_freessd);
		ssd_buffer_descriptors_for_lruofband[first_freessd].next_ssd_buf = -1;
	}
	//printf("exit fun addtoband\n");
	return NULL;
}

static volatile void* getSSDBufferofBand(SSDBufferTag ssd_buf_tag)
{
	//printf("enter fun: getssdbufferofband\n");
	long band_num = GetSMRBandNumFromSSD(ssd_buf_tag.offset);
	//printf("getsmrbandnumfromssd  band_num: %ld\n",band_num);
	unsigned long band_hash = bandtableHashcode(band_num);
	long first_page = bandtableLookup(band_num,band_hash);	
	//printf("first_page in hashbucket: %ld\n",first_page);

	SSDBufferDesc *ssd_buf_hdr;
	SSDBufferDescForLRUofBand *ssd_buf_for_lruofband;
	SSDBufferTag old_tag;
	unsigned char old_flag;
	unsigned long old_hash;
		
	while(first_page >= 0){
		ssd_buf_for_lruofband = &ssd_buffer_descriptors_for_lruofband[first_page];
		ssd_buf_hdr = &ssd_buffer_descriptors[first_page];

		//printf("first_page: %ld\n",first_page);
		ssd_buf_hdr->next_freessd = ssd_buffer_strategy_control->first_freessd;
		ssd_buffer_strategy_control->first_freessd = first_page;
		first_page = ssd_buf_for_lruofband->next_ssd_buf;
		//ssd_buf_for_lruofband = &ssd_buffer_descriptors_for_lruofband[first_page];
		deleteFromLRUofBand(ssd_buf_for_lruofband);
		
		old_flag = ssd_buf_hdr->ssd_buf_flag;
		old_tag = ssd_buf_hdr->ssd_buf_tag;
		if(old_flag & SSD_BUF_DIRTY != 0) {
			flushSSDBuffer(ssd_buf_hdr);
		}
		if(old_flag & SSD_BUF_VALID != 0) {
			old_hash = ssdbuftableHashcode(&old_tag);
			ssdbuftableDelete(&old_tag, old_hash);	
		}
		ssd_buffer_strategy_control->n_usedssd--;
		//first_page = ssd_buf_for_lruofband->next_ssd_buf;
	}
	bandtableDelete(band_num,band_hash);
//	getLRUofBandBuffer(ssd_buf_hdr);
}

SSDBufferDesc *getLRUofBandBuffer(SSDBufferTag ssd_buf_tag)
{
	//printf("n_usedssd : %ld\n",ssd_buffer_strategy_control->n_usedssd);
	//printf("enter fun: getLRUofBandBuffer\n");
	//printf("offset : %lu\n",ssd_buf_hdr->ssd_buf_tag.offset);
	SSDBufferDesc *ssd_buffer_hdr;
	SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband;
	//printf("ssd_buffer_strategy_control->first_freessd: %ld\n",ssd_buffer_strategy_control->first_freessd);
	//printf("last_lru:%ld\n",ssd_buffer_strategy_control_for_lruofband->last_lru);
	if(ssd_buffer_strategy_control->first_freessd < 0){
		flush_fifo_times ++ ;
		ssd_buffer_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control_for_lruofband->last_lru];
		getSSDBufferofBand(ssd_buffer_hdr->ssd_buf_tag);
	}
	//if(ssd_buffer_strategy_control->first_freessd >= 0){
		ssd_buffer_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->first_freessd];
		ssd_buf_hdr_for_lruofband = &ssd_buffer_descriptors_for_lruofband[ssd_buffer_strategy_control->first_freessd];
	//	printf("%ld\n",ssd_buffer_strategy_control->first_freessd);
	//	printf("offset : %lu\n",ssd_buf_hdr->ssd_buf_tag);
		ssd_buffer_hdr->ssd_buf_tag = ssd_buf_tag;
		addToBand(ssd_buf_tag,ssd_buffer_strategy_control->first_freessd);

		ssd_buffer_strategy_control->first_freessd = ssd_buffer_hdr->next_freessd;
		//printf("ssd_buffer_strategy_control->first_freessd: %ld\n",ssd_buffer_strategy_control->first_freessd);
		ssd_buffer_hdr->next_freessd = -1;
		addToLRUofBandHead(ssd_buf_hdr_for_lruofband);
//		printf("n_usedssd : %ld\n",ssd_buffer_strategy_control->n_usedssd);
		ssd_buffer_strategy_control->n_usedssd++;			
		//printf("test1\n");
	//	printf("ssd_buffer_hdr->ssd_buf_id: %ld\n",ssd_buffer_hdr->ssd_buf_id);
		return ssd_buffer_hdr;	
//	}
}

static volatile void* moveToLRUofBandHead(SSDBufferDescForLRUofBand *ssd_buf_hdr_for_lruofband)
{
	//printf("enter fun: moveToLRUofBandHead\n");
	deleteFromLRUofBand(ssd_buf_hdr_for_lruofband);
	addToLRUofBandHead(ssd_buf_hdr_for_lruofband);
}

void hitInLRUofBandBuffer(SSDBufferDesc *ssd_buf_hdr)
{
	//printf("enter fun: hitInLRUofBandBuffer\n");
	moveToLRUofBandHead(&ssd_buffer_descriptors_for_lruofband[ssd_buf_hdr->ssd_buf_id]);
}
