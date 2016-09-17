#ifndef SMR_SSD_CACHE_LRU_H
#define SMR_SSD_CACHE_LRU_H

#define DEBUG 0
/* ---------------------------lru---------------------------- */

typedef struct
{
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
} SSDBufferDescForLRU;

typedef struct
{
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
} SSDBufferStrategyControlForLRU;

SSDBufferDescForLRU	*ssd_buffer_descriptors_for_lru;
SSDBufferStrategyControlForLRU *ssd_buffer_strategy_control_for_lru;

extern void initSSDBufferForLRU();
extern SSDBufferDesc *getLRUBuffer();
extern void *hitInLRUBuffer(SSDBufferDesc *);

#endif
