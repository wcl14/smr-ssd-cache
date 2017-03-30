#define DEBUG 0
/* ---------------------------maxcold_evict---------------------------- */

typedef struct
{
	SSDBufferTag 	ssd_buf_tag;
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
    long        next_freessd;           // to link free ssd
    unsigned long   hit_times;
} SSDBufferDescForMaxColdEvictHistory;

typedef struct
{
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
} SSDBufferDescForMaxColdEvictNow;

typedef struct
{
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
    long        first_freessd;     // Head of list of free ssds
    long        last_freessd;      // Tail of list of free ssds
    long        n_usedssds;
} SSDBufferStrategyControlForMaxColdEvictHistory;

typedef struct
{
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
    long        n_usedssds;
} SSDBufferStrategyControlForMaxColdEvictNow;

typedef struct
{
    unsigned long band_num;
    long current_hits;
    long current_pages;
    long to_sort;
} BandDescForMaxColdEvictHistory;

typedef struct
{
    unsigned char ischosen_toevict;
    unsigned char ischosen_tocache;
} BandDescForMaxColdEvictNow;

extern size_t   ZONESZ;
extern unsigned long NBANDTables;
extern unsigned long NSMRBands;
extern unsigned long PERIODTIMES;
extern unsigned long NCOLDBAND;
extern unsigned long run_times;
extern unsigned long flush_fifo_times;

SSDBufferHashBucket	*ssd_buffer_hashtable_history;

SSDBufferDescForMaxColdEvictHistory *ssd_buffer_descriptors_for_maxcold_evict_history;
SSDBufferDescForMaxColdEvictNow *ssd_buffer_descriptors_for_maxcold_evict_now;
SSDBufferStrategyControlForMaxColdEvictHistory *ssd_buffer_strategy_control_for_maxcold_evict_history;
SSDBufferStrategyControlForMaxColdEvictNow *ssd_buffer_strategy_control_for_maxcold_evict_now;
BandDescForMaxColdEvictHistory *band_descriptors_for_maxcold_evict_history;
BandDescForMaxColdEvictNow *band_descriptors_for_maxcold_evict_now;

extern void initSSDBufferForMaxColdEvict();
extern SSDBufferDesc *getMaxColdEvictBuffer(SSDBufferTag);
extern void *hitInMaxColdEvictBuffer(SSDBufferDesc *);
extern bool isCachedMaxColdEvict(SSDBufferTag);
