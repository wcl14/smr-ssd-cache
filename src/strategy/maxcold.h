#define DEBUG 0
/* ---------------------------maxcold---------------------------- */

typedef struct
{
	SSDBufferTag 	ssd_buf_tag;
    unsigned    ssd_buf_flag;
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
    long        next_freessd;           // to link free ssd
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
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
} SSDBufferDescForLRURead;

typedef struct
{
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
    long        first_freessd;     // Head of list of free ssds
    long        last_freessd;      // Tail of list of free ssds
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
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
    long        first_freessd;     // Head of list of free ssds
    long        last_freessd;      // Tail of list of free ssds
    long        n_usedssds;
} SSDBufferStrategyControlForLRURead;

typedef struct
{
    unsigned long band_num;
    long current_hits;
    long current_pages;
    long current_cold_pages;
    long current_inssd_pages;
    long to_sort;
} BandDescForMaxColdHistory;

typedef struct
{
    unsigned char ischosen;
} BandDescForMaxColdNow;

extern size_t   ZONESZ;
extern unsigned long NBANDTables;
extern unsigned long NSMRBands;
extern unsigned long PERIODTIMES;
extern unsigned long NCOLDBAND;
extern unsigned long run_times;
extern unsigned long flush_fifo_times;

SSDBufferHashBucket	*ssd_buffer_hashtable_history;

SSDBufferDescForMaxColdHistory *ssd_buffer_descriptors_for_maxcold_history;
SSDBufferDescForMaxColdNow *ssd_buffer_descriptors_for_maxcold_now;
SSDBufferDescForLRURead *ssd_buffer_descriptors_for_lru_read;
SSDBufferStrategyControlForMaxColdHistory *ssd_buffer_strategy_control_for_maxcold_history;
SSDBufferStrategyControlForMaxColdNow *ssd_buffer_strategy_control_for_maxcold_now;
SSDBufferStrategyControlForLRURead *ssd_buffer_strategy_control_for_lru_read;
BandDescForMaxColdHistory *band_descriptors_for_maxcold_history;
BandDescForMaxColdNow *band_descriptors_for_maxcold_now;

extern void initSSDBufferForMaxColdSplitRW();
extern SSDBufferDesc *getMaxColdBufferSplitRW(SSDBufferTag, bool);
extern void *hitInMaxColdBufferSplitRW(SSDBufferDesc *, bool);
