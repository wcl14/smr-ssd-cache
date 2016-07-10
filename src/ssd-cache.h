#define DEBUG 0
/* ---------------------------ssd cache---------------------------- */

#include <pthread.h>
//#define size_t	unsigned long
#define off_t	unsigned long
#define bool	unsigned char

typedef struct 
{
	off_t	offset;
} SSDBufferTag;

typedef struct
{
	SSDBufferTag 	ssd_buf_tag;
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
	unsigned 	ssd_buf_flag;
	unsigned long	usage_count;
	long		next_freessd;           // to link free ssd
    long        next_lru;               // to link used ssd as LRU
    long        last_lru;               // to link used ssd as LRU
} SSDBufferDesc;

#define SSD_BUF_VALID 0x01
#define SSD_BUF_DIRTY 0x02

typedef struct SSDBufferHashBucket
{
	SSDBufferTag 			hash_key;
	long    				ssd_buf_id;
	struct SSDBufferHashBucket 	*next_item;
} SSDBufferHashBucket;

typedef struct
{
	long		n_usedssd;			// For eviction
	long		first_freessd;		// Head of list of free ssds
	long		last_freessd;		// Tail of list of free ssds
    long        first_lru;          // Head of list of LRU
    long        last_lru;           // Tail of list of LRU
	long		next_victimssd;		// For CLOCK
} SSDBufferStrategyControl;

typedef enum
{
	CLOCK = 0,
    LRU,
	LRUOfBand,
	
} SSDEvictionStrategy;

extern SSDBufferDesc	*ssd_buffer_descriptors;
extern SSDBufferHashBucket	*ssd_buffer_hashtable;
extern SSDBufferStrategyControl *ssd_buffer_strategy_control;

#define GetSSDBufHashBucket(hash_code) ((SSDBufferHashBucket *) (ssd_buffer_hashtable + (unsigned) (hash_code)))

extern void initSSDBuffer();
extern void read_block(off_t offset, char* ssd_buffer);
extern void write_block(off_t offset, char* ssd_buffer);
//extern int read(unsigned offset);
//extern int write(unsigned offset);

extern unsigned long NSSDBuffers;
extern unsigned long NSSDBufTables;
extern size_t SSD_BUFFER_SIZE;
extern char	smr_device[100];
extern int 	smr_fd;
extern int 	ssd_fd;
