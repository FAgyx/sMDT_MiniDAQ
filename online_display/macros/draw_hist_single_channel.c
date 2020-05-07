#include <stdio.h>
#include <stdlib.h>


// ROOT includes
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"





void draw_hist_single_channel(int *p_hist_array, int bin_count){

	TH1I *p_th1_hist;
  	TString h_name;
  	int xlim_left, xlim_right;
  	int tdc_id = 		*p_hist_array;
  	int tdc_chnl_id = 	*(p_hist_array+1);
  	int is_tdc_spectrum=*(p_hist_array+2);
  	int entries = 		*(p_hist_array+3);
  	cout<<tdc_id<<","<<tdc_chnl_id<<","<<is_tdc_spectrum<<endl;

  	if(is_tdc_spectrum) {
  		h_name.Form("tdc_%d_chnl_%d_tdc_spectrum", tdc_id,tdc_chnl_id);
  		xlim_left = -400;
  		xlim_right = 400;
  	}
  	else {
  		h_name.Form("tdc_%d_chnl_%d_adc_spectrum", tdc_id,tdc_chnl_id);
  		bin_count=bin_count/2;
  		xlim_left = 0;
  		xlim_right = 400;  		
  	}
  	
	p_th1_hist = new TH1I(h_name, h_name, bin_count, xlim_left, xlim_right);
	p_th1_hist->GetXaxis()->SetTitle("time/ns");
	p_th1_hist->GetYaxis()->SetTitle("entries");
	for(Int_t bin_index=1;bin_index<bin_count+1;bin_index++){
		p_th1_hist->SetBinContent(bin_index,*(p_hist_array+bin_index+3));
	}	
	// for(Int_t bin_index=1;bin_index<bin_count+1;bin_index++){
	// 	cout<<p_th1_hist->GetBinContent(bin_index)<<",";
	// }
	p_th1_hist->SetEntries(*(p_hist_array+3));
	cout<<h_name<<"_entries="<<p_th1_hist->GetEntries()<<endl;

	p_th1_hist->DrawCopy();
	delete p_th1_hist;
	return;
}







