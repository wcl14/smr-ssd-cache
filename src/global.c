#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "main.h"

unsigned long NSSDBuffers = 500000;
unsigned long NSSDBufTables = 500000;
/*unsigned long NSSDBuffers = 1000;
unsigned long NSSDBufTables = 500;*/
unsigned long SSD_BUFFER_SIZE = 4096;
unsigned long NSMRBands = 262144;		// 262144*36MB~9TB
unsigned long NSMRBlocks = 2621952;		// 2621952*8KB~20GB
//unsigned long NSSDs = 2621952;		// 2621952*8KB~20GB
unsigned long NSSDs = 100000;
unsigned long NSSDTables = 100000;
unsigned long NBANDTables = 2621952;
/*unsigned long NSMRBands = 500;		// 569*36MB~20GB
unsigned long NSMRBlocks = 500;		// 2621952*8KB~20GB
unsigned long NSSDs = 500;		// 2621952*8KB~20GB
unsigned long NSSDTables = 300;
unsigned long NBANDTables = 300; */
size_t SSD_SIZE = 4096;
size_t BLCKSZ = 4096;
size_t BNDSZ = 400*1024;
//size_t BNDSZ = 8*1024;
unsigned long INTERVALTIMELIMIT = 1000;
unsigned long NSSDLIMIT = 500000;
unsigned long NSSDCLEAN = 20000;
//unsigned long NSSDLIMIT = 2500000;
//unsigned long NSSDCLEAN = 100000;
/*unsigned long INTERVALTIMELIMIT = 1000;
unsigned long NSSDLIMIT = 400;
unsigned long NSSDCLEAN = 200;*/
//char smr_device[100] = "/dev/sdc2";
//char ssd_device[100] = "/dev/sda1";
//char inner_ssd_device[100] = "/dev/hioa2";
//char smr_device[] = "/github/smr-ssd-cache/smr";
//char ssd_device[] = "/github/smr-ssd-cache/ssd";
//char inner_ssd_device[] = "/github/smr-ssd-cache/inner_ssd";
char smr_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/smr";
char ssd_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/ssd";
char inner_ssd_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/inner_ssd";
//SSDEvictionStrategy EvictStrategy = CLOCK;
//SSDEvictionStrategy EvictStrategy = LRUofBand;
SSDEvictionStrategy EvictStrategy = Most;
//SSDEvictionStrategy EvictStrategy = LRU;
//SSDEvictionStrategy EvictStrategy = SCAN;
int BandOrBlock = 0;
/*Block = 0, Band=1*/
int 		    smr_fd;
int 		    ssd_fd;
int 		    inner_ssd_fd;
unsigned long	interval_time;
unsigned long hit_num;
unsigned long flush_bands;
unsigned long flush_fifo_blocks;
unsigned long flush_ssd_blocks;
//unsigned long write-fifo-num;
//unsigned long write-ssd-num;
unsigned long flush_fifo_times;

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
