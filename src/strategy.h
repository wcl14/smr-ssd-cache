#define DEBUG 0
/* ---------------------------strategy---------------------------- */

extern SSDBufferDesc *getCLOCKBuffer();
extern SSDBufferDesc *getLRUBuffer();
extern void *hitInLRUBuffer(SSDBufferDesc *ssd_buf_hdr);
extern SSDBufferDesc *getLRUOfBandBuffer();
