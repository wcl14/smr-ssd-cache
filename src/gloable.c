#include "ssd-cache.h"
#include "smr-simulator.h"

unsigned NSSDBuffers = 10000000;
unsigned NSSDBufTables = 10000000;
unsigned SSD_BUFFER_SIZE = 4096;
unsigned NSMRBands = 569;		// 569*36MB~20GB
unsigned NSMRBlocks = 2621952;		// 2621952*8KB~20GB
unsigned NSSDs = 2621952;		// 2621952*8KB~20GB
unsigned NSSDTables = 10000000;
unsigned SSD_SIZE = 4096;
unsigned BLCKSZ = 4096;
unsigned BNDSZ = 36*1024;
unsigned INTERVALTIMELIMIT = 1000;
unsigned NSSDLIMIT = 2500000;
unsigned NSSDCLEAN = 100000;
char smr_device[100] = "/dev/sdb1";
char ssd_device[100] = "/dev/";
char inner_ssd_device[100] = "/dev/";

int *		smr_fd;
int *		ssd_fd;
int *		inner_ssd_fd;
unsigned	interval_time;

SSDBufferDesc	*ssd_buffer_descriptors;
char		*ssd_buffer_blocks;
SSDBufferStrategyControl	*ssd_buffer_strategy_control;
SSDBufferHashBucket	*ssd_buffer_hashtable;

SSDStrategyControl	*ssd_strategy_control;


SSDDesc		*ssd_descriptors;
char		*ssd_blocks;
SSDHashBucket	*ssd_hashtable;
