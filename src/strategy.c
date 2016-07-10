#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator.h"
#include "strategy.h"

SSDBufferDesc *getCLOCKBuffer()
{
	SSDBufferDesc *ssd_buf_hdr;

	if (ssd_buffer_strategy_control->first_freessd >=0 ) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->first_freessd];
		ssd_buffer_strategy_control->first_freessd = ssd_buf_hdr->next_freessd;
		ssd_buf_hdr->next_freessd = -1;
        ssd_buffer_strategy_control->n_usedssd ++;
		return ssd_buf_hdr;
	}

	for (;;) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->next_victimssd];
		ssd_buffer_strategy_control->next_victimssd++;
		if (ssd_buffer_strategy_control->next_victimssd >= NSSDBuffers) {
			ssd_buffer_strategy_control->next_victimssd = 0;
		}
		if (ssd_buf_hdr->usage_count > 0) {
			ssd_buf_hdr->usage_count--;
		}
		else
			return ssd_buf_hdr;
	}
	
	return NULL;
}

static volatile void* addToLRUHead(SSDBufferDesc *ssd_buf_hdr)
{
    if (ssd_buffer_strategy_control->n_usedssd == 0) {
       ssd_buffer_strategy_control->first_lru = ssd_buf_hdr->ssd_buf_id;
       ssd_buffer_strategy_control->last_lru = ssd_buf_hdr->ssd_buf_id;
    } else {
        ssd_buf_hdr->next_lru = ssd_buffer_descriptors[ssd_buffer_strategy_control->first_lru].ssd_buf_id;
        ssd_buf_hdr->last_lru = -1;
        ssd_buffer_descriptors[ssd_buffer_strategy_control->first_lru].last_lru = ssd_buf_hdr->ssd_buf_id;
        ssd_buffer_strategy_control->first_lru = ssd_buf_hdr->ssd_buf_id;
    }
}
static volatile void* deleteFromLRU(SSDBufferDesc *ssd_buf_hdr)
{
    if (ssd_buf_hdr->last_lru >= 0) {
        ssd_buffer_descriptors[ssd_buf_hdr->last_lru].next_lru=ssd_buf_hdr->next_lru;
    } else {
        ssd_buffer_strategy_control->first_lru = ssd_buf_hdr->next_lru;
    }
    if (ssd_buf_hdr->next_lru >= 0 ) {
       ssd_buffer_descriptors[ssd_buf_hdr->next_lru].last_lru=ssd_buf_hdr->last_lru;
    } else {
        ssd_buffer_strategy_control->last_lru = ssd_buf_hdr->last_lru;
    }
}
static volatile void* moveToLRUHead(SSDBufferDesc *ssd_buf_hdr)
{
    deleteFromLRU(ssd_buf_hdr);
    addToLRUHead(ssd_buf_hdr);
}

SSDBufferDesc *getLRUBuffer()
{
	SSDBufferDesc *ssd_buf_hdr;
    
	if (ssd_buffer_strategy_control->first_freessd >=0 ) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->first_freessd];
		ssd_buffer_strategy_control->first_freessd = ssd_buf_hdr->next_freessd;
		ssd_buf_hdr->next_freessd = -1;
        addToLRUHead(ssd_buf_hdr);
        ssd_buffer_strategy_control->n_usedssd ++;
        return ssd_buf_hdr;
    }

    ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->last_lru];
    moveToLRUHead(ssd_buf_hdr);

    return ssd_buf_hdr;
}

void *hitInLRUBuffer(SSDBufferDesc *ssd_buf_hdr)
{
    moveToLRUHead(ssd_buf_hdr);
}

SSDBufferDesc *getLRUOfBandBuffer()
{
	return NULL;
}

