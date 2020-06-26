
TF1* func = (TF1*) gROOT->GetFunction(TString("chebyshev") + to_string(2));
TString inputFilename = "run00187939_20190514.dat.out.root";
TString fitVecName, t0path, driftTimePath, histName;

t0path  = "output/";
t0path += inputFilename;
t0path += ".dir/T0.root";
TFile t0File(t0path);

driftTimePath  = "output/";
driftTimePath += inputFilename;
driftTimePath += ".dir/";
driftTimePath += inputFilename;
driftTimePath += ".out.root";
TFile driftFile(driftTimePath);

TVectorD *fitParams;
TH1D     *driftTimes = new TH1D("driftTimes", "", 1024, -1,1);
TH1D     *tempHist;

for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
  for (int ch_id = 0; ch_id != Geometry::MAX_TDC_CHANNEL; ch_id++) {
    if (tdc_id == geo->TRIGGER_MEZZ || !geo->IsActiveTDCChannel(tdc_id, ch_id)) {continue;}
    fitVecName.Form("FitData_tdc_%d_channel_%d", tdc_id, ch_id);
    fitParams = (TVectorD*)t0File.Get(fitVecName);
    t0[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id] = (*fitParams)[T0Fit::T0_INDX];
    
    histName.Form("TDC_%02d_of_%02d_Time_Spectrum/tdc_%d_channel_%d_tdc_time_spectrum_corrected",
		  tdc_id, Geometry::MAX_TDC, tdc_id, ch_id);
    tempHist = (TH1D*) driftFile.Get(histName);
    
    for (int b = 0; b <= tempHist->GetNbinsX(); b++) {
      driftTimes->Fill(NormalizedTime(tempHist->GetBinCenter(b), tdc_id, ch_id), tempHist->GetBinContent(b));
    }
    driftTimes->Draw();
  }  
}


Initialize();
TH1D* cumul = (TH1D*)driftTimes->GetCumulative();
double maxBC = cumul->GetBinContent(cumul->GetNbinsX());
for (int b = 0; b <= cumul->GetNbinsX(); b++) {
  cumul->SetBinContent(b, (cumul->GetBinContent(b)/maxBC -0.5)*2);
}
cumul->SetStats(0);
cumul->GetXaxis()->SetTitle("Drift time (ns)");
cumul->GetYaxis()->SetTitle("Drift radius (mm)");
cumul->SetTitle("RT function Initial guess");
cumul->Fit("chebyshev2");
cumul->Draw();
func->SetLineColor(kRed);
func->Draw("same");                                                       
gPad->Modified();
gPad->Update();
func->Print();

