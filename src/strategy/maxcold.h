#define DEBUG 0
/* ---------------------------maxcold---------------------------- */
#include <band_table.h>

typedef struct
{
	SSDBufferTag 	ssd_buf_tag;
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
    unsigned long   hit_times;
} SSDBufferDescForMaxColdHistory;

typedef struct
{
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
} SSDBufferDescForMaxColdNow;

typedef struct
{
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
    long        n_usedssds;
} SSDBufferStrategyControlForMaxColdHistory;

typedef struct
{
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
    long        n_usedssds;
} SSDBufferStrategyControlForMaxColdNow;

typedef struct
{
    unsigned long band_num;
    unsigned long current_cold_pages;
} BandDescForMaxColdHistory;

typedef struct
{
    unsigned char ischosen;
} BandDescForMaxColdNow;

extern unsigned long NBANDTables;
extern unsigned long NSMRBands;
extern unsigned long PERIODTIMES;
extern unsigned long NCOLDBAND;
extern unsigned long run_times;
extern unsigned long flush_fifo_times;

SSDBufferHashBucket	*ssd_buffer_hashtable_history;

SSDBufferDescForMaxColdHistory *ssd_buffer_descriptors_for_maxcold_history;
SSDBufferDescForMaxColdNow *ssd_buffer_descriptors_for_maxcold_now;
SSDBufferStrategyControlForMaxColdHistory *ssd_buffer_strategy_control_for_maxcold_history;
SSDBufferStrategyControlForMaxColdNow *ssd_buffer_strategy_control_for_maxcold_now;
BandDescForMaxColdHistory *band_descriptors_for_maxcold_history;
BandDescForMaxColdNow *band_descriptors_for_maxcold_now;

extern void initSSDBufferForMaxCold();
extern SSDBufferDesc *getMaxColdBuffer();
extern void *hitInMaxColdBuffer(SSDBufferDesc *);
