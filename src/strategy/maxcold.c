#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "ssd_buf_table_for_coldmax_history.h"
#include "maxcold.h"

static volatile void *addToLRUHeadNow(SSDBufferDescForMaxColdNow * ssd_buf_hdr_for_maxcold);
static volatile void *deleteFromLRUNow(SSDBufferDescForMaxColdNow * ssd_buf_hdr_for_maxcold);
static volatile void *moveToLRUHeadNow(SSDBufferDescForMaxColdNow * ssd_buf_hdr_for_maxcold);
static volatile void *addToLRUHeadHistory(SSDBufferDescForMaxColdHistory * ssd_buf_hdr_for_maxcold);
static volatile void *deleteFromLRUHistory(SSDBufferDescForMaxColdHistory * ssd_buf_hdr_for_maxcold);
static volatile void *moveToLRUHeadHistory(SSDBufferDescForMaxColdHistory * ssd_buf_hdr_for_maxcold);

static volatile unsigned long
GetSMRZoneNumFromSSD(size_t offset)
{
    return offset / ZONESZ;
}

static volatile void *
resetSSDBufferForMaxColdHistory()
{
	SSDBufferDescForMaxColdHistory *ssd_buf_hdr_for_maxcold_history;
	BandDescForMaxColdHistory *band_hdr_for_maxcold_history;
	long		i, next;
    long        total_history_ssds = ssd_buffer_strategy_control_for_maxcold_history->n_usedssds;

	band_hdr_for_maxcold_history = band_descriptors_for_maxcold_history;
	for (i = 0; i < NSMRBands; band_hdr_for_maxcold_history++, i++) {
		band_hdr_for_maxcold_history->band_num = i;
		band_hdr_for_maxcold_history->current_cold_pages = 0;
	}

    next = ssd_buffer_strategy_control_for_maxcold_history->first_lru; 
    for (i = 0; i < total_history_ssds; i++) {
        ssd_buf_hdr_for_maxcold_history = &ssd_buffer_descriptors_for_maxcold_history[next];
        unsigned long   ssd_buf_hash = ssdbuftableHashcode(&ssd_buf_hdr_for_maxcold_history->ssd_buf_tag);
        long            ssd_buf_id = ssdbuftableLookup(&ssd_buf_hdr_for_maxcold_history->ssd_buf_tag, ssd_buf_hash);
        next = ssd_buf_hdr_for_maxcold_history->next_lru;
        if (ssd_buf_id < 0) {
	        SSDBufferTag	ssd_buf_tag_history = ssd_buf_hdr_for_maxcold_history->ssd_buf_tag;
	        unsigned long	ssd_buf_hash_history = ssdbuftableHashcodeHistory(&ssd_buf_tag_history);
	        long		ssd_buf_id_history = ssdbuftableLookupHistory(&ssd_buf_tag_history, ssd_buf_hash_history);
		    ssdbuftableDeleteHistory(&ssd_buf_tag_history, ssd_buf_hash_history);
            deleteFromLRUHistory(ssd_buf_hdr_for_maxcold_history);
            ssd_buf_hdr_for_maxcold_history->next_freessd = ssd_buffer_strategy_control_for_maxcold_history->first_freessd;
            ssd_buffer_strategy_control_for_maxcold_history->first_freessd = ssd_buf_hdr_for_maxcold_history->ssd_buf_id;
        }
    }

	return NULL;
}

