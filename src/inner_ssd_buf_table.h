#ifndef SSDBUFTABLE_H
#define SSDBUFTABLE_H

extern void initSSDTable(int size);
extern unsigned ssdtableHashcode(SSDTag *ssd_tag);
extern int ssdtableLookup(SSDTag *ssd_tag, unsigned hash_code);
extern int ssdtableInsert(SSDTag *ssd_tag, unsigned hash_code, int ssd_id);
extern int ssdtableDelete(SSDTag *ssd_tag, unsigned hash_code);
#endif   /* SSDBUFTABLE_H */
