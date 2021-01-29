
/*
Filled by TdcFit:
FitData[0] = Events in time spectrum
FitData[1] = T0 from fit  
FitData[2] = T0 fit error
FitData[3] = T0 Slope 
FitData[4] = T0 Slope Error 
FitData[5] = Background 
FitData[6] = Background Error 
FitData[7] = T0 chi^{2} 
FitData[8] = DTmax 
FitData[9] = DTmax Error 
FitData[10] = Tmax Slope 
FitData[11] = Tmax Slope Error 
FitData[12] = Tmax chi^{2} 
filled by AdcFit
FitData[13] = ADC peak  
FitData[14] = ADC peak Error 
FitData[15] = ADC width 
FitData[16] = ADC width Error
FitData[17] = ADC skew 
FitData[18] = ADC skew Error
FitData[19] = ADC chi^{2}   
*/
#define NEWTDC_NUMBER 9
void plot_T0_hist(){
	gROOT->SetBatch(kTRUE);
	char fitVecName[256];
	TFile * hfile = new TFile("T0.root");
	char output_filename[256];
	TVectorD *fitData;
	int TDC_array[8]={0,1,2,10,11,12};
	vector<int> TDC_vector (TDC_array, TDC_array + sizeof(TDC_array) / sizeof(int) );

	char hist_T0_HPTDC_name[256];
	char hist_T0_TDCV2_name[256];
	sprintf(hist_T0_HPTDC_name, "HPTDC_T0_hist");
	sprintf(hist_T0_TDCV2_name, "TDCV2_T0_hist");
	TH1D *fit_HPTDC_hist_T0 = new TH1D(hist_T0_HPTDC_name, hist_T0_HPTDC_name,150,-200,100); 
	TH1D *fit_TDCV2_hist_T0 = new TH1D(hist_T0_TDCV2_name, hist_T0_TDCV2_name,150,-200,100); 

	char hist_DTMax_HPTDC_name[256];
	char hist_DTMax_TDCV2_name[256];
	sprintf(hist_DTMax_HPTDC_name, "HPTDC_DTMax_hist");
	sprintf(hist_DTMax_TDCV2_name, "TDCV2_DTMax_hist");
	TH1D *fit_HPTDC_hist_DTMax = new TH1D(hist_DTMax_HPTDC_name, hist_DTMax_HPTDC_name,50,150,250); 
	TH1D *fit_TDCV2_hist_DTMax = new TH1D(hist_DTMax_TDCV2_name, hist_DTMax_TDCV2_name,50,150,250);

	char hist_T0Slope_HPTDC_name[256];
	char hist_T0Slope_TDCV2_name[256];
	sprintf(hist_T0Slope_HPTDC_name, "HPTDC_T0Slope_hist");
	sprintf(hist_T0Slope_TDCV2_name, "TDCV2_T0Slope_hist");
	TH1D *fit_HPTDC_hist_T0Slope = new TH1D(hist_T0Slope_HPTDC_name, hist_T0Slope_HPTDC_name,50,0,15); 
	TH1D *fit_TDCV2_hist_T0Slope = new TH1D(hist_T0Slope_TDCV2_name, hist_T0Slope_TDCV2_name,50,0,15);  

			
	for(auto tdc_id:TDC_vector){
		for(int ch_id=0;ch_id<24;ch_id++){
			sprintf(fitVecName, "FitData_tdc_%d_channel_%d", tdc_id, ch_id);
			fitData = (TVectorD*)hfile->Get(fitVecName);
			if(tdc_id!=NEWTDC_NUMBER){
				fit_HPTDC_hist_T0->Fill(fitData->GetMatrixArray()[1]);
				fit_HPTDC_hist_DTMax->Fill(fitData->GetMatrixArray()[8]);
				fit_HPTDC_hist_T0Slope->Fill(fitData->GetMatrixArray()[3]);
			}
			else{
				fit_TDCV2_hist_T0->Fill(fitData->GetMatrixArray()[1]);
				fit_TDCV2_hist_DTMax->Fill(fitData->GetMatrixArray()[8]);
				fit_TDCV2_hist_T0Slope->Fill(fitData->GetMatrixArray()[3]);
			}
		}//end for ch_id
	}//end for tdc_id

	TCanvas *p_output_canvas = new TCanvas("c1", "c1",0,0,700,500);	
	p_output_canvas->cd();
	TPaveStats *st;
	TLegend *legend;


	fit_HPTDC_hist_T0->GetXaxis()->SetTitle("time/ns");
    fit_HPTDC_hist_T0->GetYaxis()->SetTitle("entries");
	fit_HPTDC_hist_T0->Draw();
	fit_HPTDC_hist_T0->SetBit( TH1::kNoTitle, true );

	// fit_TDCV2_hist_T0->SetLineColor(2);
	// fit_TDCV2_hist_T0->Draw("SAMES");
	gPad->Update();
	// st = (TPaveStats*)fit_TDCV2_hist_T0->FindObject("stats");
	// st->SetY1NDC(0.55); //new x start position
	// st->SetY2NDC(0.75); //new x end position
	// legend = new TLegend(0.73,0.42,0.98,0.52);
	// legend->AddEntry(fit_HPTDC_hist_T0);
    // legend->AddEntry(fit_TDCV2_hist_T0);
    // legend->Draw();
    // p_output_canvas->Update();
	sprintf(output_filename, "output_fig/T0_hist_1.png");
    p_output_canvas->SaveAs(output_filename);


	fit_HPTDC_hist_DTMax->GetXaxis()->SetTitle("time/ns");
    fit_HPTDC_hist_DTMax->GetYaxis()->SetTitle("entries");
	fit_HPTDC_hist_DTMax->Draw();
	fit_HPTDC_hist_DTMax->SetBit( TH1::kNoTitle, true );

	// fit_TDCV2_hist_DTMax->SetLineColor(2);
	// fit_TDCV2_hist_DTMax->Draw("SAMES");
	gPad->Update();
	// st = (TPaveStats*)fit_TDCV2_hist_DTMax->FindObject("stats");
	// st->SetY1NDC(0.55); //new x start position
	// st->SetY2NDC(0.75); //new x end position
	// legend = new TLegend(0.73,0.42,0.98,0.52);
	// legend->AddEntry(fit_HPTDC_hist_DTMax);
    // legend->AddEntry(fit_TDCV2_hist_DTMax);
    // legend->Draw();
	sprintf(output_filename, "output_fig/DTMax_hist_1.png");
    p_output_canvas->SaveAs(output_filename);



	fit_HPTDC_hist_T0Slope->GetXaxis()->SetTitle("time/ns");
    fit_HPTDC_hist_T0Slope->GetYaxis()->SetTitle("entries");
	fit_HPTDC_hist_T0Slope->Draw();
	fit_HPTDC_hist_T0Slope->SetBit( TH1::kNoTitle, true );

	fit_TDCV2_hist_T0Slope->SetLineColor(2);
	// fit_TDCV2_hist_T0Slope->Draw("SAMES");
	gPad->Update();
	// st = (TPaveStats*)fit_TDCV2_hist_T0Slope->FindObject("stats");
	// st->SetY1NDC(0.55); //new x start position
	// st->SetY2NDC(0.75); //new x end position
	// legend = new TLegend(0.73,0.42,0.98,0.52);
	// legend->AddEntry(fit_HPTDC_hist_T0Slope);
    // legend->AddEntry(fit_TDCV2_hist_T0Slope);
    // legend->Draw();
	sprintf(output_filename, "output_fig/T0Slope_hist_1.png");
    p_output_canvas->SaveAs(output_filename);



}