static volatile void *
resetSSDBufferForMaxColdNow()
{
    SSDBufferDescForMaxColdNow *ssd_buf_hdr_for_maxcold_now = &ssd_buffer_descriptors_for_maxcold_now[ssd_buffer_strategy_control_for_maxcold_now->first_lru];
	long		i, next;
    for (i = 0; i < ssd_buffer_strategy_control_for_maxcold_now->n_usedssds; i++) {
        next = ssd_buf_hdr_for_maxcold_now->next_lru;
        deleteFromLRUNow(ssd_buf_hdr_for_maxcold_now);
        ssd_buf_hdr_for_maxcold_now = &ssd_buffer_descriptors_for_maxcold_now[next];
    }

	ssd_buffer_strategy_control_for_maxcold_now->first_lru = -1;
	ssd_buffer_strategy_control_for_maxcold_now->last_lru = -1;
	ssd_buffer_strategy_control_for_maxcold_now->n_usedssds = 0;

	ssd_buf_hdr_for_maxcold_now = ssd_buffer_descriptors_for_maxcold_now;
	for (i = 0; i < NSSDBuffers; ssd_buf_hdr_for_maxcold_now++, i++) {
		ssd_buf_hdr_for_maxcold_now->next_lru = -1;
		ssd_buf_hdr_for_maxcold_now->last_lru = -1;
	}
    
    SSDBufferDescForMaxColdHistory *ssd_buf_hdr_for_maxcold_history = &ssd_buffer_descriptors_for_maxcold_history[ssd_buffer_strategy_control_for_maxcold_history->first_lru];
    next = ssd_buffer_strategy_control_for_maxcold_now->first_lru; 
    for (i = 0; i < ssd_buffer_strategy_control_for_maxcold_history->n_usedssds; i++) {
        unsigned long   band_num = GetSMRZoneNumFromSSD(ssd_buf_hdr_for_maxcold_history->ssd_buf_tag.offset);
        if (band_descriptors_for_maxcold_now[band_num].ischosen > 0) {
            unsigned long   ssd_buf_hash = ssdbuftableHashcode(&ssd_buf_hdr_for_maxcold_history->ssd_buf_tag);
            long            ssd_buf_id = ssdbuftableLookup(&ssd_buf_hdr_for_maxcold_history->ssd_buf_tag, ssd_buf_hash);
            ssd_buf_hdr_for_maxcold_now = &ssd_buffer_descriptors_for_maxcold_now[ssd_buf_id];
            addToLRUHeadNow(ssd_buf_hdr_for_maxcold_now);
        }
        next = ssd_buf_hdr_for_maxcold_history->next_lru;
        ssd_buf_hdr_for_maxcold_history = &ssd_buffer_descriptors_for_maxcold_history[next];
    }

	return NULL;
}

/*
 * init buffer hash table, strategy_control, buffer, work_mem
 */
