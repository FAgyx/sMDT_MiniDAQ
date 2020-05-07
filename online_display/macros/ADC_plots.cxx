#include "src/Event.cpp"
#include "src/Cluster.cpp"
#include "src/Hit.cpp"
#include "src/IOUtility.cpp"
#include "src/Geometry.cpp"
#include "src/T0Fit.h"

using namespace Muon;

void ADC_plots(TString inputFileName = "run00187985_20190607.dat") {

  Geometry   geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  // initialize histograms

  TH2D* ADC_drift_time = new TH2D("ADC_drift_time", "Mean ADC vs. Drift Time", 40, 0, 200, 60, 0, 300);
  TH2D* ADC_channel    = new TH2D("ADC_channel", "ADC vs Channel", Geometry::MAX_TDC_CHANNEL*5, -0.5, 5*Geometry::MAX_TDC_CHANNEL-0.5, 60, 0, 300);

  TH1D* driftTime  = new TH1D("driftTime", "Drift time spectrum", 500, -180, 180);
  TH1D* ADCSpectra = new TH1D("ADCSpectra", "ADC time spectrum", 30, 0, 300);
  TH1D* timeSlew   = new TH1D("timeSlew", "Size of Time Slew", 100, 0, 50);
  TH1D* toyADC1    = new TH1D("toyADC1", "Compare TDC 8 Channels 1 and 9", 384, 0, 300);
  TH1D* toyADC2    = new TH1D("toyADC2", "TDC 8 Channel 9", 384, 0, 300);



  // get ttree for event loop
  
  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);
  Event *evt = new Event();
  t->SetBranchAddress("event", &evt);





  // initialize T0 for each tube

  double t0[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];
  double tF[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];

  TString t0path = IOUtility::getT0FilePath(inputFileName);
  TString decodedDataPath = IOUtility::getDecodedOutputFilePath(inputFileName);


  TFile t0File(t0path);
  TFile driftFile(decodedDataPath);
  TString fitVecName;

  TVectorD *fitParams;
  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    for (int ch_id = 0; ch_id != Geometry::MAX_TDC_CHANNEL; ch_id++) {
      if (tdc_id == geo.TRIGGER_MEZZ || !geo.IsActiveTDCChannel(tdc_id, ch_id)) {continue;}
      fitVecName.Form("FitData_tdc_%d_channel_%d", tdc_id, ch_id);
      fitParams = (TVectorD*)t0File.Get(fitVecName);
      t0[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id] = (*fitParams)[T0Fit::T0_INDX]       ;// - 10;                               
      tF[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id] = (*fitParams)[T0Fit::MAX_DRIFT_INDX];// + 20;                               

    }
  }  





  // perform event loop

  for (int i = 0; i < 1000000; i++) {

    if (i % 1000 == 0) cout << i << endl;


    t->GetEntry(i);

    auto clusters = evt->Clusters();
    for (Cluster c : clusters) {
      for (Hit h : c.Hits()) {
	ADC_drift_time->Fill(h.DriftTime() - t0[h.TDC()*Geometry::MAX_TDC_CHANNEL + h.Channel()], h.ADCTime());
	driftTime     ->Fill(h.DriftTime() - t0[h.TDC()*Geometry::MAX_TDC_CHANNEL + h.Channel()]);
	ADCSpectra    ->Fill(h.ADCTime());
	timeSlew      ->Fill(h.DriftTime() - h.CorrTime());
	if (h.TDC() == 8) {
	  ADC_channel->Fill(h.Channel(), h.ADCTime());
	  if (h.Channel() == 0) {
	    toyADC1->Fill(h.ADCTime());
	  }
	  else if (h.Channel() == 9) {
	    toyADC2->Fill(h.ADCTime());
	  }
	}
	else if (h.TDC() == 9) {
	  ADC_channel->Fill(h.Channel()+Geometry::MAX_TDC_CHANNEL, h.ADCTime());
	}
	else if (h.TDC() == 10) {
	  ADC_channel->Fill(h.Channel()+Geometry::MAX_TDC_CHANNEL*2, h.ADCTime());
	}
	else if (h.TDC() == 0) {
	  ADC_channel->Fill(h.Channel()+Geometry::MAX_TDC_CHANNEL*3, h.ADCTime());
	}
	else if (h.TDC() == 2) {
	  ADC_channel->Fill(h.Channel()+Geometry::MAX_TDC_CHANNEL*4, h.ADCTime());
	}
      }
    }
  }






				  


  TCanvas* c1 = new TCanvas("c1", "Mean ADC Time");
  c1->cd();
  ADC_drift_time->Draw("colz");

  TCanvas* c2 =new TCanvas("c2", "Drift Time");
  c2->cd();
  driftTime->Draw();

  TCanvas* c3 =new TCanvas("c3", "ADC");
  c3->cd();
  ADCSpectra->Draw();


  TCanvas* c4 = new TCanvas("c4", "Time Slew");
  c4->cd();
  timeSlew->Draw();

  TCanvas* c5 = new TCanvas("c5", "ADC vs Channel");
  c5->cd();
  ADC_channel->GetXaxis()->SetTitle("Tube Number");
  ADC_channel->GetYaxis()->SetTitle("ADC count");
  ADC_channel->Draw("colz");

  TCanvas* c6 = new TCanvas("c6", "Compare two ADC distributions");
  c6->cd();
  toyADC1->GetXaxis()->SetTitle("ADC count");
  toyADC1->GetYaxis()->SetTitle("Number of hits");
  toyADC1->SetLineColor(kBlue);
  toyADC2->SetLineColor(kRed);
  toyADC1->Draw();
  toyADC2->Draw("same");




  gPad->Modified();
  gPad->Update();
}
