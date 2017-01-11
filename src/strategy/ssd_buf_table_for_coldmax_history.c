#include <stdio.h>
#include <stdlib.h>

#include "ssd-cache.h"
#include "maxcold.h"
#include "ssd_buf_table_for_coldmax_history.h"

static bool isSamebuf(SSDBufferTag *, SSDBufferTag *);

void initSSDBufTableHistory(size_t size)
{
	ssd_buffer_hashtable_history = (SSDBufferHashBucket *)malloc(sizeof(SSDBufferHashBucket)*size);
	size_t i;
	SSDBufferHashBucket *ssd_buf_hash = ssd_buffer_hashtable_history;
	for (i = 0; i < size; ssd_buf_hash++, i++){
		ssd_buf_hash->ssd_buf_id = -1;
		ssd_buf_hash->hash_key.offset = -1;
		ssd_buf_hash->next_item = NULL;
	}
}

unsigned long ssdbuftableHashcodeHistory(SSDBufferTag *ssd_buf_tag)
{
	unsigned long ssd_buf_hash = (ssd_buf_tag->offset / SSD_BUFFER_SIZE) % NSSDBufTables;
	return ssd_buf_hash;
}

size_t ssdbuftableLookupHistory(SSDBufferTag *ssd_buf_tag, unsigned long hash_code)
{
	if (DEBUG)
		printf("[INFO] Lookup ssd_buf_tag: %lu\n",ssd_buf_tag->offset);
	SSDBufferHashBucket *nowbucket = GetSSDBufHashBucketForColdMaxHistory(hash_code);
	while (nowbucket != NULL) {
		if (isSamebuf(&nowbucket->hash_key, ssd_buf_tag)) {
			return nowbucket->ssd_buf_id;
		}
		nowbucket = nowbucket->next_item;
	}

	return -1;
}

long ssdbuftableInsertHistory(SSDBufferTag *ssd_buf_tag, unsigned long hash_code, long ssd_buf_id)
{
	if (DEBUG)
		printf("[INFO] Insert buf_tag: %lu\n",ssd_buf_tag->offset);
	SSDBufferHashBucket *nowbucket = GetSSDBufHashBucketForColdMaxHistory(hash_code);
	while (nowbucket->next_item != NULL && nowbucket != NULL) {
		if (isSamebuf(&nowbucket->hash_key, ssd_buf_tag)) {
			return nowbucket->ssd_buf_id;
		}
		nowbucket = nowbucket->next_item;
	}
	if (nowbucket != NULL) {
		SSDBufferHashBucket *newitem = (SSDBufferHashBucket*)malloc(sizeof(SSDBufferHashBucket));
		newitem->hash_key = *ssd_buf_tag;
		newitem->ssd_buf_id = ssd_buf_id;
		newitem->next_item = NULL;
		nowbucket->next_item = newitem;
	}
	else {
		nowbucket->hash_key = *ssd_buf_tag;
		nowbucket->ssd_buf_id = ssd_buf_id;
		nowbucket->next_item = NULL;
	}

	return -1;
}

long ssdbuftableDeleteHistory(SSDBufferTag *ssd_buf_tag, unsigned long hash_code)
{
	if (DEBUG)
		printf("[INFO] Delete buf_tag: %lu\n",ssd_buf_tag->offset);
	SSDBufferHashBucket *nowbucket = GetSSDBufHashBucketForColdMaxHistory(hash_code);
	long del_id;
	SSDBufferHashBucket *delitem;
	nowbucket->next_item;
	while (nowbucket->next_item != NULL && nowbucket != NULL) {
		if (isSamebuf(&nowbucket->next_item->hash_key, ssd_buf_tag)) {
			del_id = nowbucket->next_item->ssd_buf_id;
			break;
		}
		nowbucket = nowbucket->next_item;
	}
	//printf("not found2\n");
	if (isSamebuf(&nowbucket->hash_key, ssd_buf_tag)) {
		del_id = nowbucket->ssd_buf_id;
	}
	//printf("not found3\n");
	if (nowbucket->next_item != NULL) {
		delitem = nowbucket->next_item;
		nowbucket->next_item = nowbucket->next_item->next_item;
		free(delitem);
		return del_id;
	}
	else {
		delitem = nowbucket->next_item;
		nowbucket->next_item = NULL;
		free(delitem);
		return del_id;
	}

	return -1;
}

static bool isSamebuf(SSDBufferTag *tag1, SSDBufferTag *tag2)
{
	if (tag1->offset != tag2->offset)
		return 0;
	else return 1;
}