void
initSSDBufferForMaxCold()
{
	initSSDBufTableHistory(NSSDBufTables * 5);

	ssd_buffer_strategy_control_for_maxcold_history = (SSDBufferStrategyControlForMaxColdHistory *) malloc(sizeof(SSDBufferStrategyControlForMaxColdHistory));
	ssd_buffer_strategy_control_for_maxcold_now = (SSDBufferStrategyControlForMaxColdNow *) malloc(sizeof(SSDBufferStrategyControlForMaxColdNow));
	ssd_buffer_descriptors_for_maxcold_history = (SSDBufferDescForMaxColdHistory *) malloc(sizeof(SSDBufferDescForMaxColdHistory) * NSSDBuffers * (PERIODTIMES / NSSDLIMIT + 1));
	ssd_buffer_descriptors_for_maxcold_now = (SSDBufferDescForMaxColdNow *) malloc(sizeof(SSDBufferDescForMaxColdNow) * NSSDBuffers);
	band_descriptors_for_maxcold_history = (BandDescForMaxColdHistory *) malloc(sizeof(BandDescForMaxColdHistory) * NSMRBands);
	band_descriptors_for_maxcold_now = (BandDescForMaxColdNow *) malloc(sizeof(BandDescForMaxColdNow) * NSMRBands);

	//At first, all data pages in SMR can be chosen for evict, and the strategy is actually LRU.
			BandDescForMaxColdNow * band_hdr_for_maxcold_now;
	long		i;
	band_hdr_for_maxcold_now = band_descriptors_for_maxcold_now;
	for (i = 0; i < NSMRBands; band_hdr_for_maxcold_now++, i++) {
		band_hdr_for_maxcold_now->ischosen = 1;
	}

    // init ssd_buffer_strategy_control_for_maxcold_now & ssd_buffer_descriptors_for_maxcold_now
	ssd_buffer_strategy_control_for_maxcold_now->first_lru = -1;
	ssd_buffer_strategy_control_for_maxcold_now->last_lru = -1;
	ssd_buffer_strategy_control_for_maxcold_now->n_usedssds = 0;

	SSDBufferDescForMaxColdNow *ssd_buf_hdr_for_maxcold_now;

	ssd_buf_hdr_for_maxcold_now = ssd_buffer_descriptors_for_maxcold_now;
	for (i = 0; i < NSSDBuffers; ssd_buf_hdr_for_maxcold_now++, i++) {
		ssd_buf_hdr_for_maxcold_now->ssd_buf_id = i;
		ssd_buf_hdr_for_maxcold_now->next_lru = -1;
		ssd_buf_hdr_for_maxcold_now->last_lru = -1;
	}
	
    // init ssd_buffer_strategy_control_for_maxcold_history & ssd_buffer_descriptors_for_maxcold_history
    ssd_buffer_strategy_control_for_maxcold_history->first_lru = -1;
	ssd_buffer_strategy_control_for_maxcold_history->last_lru = -1;
    ssd_buffer_strategy_control_for_maxcold_history->first_freessd = 0;
	ssd_buffer_strategy_control_for_maxcold_history->last_freessd = NSSDBuffers * (PERIODTIMES / NSSDLIMIT + 1) - 1;
	ssd_buffer_strategy_control_for_maxcold_history->n_usedssds = 0;

	SSDBufferDescForMaxColdHistory *ssd_buf_hdr_for_maxcold_history;
	BandDescForMaxColdHistory *band_hdr_for_maxcold_history;

	ssd_buf_hdr_for_maxcold_history = ssd_buffer_descriptors_for_maxcold_history;
	for (i = 0; i < NSSDBuffers * (PERIODTIMES / NSSDLIMIT + 1); ssd_buf_hdr_for_maxcold_history++, i++) {
		ssd_buf_hdr_for_maxcold_history->ssd_buf_tag.offset = 0;
		ssd_buf_hdr_for_maxcold_history->ssd_buf_id = i;
		ssd_buf_hdr_for_maxcold_history->next_lru = -1;
		ssd_buf_hdr_for_maxcold_history->last_lru = -1;
		ssd_buf_hdr_for_maxcold_history->next_freessd = i+1;
		ssd_buf_hdr_for_maxcold_history->hit_times = 0;
	}
	ssd_buf_hdr_for_maxcold_history->next_freessd = -1;

	band_hdr_for_maxcold_history = band_descriptors_for_maxcold_history;
	for (i = 0; i < NSMRBands; band_hdr_for_maxcold_history++, i++) {
		band_hdr_for_maxcold_history->band_num = i;
		band_hdr_for_maxcold_history->current_cold_pages = 0;
	}

	flush_fifo_times = 0;
}

static volatile void *
addToLRUHeadHistory(SSDBufferDescForMaxColdHistory * ssd_buf_hdr_for_maxcold_history)
{
	if (ssd_buffer_strategy_control_for_maxcold_history->n_usedssds == 0) {
		ssd_buffer_strategy_control_for_maxcold_history->first_lru = ssd_buf_hdr_for_maxcold_history->ssd_buf_id;
		ssd_buffer_strategy_control_for_maxcold_history->last_lru = ssd_buf_hdr_for_maxcold_history->ssd_buf_id;
	} else {
		ssd_buf_hdr_for_maxcold_history->next_lru = ssd_buffer_descriptors_for_maxcold_history[ssd_buffer_strategy_control_for_maxcold_history->first_lru].ssd_buf_id;
		ssd_buf_hdr_for_maxcold_history->last_lru = -1;
		ssd_buffer_descriptors_for_maxcold_history[ssd_buffer_strategy_control_for_maxcold_history->first_lru].last_lru = ssd_buf_hdr_for_maxcold_history->ssd_buf_id;
		ssd_buffer_strategy_control_for_maxcold_history->first_lru = ssd_buf_hdr_for_maxcold_history->ssd_buf_id;
	}
    ssd_buffer_strategy_control_for_maxcold_history->n_usedssds ++;

	return NULL;
}

