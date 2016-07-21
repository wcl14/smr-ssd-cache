#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "main.h"

//unsigned long NSSDBuffers = 10000000;
//unsigned long NSSDBufTables = 10000000;
unsigned long NSSDBuffers = 10;
unsigned long NSSDBufTables = 5;
unsigned long SSD_BUFFER_SIZE = 4096;
//unsigned long NSMRBands = 569;		// 569*36MB~20GB
//unsigned long NSMRBlocks = 2621952;		// 2621952*8KB~20GB
//unsigned long NSSDs = 2621952;		// 2621952*8KB~20GB
//unsigned long NSSDTables = 10000000;
unsigned long NSMRBands = 5;		// 569*36MB~20GB
unsigned long NSMRBlocks = 5;		// 2621952*8KB~20GB
unsigned long NSSDs = 5;		// 2621952*8KB~20GB
unsigned long NSSDTables = 3;
size_t SSD_SIZE = 4096;
size_t BLCKSZ = 4096;
//size_t BNDSZ = 36*1024;
size_t BNDSZ = 8*1024;
//unsigned long INTERVALTIMELIMIT = 1000;
//unsigned long NSSDLIMIT = 2500000;
//unsigned long NSSDCLEAN = 100000;
unsigned long INTERVALTIMELIMIT = 1000;
unsigned long NSSDLIMIT = 4;
unsigned long NSSDCLEAN = 2;
//char smr_device[100] = "/dev/sdc2";
//char ssd_device[100] = "/dev/sda1";
//char inner_ssd_device[100] = "/dev/hioa2";
//char smr_device[] = "/github/smr-ssd-cache/smr";
//char ssd_device[] = "/github/smr-ssd-cache/ssd";
//char inner_ssd_device[] = "/github/smr-ssd-cache/inner_ssd";
char smr_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/smr";
char ssd_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/ssd";
char inner_ssd_device[] = "/Users/wangchunling/Software/code/smr-test/smr-ssd-cache/src/inner_ssd";
SSDEvictionStrategy EvictStrategy = LRU;

int 		    smr_fd;
int 		    ssd_fd;
int 		    inner_ssd_fd;
unsigned long	interval_time;

pthread_mutex_t free_ssd_mutex;
pthread_mutex_t inner_ssd_hdr_mutex;
pthread_mutex_t inner_ssd_hash_mutex;

SSDBufferDesc	*ssd_buffer_descriptors;
SSDBufferStrategyControl	*ssd_buffer_strategy_control;
SSDBufferHashBucket	        *ssd_buffer_hashtable;

SSDStrategyControl	*ssd_strategy_control;

SSDDesc		*ssd_descriptors;
char		*ssd_blocks;
SSDHashBucket	*ssd_hashtable;

