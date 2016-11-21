#include <stdio.h>
#include <stdlib.h>
#include "../ssd-cache.h"
#include "../smr-simulator/smr-simulator.h"
#include "most.h"
#include "band_table.h"

void 
initSSDBufferForMost()
{
	initBandTable(NBANDTables);

	SSDBufferDescForMost *ssd_buf_hdr_for_most;
    BandDescForMost *band_hdr_for_most;
	ssd_buffer_descriptors_for_most = (SSDBufferDescForMost *) malloc(sizeof(SSDBufferDescForMost) * NSSDBuffers);
	long		i;
	ssd_buf_hdr_for_most = ssd_buffer_descriptors_for_most;
	for (i = 0; i < NSSDBuffers; ssd_buf_hdr_for_most++, i++) {
		ssd_buf_hdr_for_most->ssd_buf_id = i;
		ssd_buf_hdr_for_most->next_ssd_buf = -1;
	}
	
    band_descriptors_for_most = (BandDescForMost *) malloc(sizeof(BandDescForMost) * NSMRBands);
	band_hdr_for_most = band_descriptors_for_most;
	for (i = 0; i < NSMRBands; band_hdr_for_most++, i++) {
		band_hdr_for_most->band_id = 0;
		band_hdr_for_most->current_pages = 0;
	}

    ssd_buffer_strategy_control_for_most = (SSDBufferStrategyControlForMost *) malloc(sizeof(SSDBufferStrategyControlForMost));
    ssd_buffer_strategy_control_for_most->nbands = 0; 
}

void 
hitInMostBuffer()
{
	return;
}

void 
deleteBand()
{
	//printf("enter fun:deleteBand\n");
	BandDescForMost *band_hdr_for_most;
    BandDescForMost temp;

	band_hdr_for_most = band_descriptors_for_most;
    temp = band_descriptors_for_most[ssd_buffer_strategy_control_for_most->nbands - 1];
    long            parent = 0;
    long            child = parent * 2 + 1;
    while (child < ssd_buffer_strategy_control_for_most->nbands) {
        if (child < ssd_buffer_strategy_control_for_most->nbands && band_descriptors_for_most[child].current_pages < band_descriptors_for_most[child + 1].current_pages) 
            child ++;
        if (temp.current_pages >= band_descriptors_for_most[child].current_pages)
            break;
        else {
            band_descriptors_for_most[parent] = band_descriptors_for_most[child];
            parent = child;
            child = child * 2 + 1;
        }
    }
    band_descriptors_for_most[parent] = temp;
    band_descriptors_for_most[ssd_buffer_strategy_control_for_most->nbands - 1].band_id = 0;
    band_descriptors_for_most[ssd_buffer_strategy_control_for_most->nbands - 1].current_pages = 0;
    ssd_buffer_strategy_control_for_most->nbands --;
	long		    band_num = band_hdr_for_most->band_id;
	//printf("delete band:%d\n", band_num);

	unsigned long	band_hash = bandtableHashcode(band_num);
	long		    ssd_buf_band_id = bandtableLookup(band_num, band_hash);
	SSDBufferDescForMost *ssd_buf_for_most;
	SSDBufferTag	old_tag;
	unsigned char	old_flag;
	unsigned long	old_hash;
	SSDBufferDesc   *ssd_buf_hdr;

	while (ssd_buf_band_id >= 0) {
        long    first_page = band_descriptors_for_most[ssd_buf_band_id].first_ssd_id;
		ssd_buf_for_most = &ssd_buffer_descriptors_for_most[first_page];
		ssd_buf_hdr = &ssd_buffer_descriptors[first_page];

		//printf("--------------first_page:%d\n", first_page);
		ssd_buf_hdr->next_freessd = ssd_buffer_strategy_control->first_freessd;
		ssd_buffer_strategy_control->first_freessd = first_page;
		first_page = ssd_buf_for_most->next_ssd_buf;

		old_flag = ssd_buf_hdr->ssd_buf_flag;
		old_tag = ssd_buf_hdr->ssd_buf_tag;
		if (old_flag & SSD_BUF_DIRTY != 0) {
			flushSSDBuffer(ssd_buf_hdr);
		}
		if (old_flag & SSD_BUF_VALID != 0) {
			old_hash = ssdbuftableHashcode(&old_tag);
			ssdbuftableDelete(&old_tag, old_hash);
		}
		ssd_buffer_strategy_control->n_usedssd--;
	}
	bandtableDelete(band_num, band_hash);
}

