#define DEBUG 0
/* ---------------------------smr simulator---------------------------- */
#include <pthread.h>

typedef struct
{
        unsigned        offset;
} SSDTag;

typedef struct
{
        SSDTag     ssd_tag;
        long       ssd_id;			// ssd buffer location 
        unsigned   ssd_flag;
//	long		usage_count;
//	long		next_freessd;
} SSDDesc;

#define SSD_VALID 0x01
#define SSD_DIRTY 0x02

typedef struct SSDHashBucket
{
        SSDTag				hash_key;
        long                             ssd_id;
        struct SSDHashBucket		*next_item;
} SSDHashBucket;

typedef struct
{
	long		n_usedssd;
	long		first_usedssd;		// Head of list of used ssds
	long		last_usedssd;		// Tail of list of used ssds
} SSDStrategyControl;

extern SSDDesc		*ssd_descriptors;
extern char             *ssd_blocks;
extern SSDStrategyControl *ssd_strategy_control;
extern SSDHashBucket	*ssd_hashtable;

#define GetSSDblockFromId(ssd_id) ((void *) (ssd_blocks + ((unsigned) (ssd_id)) * SSD_SIZE))
#define GetSSDHashBucket(hash_code) ((SSDHashBucket *) (ssd_hashtable + (unsigned) (hash_code)))

extern int smrread(int smr_fd, char* buffer, size_t size, off_t offset);
extern int smrwrite(int smr_fd, char* buffer, size_t size, off_t offset);

extern unsigned long NSSDs;
extern unsigned long NSSDTables;
extern unsigned long SSD_SIZE;
extern size_t BLCKSZ;
extern size_t BNDSZ;
extern unsigned long INTERVALTIMELIMIT;
extern unsigned	long NSSDLIMIT;
extern unsigned long NSSDCLEAN;
extern char     smr_device[100];
extern char	inner_ssd_device[100];
extern int 	inner_ssd_fd;
extern int 	smr_fd;
extern unsigned	long interval_time;
extern pthread_mutex_t free_ssd_mutex;
extern pthread_mutex_t inner_ssd_hdr_mutex;
extern pthread_mutex_t inner_ssd_hash_mutex;

