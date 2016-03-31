#define DEBUG 0
/* ---------------------------smr simulator---------------------------- */

typedef struct
{
        unsigned        offset;
} SSDTag;

typedef struct
{
        SSDTag    	ssd_tag;
        int             ssd_id;			// ssd buffer location 
        unsigned        ssd_flag;
//	int		usage_count;
//	int		next_freessd;
} SSDDesc;

#define SSD_VALID 0x01
#define SSD_DIRTY 0x02

typedef struct SSDHashBucket
{
        SSDTag				hash_key;
        int                             ssd_id;
        struct SSDHashBucket		*next_item;
} SSDHashBucket;

typedef struct
{
	int		n_usedssd;
	int		first_usedssd;		// Head of list of used ssds
	int		last_usedssd;		// Tail of list of used ssds
} SSDStrategyControl;

extern SSDDesc		*ssd_descriptors;
extern char             *ssd_blocks;
extern SSDStrategyControl *ssd_strategy_control;
extern SSDHashBucket	*ssd_hashtable;

#define GetSSDblockFromId(ssd_id) ((void *) (ssd_blocks + ((unsigned) (ssd_id)) * SSD_SIZE))
#define GetSSDHashBucket(hash_code) ((SSDHashBucket *) (ssd_hashtable + (unsigned) (hash_code)))

extern int smrread(int smr_fd, char* buffer, size_t size, off_t offset);
extern int smrwrite(int smr_fd, char* buffer, size_t size, off_t offset);

extern unsigned NSSDs;
extern unsigned NSSDTables;
extern unsigned SSD_SIZE;
extern unsigned BLCKSZ;
extern unsigned BNDSZ;
extern unsigned INTERVALTIMELIMIT;
extern unsigned	NSSDLIMIT;
extern unsigned NSSDCLEAN;
extern char     smr_device[100];
extern char	inner_ssd_device[100];
extern int *	inner_ssd_fd;
extern int *	smr_fd;
extern unsigned	interval_time;