void 
addToBand(SSDBufferTag ssd_buf_tag, long first_freessd, long flag)
{
	//printf("enter fun:addtoband\n");
	long		band_num = GetSMRBandNumFromSSD(ssd_buf_tag.offset);
	unsigned long	band_hash = bandtableHashcode(band_num);
	long		ssd_buf_band_id = bandtableLookup(band_num, band_hash);

	SSDBufferDesc  *ssd_buf_hdr;
	SSDBufferDescForMost *ssd_buf_for_most;
	BandDescForMost *band_hdr_for_most;

	if (ssd_buf_band_id >= 0) {
		//printf("already exist:%d\n", band_num);
        long    first_page = band_descriptors_for_most[ssd_buf_band_id].first_ssd_id; 
		ssd_buf_hdr = &ssd_buffer_descriptors[first_page];
		ssd_buf_for_most = &ssd_buffer_descriptors_for_most[first_page];
		SSDBufferDescForMost *new_ssd_buf_for_most;
		new_ssd_buf_for_most = &ssd_buffer_descriptors_for_most[first_freessd];
		new_ssd_buf_for_most->next_ssd_buf = ssd_buf_for_most->next_ssd_buf;
		ssd_buf_for_most->next_ssd_buf = first_freessd;

		//SSDBufferTag tag = ssd_buf_hdr->ssd_buf_tag;
		if ((flag & SSD_BUF_DIRTY) && (ssd_buf_hdr->ssd_buf_flag & SSD_BUF_DIRTY) || (flag & SSD_BUF_VALID)) {
            band_descriptors_for_most[ssd_buf_band_id].current_pages ++;
            BandDescForMost temp;
            long    parent = (ssd_buf_band_id - 1) / 2;
            long    child = ssd_buf_band_id;
            while (parent >= 0 && band_descriptors_for_most[child].current_pages > band_descriptors_for_most[parent].current_pages ) {
                temp = band_descriptors_for_most[child];
                band_descriptors_for_most[child] = band_descriptors_for_most[parent];
                band_descriptors_for_most[parent] = temp;
                child = parent;
                parent = (child - 1) / 2;
            }
		}
	} else {
        ssd_buffer_strategy_control_for_most->nbands ++;
		if ((flag & SSD_BUF_DIRTY) && (ssd_buf_hdr->ssd_buf_flag & SSD_BUF_DIRTY) || (flag & SSD_BUF_VALID)) {
            band_descriptors_for_most[ssd_buffer_strategy_control_for_most->nbands - 1].band_id = band_num;
            band_descriptors_for_most[ssd_buffer_strategy_control_for_most->nbands - 1].current_pages = 1;
		}
	}
}

SSDBufferDesc  *
getMostBuffer(SSDBufferTag ssd_buf_tag, long flag)
{
	//printf("enter fun:getmostbuffer\n");
	SSDBufferDesc  *ssd_buffer_hdr;
	long		first_freessd = ssd_buffer_strategy_control->first_freessd;
	//printf("first_freessd:%d\n", first_freessd);

	if (first_freessd < 0) {
		//printf("no free ssd\n");
		deleteBand();
		first_freessd = ssd_buffer_strategy_control->first_freessd;
	}
	//printf("first_freessd:%d\n", first_freessd);
	addToBand(ssd_buf_tag, first_freessd, flag);
	ssd_buffer_hdr = &ssd_buffer_descriptors[first_freessd];
	ssd_buffer_strategy_control->first_freessd = ssd_buffer_hdr->next_freessd;
	ssd_buffer_hdr->next_freessd = -1;
	ssd_buffer_strategy_control->n_usedssd++;
	return ssd_buffer_hdr;
}
