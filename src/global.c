#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "main.h"


unsigned long NSSDBuffers;
unsigned long NSSDBufTables;
unsigned long SSD_BUFFER_SIZE;
unsigned long NSMRBands = 194180;		// 194180*(18MB+36MB)/2~5TB
unsigned long NSMRBlocks = 2621952;		// 2621952*8KB~20GB
unsigned long NSSDs;
unsigned long NSSDTables;
unsigned long NBANDTables = 2621952;
size_t SSD_SIZE = 4096;
size_t BLCKSZ = 4096;
size_t BNDSZ = 36*1024*1024;
size_t ZONESZ;
unsigned long INTERVALTIMELIMIT = 100000000;
unsigned long NSSDLIMIT;
unsigned long NSSDCLEAN = 1;
unsigned long WRITEAMPLIFICATION = 100;
unsigned long NCOLDBAND = 1;
unsigned long PERIODTIMES;
char smr_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/smr";
char ssd_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/ssd";
char inner_ssd_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/inner_ssd";
SSDEvictionStrategy EvictStrategy;
int BandOrBlock;
/*Block = 0, Band=1*/
int 		    smr_fd;
int 		    ssd_fd;
int 		    inner_ssd_fd;
unsigned long	interval_time;
unsigned long hit_num;
unsigned long flush_bands;
unsigned long flush_band_size;
unsigned long flush_fifo_blocks;
unsigned long flush_ssd_blocks;
//unsigned long write-fifo-num;
//unsigned long write-ssd-num;
unsigned long flush_fifo_times;
unsigned long run_times;
unsigned long read_ssd_blocks;
unsigned long read_fifo_blocks;
unsigned long read_smr_blocks;
unsigned long read_hit_num;
unsigned long read_smr_bands;
double time_read_ssd;
double time_write_ssd;
double time_read_fifo;
double time_read_smr;
double time_write_fifo;
double time_write_smr;
double time_begin_temp;
double time_now_temp;

pthread_mutex_t free_ssd_mutex;
pthread_mutex_t inner_ssd_hdr_mutex;
pthread_mutex_t inner_ssd_hash_mutex;

SSDBufferDesc	*ssd_buffer_descriptors;
SSDBufferStrategyControl	*ssd_buffer_strategy_control;
SSDBufferHashBucket	        *ssd_buffer_hashtable;

SSDStrategyControl	*ssd_strategy_control;

SSDDesc		*ssd_descriptors;
//char		*ssd_blocks;
SSDHashBucket	*ssd_hashtable;
