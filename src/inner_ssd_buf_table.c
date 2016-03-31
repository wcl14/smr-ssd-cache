#include <stdio.h>

#include "smr-simulator.h"
#include "ssd-cache.h"

static bool isSamessd(SSDTag *, SSDTag *);

void initSSDTable(int size)
{
	ssd_hashtable = (SSDHashBucket *)malloc(sizeof(SSDHashBucket)*size);
	int i;
	SSDHashBucket *ssd_hash = ssd_hashtable;
	for (i = 0; i < size; ssd_hash++, i++){
		ssd_hash->ssd_id = -1;
		ssd_hash->hash_key.offset = -1;
		ssd_hash->next_item = NULL;
	}
}

unsigned ssdtableHashcode(SSDTag *ssd_tag)
{
	unsigned ssd_hash = (ssd_tag->offset / SSD_BUFFER_SIZE) % NSSDTables;
	return ssd_hash;
}

int ssdtableLookup(SSDTag *ssd_tag, unsigned hash_code)
{
	if (DEBUG)
		printf("[INFO] Lookup ssd_tag: %u\n",ssd_tag->offset);
	SSDHashBucket *nowbucket = GetSSDHashBucket(hash_code);
	while (nowbucket != NULL) {
	//	printf("nowbucket->buf_id = %u %u %u\n", nowbucket->hash_key.rel.database, nowbucket->hash_key.rel.relation, nowbucket->hash_key.block_num);
		if (isSamessd(&nowbucket->hash_key, ssd_tag)) {
	//		printf("find\n");
			return nowbucket->ssd_id;
		}
		nowbucket = nowbucket->next_item;
	}
//	printf("no find\n");

	return -1;
}

int ssdtableInsert(SSDTag *ssd_tag, unsigned hash_code, int ssd_id)
{
	if (DEBUG)
		printf("[INFO] Insert buf_tag: %u\n",ssd_tag->offset);
	SSDHashBucket *nowbucket = GetSSDHashBucket(hash_code);
	while (nowbucket->next_item != NULL && nowbucket != NULL) {
		if (isSamessd(&nowbucket->hash_key, ssd_tag)) {
			return nowbucket->ssd_id;
		}
		nowbucket = nowbucket->next_item;
	}
	if (nowbucket != NULL) {
		SSDHashBucket *newitem = malloc(sizeof(SSDHashBucket));
		newitem->hash_key = *ssd_tag;
		newitem->ssd_id = ssd_id;
		newitem->next_item = NULL;
		nowbucket->next_item = newitem;
	}
	else {
		nowbucket->hash_key = *ssd_tag;
		nowbucket->ssd_id = ssd_id;
		nowbucket->next_item = NULL;
	}

	return -1;
}

int ssdtableDelete(SSDTag *ssd_tag, unsigned hash_code)
{
	if (DEBUG)
		printf("[INFO] Delete buf_tag: %u\n",ssd_tag->offset);
	SSDHashBucket *nowbucket = GetSSDHashBucket(hash_code);
	int del_id;
	SSDHashBucket *delitem;
	nowbucket->next_item;
	while (nowbucket->next_item != NULL && nowbucket != NULL) {
		if (isSamessd(&nowbucket->next_item->hash_key, ssd_tag)) {
			del_id = nowbucket->next_item->ssd_id;
			break;
		}
		nowbucket = nowbucket->next_item;
	}
	//printf("not found2\n");
	if (isSamessd(&nowbucket->hash_key, ssd_tag)) {
		del_id = nowbucket->ssd_id;
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

static bool isSamessd(SSDTag *tag1, SSDTag *tag2)
{
	if (tag1->offset != tag2->offset)
		return 0;
	else return 1;
}
