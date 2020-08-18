//This file is to plot HPTDC and TDCV2 spectrum in the same figure.


void plot_T0_comparison(){
	gROOT->SetBatch(kTRUE);
	// gStyle->SetOptStat(10); //only print entries
	gStyle->SetOptStat(0); //no stat box

	char fitVecName[256];
	char output_filename[256];
	double x_shift = -8;

	TFile * hfile = new TFile("output/20200727_112301.dat.dir/20200727_112301.dat.out.root");
	TH1F *p;

	TCanvas *p_output_canvas = new TCanvas("c1", "c1",0,0,700,500);	
	p_output_canvas->cd();
	TPaveStats *st;
	TLegend *legend;

	sprintf(fitVecName, "TDC_00_of_18_Time_Spectrum/tdc_0_channel_17_tdc_time_spectrum_corrected");
	p = (TH1F *)(hfile->Get(fitVecName));
	p->SetLineColor(2);
	p->SetTitle("HPTDC");
	p->SetName("HPTDC");
	p->SetBit( TH1::kNoTitle, true );	
	p->SetMaximum(300);
	p->Draw();
	legend = new TLegend(0.7,0.8,0.9,0.9);
	legend->AddEntry(p);

	sprintf(fitVecName, "TDC_09_of_18_Time_Spectrum/tdc_9_channel_13_tdc_time_spectrum_corrected");
	p = (TH1F *)(hfile->Get(fitVecName));
	p->GetXaxis()->SetLimits(-400+x_shift,400+x_shift); 
	p->SetTitle("TDCV2");
	p->SetName("TDCV2");	
	p->Draw("SAMES");
	legend->AddEntry(p);
	legend->Draw();
	sprintf(output_filename, "output_fig/TDC_spectrum_comparison.png");
    p_output_canvas->SaveAs(output_filename);


    sprintf(fitVecName, "TDC_00_of_18_Time_Spectrum/tdc_0_channel_17_adc_time_spectrum");
	p = (TH1F *)(hfile->Get(fitVecName));
	p->SetLineColor(2);
	p->SetTitle("HPTDC");
	p->SetName("HPTDC");
	p->SetBit( TH1::kNoTitle, true );	
	p->SetMaximum(1000);
	p->Draw();

	legend = new TLegend(0.7,0.8,0.9,0.9);
	legend->AddEntry(p);
	sprintf(fitVecName, "TDC_09_of_18_Time_Spectrum/tdc_9_channel_13_adc_time_spectrum");
	p = (TH1F *)(hfile->Get(fitVecName));
	// p->GetXaxis()->SetLimits(-400+x_shift,400+x_shift); 
	p->SetTitle("TDCV2");
	p->SetName("TDCV2");	
	p->Draw("SAMES");

	legend->AddEntry(p);
	legend->Draw();
	sprintf(output_filename, "output_fig/ADC_spectrum_comparison.png");
    p_output_canvas->SaveAs(output_filename);






}