static volatile void *
deleteFromLRUHistory(SSDBufferDescForMaxColdHistory * ssd_buf_hdr_for_maxcold_history)
{
	if (ssd_buf_hdr_for_maxcold_history->last_lru >= 0) {
		ssd_buffer_descriptors_for_maxcold_history[ssd_buf_hdr_for_maxcold_history->last_lru].next_lru = ssd_buf_hdr_for_maxcold_history->next_lru;
	} else {
		ssd_buffer_strategy_control_for_maxcold_history->first_lru = ssd_buf_hdr_for_maxcold_history->next_lru;
	}
	if (ssd_buf_hdr_for_maxcold_history->next_lru >= 0) {
		ssd_buffer_descriptors_for_maxcold_history[ssd_buf_hdr_for_maxcold_history->next_lru].last_lru = ssd_buf_hdr_for_maxcold_history->last_lru;
	} else {
		ssd_buffer_strategy_control_for_maxcold_history->last_lru = ssd_buf_hdr_for_maxcold_history->last_lru;
	}
    ssd_buffer_strategy_control_for_maxcold_history->n_usedssds --;

	return NULL;
}

static volatile void *
moveToLRUHeadHistory(SSDBufferDescForMaxColdHistory * ssd_buf_hdr_for_maxcold_history)
{
	deleteFromLRUHistory(ssd_buf_hdr_for_maxcold_history);
	addToLRUHeadHistory(ssd_buf_hdr_for_maxcold_history);

	return NULL;
}

static volatile void *
addToLRUHeadNow(SSDBufferDescForMaxColdNow * ssd_buf_hdr_for_maxcold_now)
{

	if (ssd_buffer_strategy_control_for_maxcold_now->n_usedssds == 0) {
		ssd_buffer_strategy_control_for_maxcold_now->first_lru = ssd_buf_hdr_for_maxcold_now->ssd_buf_id;
		ssd_buffer_strategy_control_for_maxcold_now->last_lru = ssd_buf_hdr_for_maxcold_now->ssd_buf_id;
	} else {
		ssd_buf_hdr_for_maxcold_now->next_lru = ssd_buffer_descriptors_for_maxcold_now[ssd_buffer_strategy_control_for_maxcold_now->first_lru].ssd_buf_id;
		ssd_buf_hdr_for_maxcold_now->last_lru = -1;
		ssd_buffer_descriptors_for_maxcold_now[ssd_buffer_strategy_control_for_maxcold_now->first_lru].last_lru = ssd_buf_hdr_for_maxcold_now->ssd_buf_id;
		ssd_buffer_strategy_control_for_maxcold_now->first_lru = ssd_buf_hdr_for_maxcold_now->ssd_buf_id;
	}
    ssd_buffer_strategy_control_for_maxcold_now->n_usedssds ++;

    return NULL;
}

static volatile void *
deleteFromLRUNow(SSDBufferDescForMaxColdNow * ssd_buf_hdr_for_maxcold_now)
{
    long i;

	if (ssd_buf_hdr_for_maxcold_now->last_lru >= 0) {
		ssd_buffer_descriptors_for_maxcold_now[ssd_buf_hdr_for_maxcold_now->last_lru].next_lru = ssd_buf_hdr_for_maxcold_now->next_lru;
	} else {
		ssd_buffer_strategy_control_for_maxcold_now->first_lru = ssd_buf_hdr_for_maxcold_now->next_lru;
	}
	if (ssd_buf_hdr_for_maxcold_now->next_lru >= 0) {
		ssd_buffer_descriptors_for_maxcold_now[ssd_buf_hdr_for_maxcold_now->next_lru].last_lru = ssd_buf_hdr_for_maxcold_now->last_lru;
	} else {
		ssd_buffer_strategy_control_for_maxcold_now->last_lru = ssd_buf_hdr_for_maxcold_now->last_lru;
	}
    ssd_buffer_strategy_control_for_maxcold_now->n_usedssds --;

	return NULL;
}

