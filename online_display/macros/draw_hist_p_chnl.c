#include <stdio.h>
#include <stdlib.h>


// ROOT includes
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"

#include "macros/channel_packet.c"
#include "macros/GlobalIncludes.h"





void draw_hist_p_chnl(struct Channel_packet *p_chnl){

    TH1F *p_th1_hist;
  	TString h_name;
    


  	//draw ADC hist
    TCanvas *adc_canvas = new TCanvas("c1", "ADC Plots");
    h_name.Form("tdc_%d_chnl_%d_adc_spectrum", p_chnl->tdc_id, p_chnl->tdc_chnl_id);
  	p_th1_hist = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
  	p_th1_hist->GetXaxis()->SetTitle("time/ns");
  	p_th1_hist->GetYaxis()->SetTitle("entries");
  	for(Int_t bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
  		p_th1_hist->SetBinContent(bin_index+1,p_chnl->adc_hist[bin_index]);
  	}	
  	p_th1_hist->SetEntries(p_chnl->adc_entries);
  	p_th1_hist->DrawCopy();


    //draw TDC hist
    TCanvas *tdc_canvas = new TCanvas("c2", "TDC Plots");
    tdc_canvas->SetWindowPosition(710,0);

    h_name.Form("tdc_%d_chnl_%d_tdc_spectrum", p_chnl->tdc_id, p_chnl->tdc_chnl_id);
    p_th1_hist = new TH1F(h_name, h_name, TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
    p_th1_hist->GetXaxis()->SetTitle("time/ns");
    p_th1_hist->GetYaxis()->SetTitle("entries");
    for(Int_t bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
      p_th1_hist->SetBinContent(bin_index+1,p_chnl->tdc_hist[bin_index]);
    } 
    p_th1_hist->SetEntries(p_chnl->tdc_entries);
    p_th1_hist->DrawCopy();

  	delete p_th1_hist;
    return;
}







