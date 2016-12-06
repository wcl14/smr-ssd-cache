#define DEBUG 0
/*----------------------------------Most---------------------------------*/
#include <band_table.h>

extern unsigned long WRITEAMPLIFICATION;

void initSSDBufferForWA();
SSDBufferDesc *getWABuffer(SSDBufferTag);
void hitInMostBuffer();
