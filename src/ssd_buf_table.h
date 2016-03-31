#ifndef SSDBUFTABLE_H
#define SSDBUFTABLE_H

extern void initSSDBufTable(int size);
extern unsigned ssdbuftableHashcode(SSDBufferTag *ssd_buf_tag);
extern int ssdbuftableLookup(SSDBufferTag *ssd_buf_tag, unsigned hash_code);
extern int ssdbuftableInsert(SSDBufferTag *ssd_buf_tag, unsigned hash_code, int ssd_buf_id);
extern int ssdbuftableDelete(SSDBufferTag *ssd_buf_tag, unsigned hash_code);
#endif   /* SSDBUFTABLE_H */
