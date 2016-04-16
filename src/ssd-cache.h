#define DEBUG 0
/* ---------------------------ssd cache---------------------------- */

#define size_t	unsigned
#define off_t	unsigned
#define bool	unsigned char

typedef struct 
{
	unsigned	offset;
} SSDBufferTag;

typedef struct
{
	SSDBufferTag 	ssd_buf_tag;
	int 		ssd_buf_id;				// ssd buffer location in shared buffer
	unsigned 	ssd_buf_flag;
	unsigned	usage_count;
	int			next_freessd;
} SSDBufferDesc;

#define SSD_BUF_VALID 0x01
#define SSD_BUF_DIRTY 0x02

typedef struct SSDBufferHashBucket
{
	SSDBufferTag 			hash_key;
	int 				ssd_buf_id;
	struct SSDBufferHashBucket 	*next_item;
} SSDBufferHashBucket;

typedef struct
{
	int		n_usedssd;			// For eviction
	int		first_freessd;		// Head of list of free ssds
	int		last_freessd;		// Tail of list of free ssds
	int		next_victimssd;		// For CLOCK
} SSDBufferStrategyControl;

typedef enum
{
	CLOCK = 0,
	LRUOfBand,
	
} SSDEvictionStrategy;

extern SSDBufferDesc	*ssd_buffer_descriptors;
extern char		*ssd_buffer_blocks;
extern SSDBufferHashBucket	*ssd_buffer_hashtable;
extern SSDBufferStrategyControl *ssd_buffer_strategy_control;

#define GetSSDBufblockFromId(ssd_buf_id) ((void *) (ssd_buffer_blocks + ((unsigned) (ssd_buf_id)) * SSD_BUFFER_SIZE))
#define GetSSDBufHashBucket(hash_code) ((SSDBufferHashBucket *) (ssd_buffer_hashtable + (unsigned) (hash_code)))

extern void initSSDBuffer();
extern int read(unsigned offset);
extern int write(unsigned offset);

extern unsigned NSSDBuffers;
extern unsigned NSSDBufTables;
extern unsigned SSD_BUFFER_SIZE;
extern char	smr_device[100];
extern int *	smr_fd;
extern int *	ssd_fd;
