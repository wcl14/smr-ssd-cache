#ifndef SSDBUFTABLE_H
#define SSDBUFTABLE_H

#define size_t long
#define GetSSDBufHashBucketForColdMaxHistory(hash_code) ((SSDBufferHashBucket *) (ssd_buffer_hashtable_history + (unsigned) (hash_code)))

extern void initSSDBufTableHistory(size_t size);
extern unsigned long ssdbuftableHashcodeHistory(SSDBufferTag *ssd_buf_tag);
extern long ssdbuftableLookupHistory(SSDBufferTag *ssd_buf_tag, unsigned long hash_code);
extern long ssdbuftableInsertHistory(SSDBufferTag *ssd_buf_tag, unsigned long hash_code, size_t ssd_buf_id);
extern long ssdbuftableDeleteHistory(SSDBufferTag *ssd_buf_tag, unsigned long hash_code);
#endif   /* SSDBUFTABLE_H */
