#include "ssd-cache.h"
#include "smr-simulator.h"

unsigned long NSSDBuffers = 10000000;
unsigned long NSSDBufTables = 10000000;
unsigned long SSD_BUFFER_SIZE = 4096;
unsigned long NSMRBands = 569;		// 569*36MB~20GB
unsigned long NSMRBlocks = 2621952;		// 2621952*8KB~20GB
unsigned long NSSDs = 2621952;		// 2621952*8KB~20GB
unsigned long NSSDTables = 10000000;
size_t SSD_SIZE = 4096;
size_t BLCKSZ = 4096;
size_t BNDSZ = 36*1024;
unsigned long INTERVALTIMELIMIT = 1000;
unsigned long NSSDLIMIT = 2500000;
unsigned long NSSDCLEAN = 100000;
char smr_device[100] = "/dev/sdb1";
char ssd_device[100] = "/dev/";
char inner_ssd_device[100] = "/dev/";

int *		    smr_fd;
int *		    ssd_fd;
int *		    inner_ssd_fd;
unsigned long	interval_time;

SSDBufferDesc	*ssd_buffer_descriptors;
char		    *ssd_buffer_blocks;
SSDBufferStrategyControl	*ssd_buffer_strategy_control;
SSDBufferHashBucket	        *ssd_buffer_hashtable;

SSDStrategyControl	*ssd_strategy_control;


SSDDesc		*ssd_descriptors;
char		*ssd_blocks;
SSDHashBucket	*ssd_hashtable;
