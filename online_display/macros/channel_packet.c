//This channel_packet.c defines a struct for data transfer through udp socket

#include "macros/GlobalIncludes.h"
struct Channel_packet{
	int tdc_id;
	int tdc_chnl_id;
	int adc_hist[ADC_HIST_TOTAL_BIN];
	int tdc_hist[TDC_HIST_TOTAL_BIN];
	int adc_entries;
	int tdc_entries;
	int adc_hist_raw[ADC_HIST_TOTAL_BIN];
	int tdc_hist_raw[TDC_HIST_TOTAL_BIN];
	int adc_entries_raw;
	int tdc_entries_raw;
};