static volatile void *
moveToLRUHeadNow(SSDBufferDescForMaxColdNow * ssd_buf_hdr_for_maxcold_now)
{
	deleteFromLRUNow(ssd_buf_hdr_for_maxcold_now);
	addToLRUHeadNow(ssd_buf_hdr_for_maxcold_now);

	return NULL;
}

static volatile void *
qsort_band_history(long start, long end)
{
	long		i = start;
	long		j = end;
	BandDescForMaxColdHistory x = band_descriptors_for_maxcold_history[i];
	while (i < j) {
		while (band_descriptors_for_maxcold_history[j].current_cold_pages <= x.current_cold_pages && i < j)
			j--;
		if (i < j)
			band_descriptors_for_maxcold_history[i] = band_descriptors_for_maxcold_history[j];
		while (band_descriptors_for_maxcold_history[i].current_cold_pages >= x.current_cold_pages && i < j)
			i++;
		if (i < j)
			band_descriptors_for_maxcold_history[j] = band_descriptors_for_maxcold_history[i];
	}
	band_descriptors_for_maxcold_history[i] = x;
	if (i - 1 > start)
		qsort_band_history(start, i - 1);
	if (j + 1 < end)
		qsort_band_history(j + 1, end);
}

static volatile long
find_non_empty()
{
    long        i = 0, j = NSMRBands - 1;
    BandDescForMaxColdHistory tmp;

    while (i < j) {
        while (i < j && band_descriptors_for_maxcold_history[j].current_cold_pages == 0)
            j--;
        while (i < j && band_descriptors_for_maxcold_history[i].current_cold_pages != 0)
            i++;
        if (i < j) {
            tmp = band_descriptors_for_maxcold_history[i];
            band_descriptors_for_maxcold_history[i] = band_descriptors_for_maxcold_history[j];
            band_descriptors_for_maxcold_history[j] = tmp;
        }
    }

    return i;
}

static volatile void *
pause_and_caculate_next_period_maxcold()
{
	unsigned long	band_num;
	SSDBufferDescForMaxColdHistory *ssd_buf_hdr_for_maxcold_history;
	long		i, NNonEmpty;

	BandDescForMaxColdNow *band_hdr_for_maxcold_now;
	band_hdr_for_maxcold_now = band_descriptors_for_maxcold_now;
	for (i = 0; i < NSMRBands; band_hdr_for_maxcold_now++, i++) {
		band_hdr_for_maxcold_now->ischosen = 0;
	}

	ssd_buf_hdr_for_maxcold_history = ssd_buffer_descriptors_for_maxcold_history;
	for (i = 0; i < ssd_buffer_strategy_control_for_maxcold_history->n_usedssds; i++, ssd_buf_hdr_for_maxcold_history++) {
		if (ssd_buf_hdr_for_maxcold_history->hit_times == 1) {
			band_num = GetSMRZoneNumFromSSD(ssd_buf_hdr_for_maxcold_history->ssd_buf_tag.offset);
			band_descriptors_for_maxcold_history[band_num].current_cold_pages++;
		}
	}

    NNonEmpty = find_non_empty();
    qsort_band_history(0, NNonEmpty - 1);

	i = 0;
	unsigned long	total_cold = 0;
	while ((i < NSMRBands) && (total_cold < NSSDLIMIT || i < NCOLDBAND)) {
        //printf("NSSDLIMIT=%ld, total_cold=%ld, band_num=%ld, current_cold_pages=%ld\n", NSSDLIMIT, total_cold, band_descriptors_for_maxcold_history[i].band_num, band_descriptors_for_maxcold_history[i].current_cold_pages);
		total_cold += band_descriptors_for_maxcold_history[i].current_cold_pages;
		band_descriptors_for_maxcold_now[band_descriptors_for_maxcold_history[i].band_num].ischosen = 1;
		i++;
	}

	resetSSDBufferForMaxColdHistory();
	resetSSDBufferForMaxColdNow();
	run_times = 0;

	return NULL;
}

