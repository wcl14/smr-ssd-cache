#ifndef SMR_SSD_CACHE_SCAN_H
#define SMR_SSD_CACHE_SCAN_H

#define DEBUG 0
/* ---------------------------scan---------------------------- */
#include <band_table.h>

typedef struct
{
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_scan;               // to link used ssd as SCAN
    long        last_scan;               // to link used ssd as SCAN
} SSDBufferDescForSCAN;

typedef struct
{
	long start;
    long        scan_ptr;        
} SSDBufferStrategyControlForSCAN;

SSDBufferDescForSCAN	*ssd_buffer_descriptors_for_scan;
SSDBufferStrategyControlForSCAN *ssd_buffer_strategy_control_for_scan;
BandHashBucket *band_hashtable;

extern unsigned long flush_fifo_times;
extern void initSSDBufferForSCAN();
extern SSDBufferDesc *getSCANBuffer();
extern void *hitInSCANBuffer(SSDBufferDesc *);
extern void insertByTag();
#endif
