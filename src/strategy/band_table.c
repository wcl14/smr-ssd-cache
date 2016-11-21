#include <stdio.h>
#include <stdlib.h>
//#include "lruofband.h"
#include "band_table.h"

static bool	isSameband(long band_id1, long band_id2);

void 
initBandTable(size_t size)
{
	band_hashtable = (BandHashBucket *) malloc(sizeof(BandHashBucket) * size);
	size_t		i;
	BandHashBucket *band_hash = band_hashtable;
	for (i = 0; i < size; band_hash++, i++) {
		band_hash->band_id = -1;
		band_hash->ssd_buf_band_id = -1;
		band_hash->next_item = NULL;
	}
}

unsigned long 
bandtableHashcode(long band_id)
{
	unsigned long	band_hash = band_id % NBANDTables;
	return band_hash;
}

size_t 
bandtableLookup(long band_id, unsigned long hash_code)
{
    if (DEBUG)
        printf("[INFO] Lookup band_id: %lu\n", band_id);
	BandHashBucket *nowbucket = GetBandHashBucket(hash_code);
	while (nowbucket != NULL) {
		if (isSameband(nowbucket->band_id, band_id)) {
			return nowbucket->ssd_buf_band_id;
		}
		nowbucket = nowbucket->next_item;
	}
	return -1;
}

long 
bandtableInsert(long band_id, unsigned long hash_code, long ssd_buf_band_id)
{
	BandHashBucket *nowbucket = GetBandHashBucket(hash_code);
	while (nowbucket->next_item != NULL && nowbucket != NULL) {
		nowbucket = nowbucket->next_item;
	}
	if (nowbucket != NULL) {
		BandHashBucket *newitem = (BandHashBucket *) malloc(sizeof(BandHashBucket));
		newitem->band_id = band_id;
		newitem->ssd_buf_band_id = ssd_buf_band_id;
		newitem->next_item = NULL;
		nowbucket->next_item = newitem;
	} else {
		nowbucket->band_id = band_id;
		nowbucket->ssd_buf_band_id = ssd_buf_band_id;
		nowbucket->next_item = NULL;
	}

	return -1;
}

long 
bandtableDelete(long band_id, unsigned long hash_code)
{
	if (DEBUG)
		printf("[INFO] Delete band_id: %ld\n", band_id);
	BandHashBucket *nowbucket = GetBandHashBucket(hash_code);
	long		del_id;
	BandHashBucket *delitem;
	while (nowbucket->next_item != NULL && nowbucket != NULL) {
		if (isSameband(nowbucket->next_item->band_id, band_id)) {
			del_id = nowbucket->next_item->ssd_buf_band_id;
			break;
		}
		nowbucket = nowbucket->next_item;
	}
	//printf("not found2\n");
	if(isSameband(nowbucket->next_item->band_id,band_id)){ 
        del_id = nowbucket->ssd_buf_band_id; 
    }
	//printf("not found3\n");
	if (nowbucket->next_item != NULL) {
		delitem = nowbucket->next_item;
		nowbucket->next_item = nowbucket->next_item->next_item;
		free(delitem);
		return del_id;
	} else {
		delitem = nowbucket->next_item;
		nowbucket->next_item = NULL;
		free(delitem);
		return del_id;
	}

	return -1;
}

static bool 
isSameband(long band_id1, long band_id2)
{
	if (band_id1 != band_id2)
		return 0;
	else
		return 1;
}