static volatile void *
pause_and_caculate_next_period_maxall()
{
	unsigned long	band_num;
	SSDBufferDesc *ssd_buf_hdr;
	long		i, NNonEmpty;

	BandDescForMaxColdNow *band_hdr_for_maxcold_now;
	band_hdr_for_maxcold_now = band_descriptors_for_maxcold_now;
	for (i = 0; i < NSMRBands; band_hdr_for_maxcold_now++, i++) {
		band_hdr_for_maxcold_now->ischosen = 0;
	}

	ssd_buf_hdr = ssd_buffer_descriptors;
	for (i = 0; i < ssd_buffer_strategy_control->n_usedssd; i++, ssd_buf_hdr++) {
	    band_num = GetSMRZoneNumFromSSD(ssd_buf_hdr->ssd_buf_tag.offset);
		band_descriptors_for_maxcold_history[band_num].current_cold_pages++;
	}

    NNonEmpty = find_non_empty();
    qsort_band_history(0, NNonEmpty - 1);

	i = 0;
	unsigned long	total_cold = 0;
	while ((i < NSMRBands) && (total_cold < NSSDLIMIT || i < NCOLDBAND)) {
        printf("NSSDLIMIT=%ld, total_cold=%ld, band_num=%ld, current_cold_pages=%ld\n", NSSDLIMIT, total_cold, band_descriptors_for_maxcold_history[i].band_num, band_descriptors_for_maxcold_history[i].current_cold_pages);
		total_cold += band_descriptors_for_maxcold_history[i].current_cold_pages;
		band_descriptors_for_maxcold_now[band_descriptors_for_maxcold_history[i].band_num].ischosen = 1;
		i++;
	}

	resetSSDBufferForMaxColdHistory();
	resetSSDBufferForMaxColdNow();
	run_times = 0;

	return NULL;
}

