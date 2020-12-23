//This file is to plot TDCV2 spectrum from the .root file
void root_plot_single_channel(int tdc, int chnl){

	gROOT->SetBatch(kTRUE); // set to batch mode to inprove the speed
	// int maxEventCount = 100;
	gStyle->SetOptStat(10); //only print entries
	gStyle->SetTitleX(999.);//hist no title
	gStyle->SetTitleY(999.);
	gStyle->SetStatY(0.9);                
	// Set y-position (fraction of pad size)
	gStyle->SetStatX(0.9);                
	// Set x-position (fraction of pad size)
	gStyle->SetStatW(0.25);                
	// Set width of stat-box (fraction of pad size)
	gStyle->SetStatH(0.25);                
	// Set height of stat-box (fraction of pad size)

	char run_name[256];
	sprintf(run_name, "20200727_112301");
	char root_file_name[256];
	sprintf(root_file_name, "output/%s.dat.dir/%s.dat.out.root",run_name,run_name);
	cout<<root_file_name<<endl;

	char adc_hist_name[256];
	char tdc_hist_name[256];
	char tdc_corrected_hist_name[256];
	char output_filename[256];

	TFile * hfile = new TFile(root_file_name);
	TCanvas *p_output_canvas = new TCanvas("c1", "c1",0,0,700,500);	
	TH1F *tdc_hist, *tdc_corrected_hist, *adc_hist;



	sprintf(adc_hist_name, "TDC_%02d_of_18_Time_Spectrum/tdc_%d_channel_%d_adc_time_spectrum", tdc,tdc,chnl);
	adc_hist = (TH1F *)(hfile->Get(adc_hist_name));
	adc_hist->GetXaxis()->SetRangeUser(0,300);
	adc_hist->Draw();
	sprintf(output_filename, "output_fig/tdc_%d_channel_%d_adc_time_spectrum.pdf",tdc,chnl);
    p_output_canvas->SaveAs(output_filename);

    sprintf(tdc_hist_name, "TDC_%02d_of_18_Time_Spectrum/tdc_%d_channel_%d_tdc_time_spectrum", tdc,tdc,chnl);
	tdc_hist = (TH1F *)(hfile->Get(tdc_hist_name));
	tdc_hist->GetXaxis()->SetRangeUser(-300,200);
	tdc_hist->Draw();
	sprintf(output_filename, "output_fig/tdc_%d_channel_%d_tdc_time_spectrum.pdf",tdc,chnl);
    p_output_canvas->SaveAs(output_filename);

    sprintf(tdc_corrected_hist_name, "TDC_%02d_of_18_Time_Spectrum/tdc_%d_channel_%d_tdc_time_spectrum_corrected", tdc,tdc,chnl);
	tdc_corrected_hist = (TH1F *)(hfile->Get(tdc_corrected_hist_name));
	tdc_corrected_hist->GetXaxis()->SetRangeUser(-300,200);
	tdc_corrected_hist->Draw();
	sprintf(output_filename, "output_fig/tdc_%d_channel_%d_tdc_time_spectrum_corrected.pdf",tdc,chnl);
    p_output_canvas->SaveAs(output_filename);






	return;



}
	