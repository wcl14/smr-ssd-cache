#define DEBUG 0
/* ---------------------------clock---------------------------- */

typedef struct
{
	long 		ssd_buf_id;				// ssd buffer location in shared buffer
	unsigned long	usage_count;
} SSDBufferDescForClock;

typedef struct
{
	long		next_victimssd;		// For CLOCK
} SSDBufferStrategyControlForClock;

SSDBufferDescForClock	*ssd_buffer_descriptors_for_clock;
SSDBufferStrategyControlForClock *ssd_buffer_strategy_control_for_clock;

extern void initSSDBufferForClock();
extern SSDBufferDesc *getCLOCKBuffer();
extern void *hitInCLOCKBuffer(SSDBufferDesc *);