SSDBufferDesc  *
getMaxColdBuffer(SSDBufferTag new_ssd_buf_tag)
{
	if (ssd_buffer_strategy_control->first_freessd < 0 && run_times >= PERIODTIMES)
		pause_and_caculate_next_period_maxcold();
	
	SSDBufferDesc  *ssd_buf_hdr;
	SSDBufferDescForMaxColdHistory *ssd_buf_hdr_for_maxcold_history;
	SSDBufferDescForMaxColdNow *ssd_buf_hdr_for_maxcold_now;

	SSDBufferTag	ssd_buf_tag_history = new_ssd_buf_tag;
	unsigned long	ssd_buf_hash_history = ssdbuftableHashcodeHistory(&ssd_buf_tag_history);
	long		ssd_buf_id_history = ssdbuftableLookupHistory(&ssd_buf_tag_history, ssd_buf_hash_history);
	if (ssd_buf_id_history >= 0) {
		ssd_buf_hdr_for_maxcold_history = &ssd_buffer_descriptors_for_maxcold_history[ssd_buf_id_history];
		moveToLRUHeadHistory(ssd_buf_hdr_for_maxcold_history);
	} else {
        // we make sure that the condition below is always true
        if (ssd_buffer_strategy_control_for_maxcold_history->first_freessd >= 0) {
            ssd_buf_hdr_for_maxcold_history = &ssd_buffer_descriptors_for_maxcold_history[ssd_buffer_strategy_control_for_maxcold_history->first_freessd];
            ssd_buffer_strategy_control_for_maxcold_history->first_freessd = ssd_buf_hdr_for_maxcold_history->next_freessd;
            ssd_buf_hdr_for_maxcold_history->next_freessd = -1;
        }
        ssd_buf_hdr_for_maxcold_history->ssd_buf_tag = new_ssd_buf_tag;
		ssdbuftableInsertHistory(&ssd_buf_tag_history, ssd_buf_hash_history, ssd_buf_hdr_for_maxcold_history->ssd_buf_id);
		addToLRUHeadHistory(ssd_buf_hdr_for_maxcold_history);
	}
	ssd_buf_hdr_for_maxcold_history->hit_times++;

	run_times++;

	if (ssd_buffer_strategy_control->first_freessd >= 0) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->first_freessd];
		ssd_buf_hdr_for_maxcold_now = &ssd_buffer_descriptors_for_maxcold_now[ssd_buffer_strategy_control->first_freessd];
		ssd_buffer_strategy_control->first_freessd = ssd_buf_hdr->next_freessd;
		ssd_buf_hdr->next_freessd = -1;

		unsigned long	band_num = GetSMRZoneNumFromSSD(ssd_buf_hdr->ssd_buf_tag.offset);
		if (band_descriptors_for_maxcold_now[band_num].ischosen > 0) {
			addToLRUHeadNow(ssd_buf_hdr_for_maxcold_now);
		}
		ssd_buffer_strategy_control->n_usedssd++;
		return ssd_buf_hdr;
	}
	flush_fifo_times++;

	ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control_for_maxcold_now->last_lru];
	ssd_buf_hdr_for_maxcold_now = &ssd_buffer_descriptors_for_maxcold_now[ssd_buffer_strategy_control_for_maxcold_now->last_lru];

	unsigned long	band_num = GetSMRZoneNumFromSSD(ssd_buf_hdr->ssd_buf_tag.offset);
	if (band_descriptors_for_maxcold_now[band_num].ischosen > 0) {
		deleteFromLRUNow(ssd_buf_hdr_for_maxcold_now);
	}
	band_num = GetSMRZoneNumFromSSD(new_ssd_buf_tag.offset);
	if (band_descriptors_for_maxcold_now[band_num].ischosen > 0) {
		addToLRUHeadNow(ssd_buf_hdr_for_maxcold_now);
	}
	unsigned char	old_flag = ssd_buf_hdr->ssd_buf_flag;
	SSDBufferTag	old_tag = ssd_buf_hdr->ssd_buf_tag;
	if (DEBUG)
		printf("[INFO] SSDBufferAlloc(): old_flag&SSD_BUF_DIRTY=%d\n", old_flag & SSD_BUF_DIRTY);
	if (old_flag & SSD_BUF_DIRTY != 0) {
        printf("in flush: ssd_buf_id=%ld, offset=%ld, band_num=%ld, zone_num=%ld\n", ssd_buf_hdr->ssd_buf_id, ssd_buf_hdr->ssd_buf_tag.offset, GetSMRBandNumFromSSD(ssd_buf_hdr->ssd_buf_tag.offset), GetSMRZoneNumFromSSD(ssd_buf_hdr->ssd_buf_tag.offset));
		flushSSDBuffer(ssd_buf_hdr);
	}
	if (old_flag & SSD_BUF_VALID != 0) {
		unsigned long	old_hash = ssdbuftableHashcode(&old_tag);
		ssdbuftableDelete(&old_tag, old_hash);
	}

    return ssd_buf_hdr;
}

void           *
hitInMaxColdBuffer(SSDBufferDesc * ssd_buf_hdr)
{
	unsigned long	band_num = GetSMRZoneNumFromSSD(ssd_buf_hdr->ssd_buf_tag.offset);
	SSDBufferDescForMaxColdHistory *ssd_buf_hdr_for_maxcold_history;

	SSDBufferTag	ssd_buf_tag_history = ssd_buf_hdr->ssd_buf_tag;
	unsigned long	ssd_buf_hash_history = ssdbuftableHashcodeHistory(&ssd_buf_tag_history);
	long		ssd_buf_id_history = ssdbuftableLookupHistory(&ssd_buf_tag_history, ssd_buf_hash_history);
	ssd_buf_hdr_for_maxcold_history = &ssd_buffer_descriptors_for_maxcold_history[ssd_buf_id_history];
	ssd_buf_hdr_for_maxcold_history->hit_times++;

	moveToLRUHeadHistory(&ssd_buffer_descriptors_for_maxcold_history[ssd_buf_hdr_for_maxcold_history->ssd_buf_id]);
	if (band_descriptors_for_maxcold_now[band_num].ischosen > 0) {
	    SSDBufferDescForMaxColdNow *ssd_buf_hdr_for_maxcold_now = &ssd_buffer_descriptors_for_maxcold_now[ssd_buf_hdr->ssd_buf_id];
		moveToLRUHeadNow(&ssd_buffer_descriptors_for_maxcold_now[ssd_buf_hdr->ssd_buf_id]);
	}
	return NULL;
}
