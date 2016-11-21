#define DEBUG 0
/*----------------------------------Most---------------------------------*/

typedef struct
{
	long ssd_buf_id;//ssd buffer location in shared buffer
	long next_ssd_buf;
} SSDBufferDescForMost;

typedef struct
{
	unsigned long band_id;
	unsigned long current_pages;
    unsigned long first_ssd_id;
} BandDescForMost;

typedef struct
{
    long        nbands;          // # of cached bands
} SSDBufferStrategyControlForMost;

extern unsigned long NBANDTables;

SSDBufferDescForMost *ssd_buffer_descriptors_for_most;
BandDescForMost *band_descriptors_for_most;
SSDBufferStrategyControlForMost *ssd_buffer_strategy_control_for_most;

void initSSDBufferForMost();
SSDBufferDesc *getMostBuffer(SSDBufferTag,long flag);
void hitInMostBuffer();
