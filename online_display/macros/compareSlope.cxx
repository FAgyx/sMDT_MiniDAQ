#include "macros/DecodeRawData.cxx"
#include "src/Geometry.cpp"
#include "macros/AtlasStyle/AtlasLabels.C"
#include "macros/AtlasStyle/AtlasStyle.C"
#include "macros/AtlasStyle/AtlasUtils.C"
#include "src/EventDisplay.cpp"

void PlotT0DataOnTubes(TString filepath, int fitIndex, TString plotTitle, Geometry geo) {
  TH2D* hist      = new TH2D(plotTitle.ReplaceAll(" ", "_"), plotTitle, Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5,
			     Geometry::MAX_TUBE_LAYER, -0.5, Geometry::MAX_TUBE_LAYER-0.5);
  TFile t0File(filepath);
  TString fitVecName;
  TVectorD *fitParams;

  for (Int_t tdc_id = 0; tdc_id < geo.MAX_TDC; tdc_id++) {
    for (Int_t ch_id = 0; ch_id != geo.MAX_TDC_CHANNEL; ch_id++) {
      if (tdc_id == geo.TRIGGER_MEZZ || !geo.IsActiveTDCChannel(tdc_id, ch_id)) {continue;}
      
      fitVecName.Form("FitData_tdc_%d_channel_%d", tdc_id, ch_id);
      fitParams = (TVectorD*)t0File.Get(fitVecName);
      
      int hitL, hitC;
      geo.GetHitLayerColumn(tdc_id, ch_id, &hitL, &hitC);
      hist->Fill(hitC, hitL, (*fitParams)[fitIndex]);
      
    }
  }
  
  static EventDisplay ed = EventDisplay();
  ed.DrawTubeHist(geo, hist, NULL, kTRUE);

  delete hist;
}



void compareSlope(TString inputFilename = "run00187985_20190607.dat") {
  SetAtlasStyle();  
  Geometry geo = Geometry();

  TString fn = TString(inputFilename);
  int runN = ((TObjString*)(TString(fn(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  geo.SetRunN(runN);

  // initialize a vector of TString to hold different types of T0 hists
  vector<TString> files;
  //files.push_back("T0_noCorr.root");
  //files.push_back("T0_ADCCorr.root");
  files.push_back("T0.root");

  vector<int> fillColors;
  fillColors.push_back(40);
  //fillColors.push_back(41);
  fillColors.push_back(31);

  vector<TString> conditions;
  //conditions.push_back("No Correction");
  //conditions.push_back("Signal Slew Correction");
  conditions.push_back("With Slew Corr");





  vector<TH1D*> slopeHists, t0Hists, tmaxHists;
  
  TString t0path = "output/";
  t0path += inputFilename;
  t0path += ".dir/";
  
  // for each file, read in the slopes and histogram them
  TH1D *slopeHist, *t0Hist, *tmaxHist;
  TH2D *TDC_T0    = new TH2D("TDC_T0",     "T0 vs TDC",         50, -150,-100, 11, -0.5, 10.5);
  TH2D *ChanSlope = new TH2D("Chan_Slope", "Channel vs. Slope", 12, -0.5, 11.5, 8, -0.5,  7.5);
  TH2D *tube_t0   = new TH2D("tube_t0",    "T0 vs Tube position", Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5,
			     Geometry::MAX_TUBE_LAYER, -0.5, Geometry::MAX_TUBE_LAYER-0.5);
  char fitVecName[256];
  TVectorD *fitParams;
  int hitlayer, hitcolumn;

  // fill 1D histograms
  for (int i = 0; i < files.size(); i++) {
    slopeHist = new TH1D((TString)("slopes_" + to_string(i)), "T0 Slope with various corrections",       40,    2,   12);
    t0Hist    = new TH1D((TString)("t0_"     + to_string(i)), "T0 with various corrections",             50, -150, -100);
    tmaxHist  = new TH1D((TString)("tmax_"   + to_string(i)), "Max Drift Time with various corrections", 50,  175,  225);
    
    // fill the histogram with many fit params from one calibration condition    
    TFile t0File(t0path + files.at(i));

    PlotT0DataOnTubes(t0path + files.at(i), 1, "T0 of each tube", geo);
    PlotT0DataOnTubes(t0path + files.at(i), 8, "Max Drift Time of each tube", geo);
    PlotT0DataOnTubes(t0path + files.at(i), 13, "Peak ADC of each tube", geo);

    vector<Int_t> tdc_ids = vector<Int_t>();
    tdc_ids.push_back(10);
    //tdc_ids.push_back(8);
    //tdc_ids.push_back(2);
    //tdc_ids.push_back(9);
    //for (Int_t tdc_id : tdc_ids) {
    
      
    for (Int_t tdc_id = 0; tdc_id < geo.MAX_TDC; tdc_id++) {

      for (Int_t ch_id = 0; ch_id != geo.MAX_TDC_CHANNEL; ch_id++) {
	if (tdc_id == geo.TRIGGER_MEZZ || !geo.IsActiveTDCChannel(tdc_id, ch_id)) {continue;}

	sprintf(fitVecName, "FitData_tdc_%d_channel_%d", tdc_id, ch_id);
	fitParams = (TVectorD*)t0File.Get(fitVecName);
	slopeHist->Fill((*fitParams)[3]);
	t0Hist   ->Fill((*fitParams)[1]);
	tmaxHist ->Fill((*fitParams)[8]);

	if (i==0) {
	  TDC_T0->Fill((*fitParams)[1], tdc_id);
	  geo.GetHitLayerColumn(tdc_id, ch_id, &hitlayer, &hitcolumn);
	  ChanSlope->Fill(hitcolumn-24, hitlayer, (*fitParams)[3]);

	  int hitL, hitC;
	  geo.GetHitLayerColumn(tdc_id, ch_id, &hitL, &hitC);
	  tube_t0->Fill(hitC, hitL, (*fitParams)[1]);
	}
      }
    }

    // plot the histogram on the canvas
    slopeHists.push_back(slopeHist);
    t0Hists.push_back(t0Hist);
    tmaxHists.push_back(tmaxHist);
  } // end for: loop over input files

  cout << "a" << endl;
  // do final plotting
  TCanvas* c_slopes = new TCanvas("c_slopes", "T0 Slopes");
  TLegend* l_slopes = new TLegend(0.6, 0.75, 0.9, 0.9);

  c_slopes->cd();
  for (int i = slopeHists.size()-1; i != -1; i--) {
    slopeHists.at(i)->SetFillColorAlpha(fillColors.at(i), 0.8);
    slopeHists.at(i)->SetLineColor(fillColors.at(i));
    slopeHists.at(i)->SetLineWidth(2);
    slopeHists.at(i)->SetFillStyle(3001 + i);
    slopeHists.at(i)->SetStats(0);
    slopeHists.at(i)->GetXaxis()->SetTitle("T0 Slope");
    slopeHists.at(i)->GetYaxis()->SetTitle("Number of Tubes/0.25 ns");
    slopeHists.at(i)->Draw("same");
    l_slopes->AddEntry(slopeHists.at(i), conditions.at(i), "f");
  }
  l_slopes->Draw();
  ATLASLabel(0.2,0.85,"Internal", kBlack);

  TCanvas* c_t0 = new TCanvas("c_t0", "T0 values");
  TLegend* l_t0 = new TLegend(0.6,0.75,0.9,0.9);

  c_t0->cd();
  for (int i = t0Hists.size()-1; i != -1; i--) {
    t0Hists.at(i)->SetFillColorAlpha(fillColors.at(i), 0.8);
    t0Hists.at(i)->SetLineColor(fillColors.at(i));
    t0Hists.at(i)->SetLineWidth(2);
    t0Hists.at(i)->SetFillStyle(3001 + i);
    t0Hists.at(i)->SetStats(0);
    t0Hists.at(i)->GetXaxis()->SetTitle("T0 Value (ns)");
    t0Hists.at(i)->GetYaxis()->SetTitle("Number of Tubes/ns");
    t0Hists.at(i)->SetStats(1);
    t0Hists.at(i)->SetTitle("TDC 10");
    t0Hists.at(i)->Draw("same");
    l_t0->AddEntry(t0Hists.at(i), conditions.at(i), "f");
  }
  //l_t0->Draw();
  //ATLASLabel(0.2,0.85,"Internal", kBlack);

  TCanvas* c_tmax = new TCanvas("c_tmax", "Maximum Drift Times");
  TLegend* l_tmax = new TLegend(0.6,0.75,0.9,0.9);

  c_tmax->cd();
  for (int i = tmaxHists.size()-1; i != -1; i--) {
    tmaxHists.at(i)->SetFillColorAlpha(fillColors.at(i), 0.8);
    tmaxHists.at(i)->SetLineColor(fillColors.at(i));
    tmaxHists.at(i)->SetLineWidth(2);
    tmaxHists.at(i)->SetFillStyle(3001 + i);
    tmaxHists.at(i)->SetStats(0);
    tmaxHists.at(i)->GetXaxis()->SetTitle("Max Drift Time (ns)");
    tmaxHists.at(i)->GetYaxis()->SetTitle("Number of Tubes/ns");
    tmaxHists.at(i)->Draw("same");
    l_tmax->AddEntry(t0Hists.at(i), conditions.at(i), "f");
  }
  l_tmax->Draw();
  ATLASLabel(0.2,0.85,"Internal", kBlack);

  TCanvas* c_tdc_t0 = new TCanvas("c_tdc_t0", "TDC vs. T0");
  c_tdc_t0->cd();
  TDC_T0->GetXaxis()->SetTitle("T0 (ns)");
  TDC_T0->GetYaxis()->SetTitle("TDC Number");
  TDC_T0->Draw("colz");

  TCanvas* c_chan_slope = new TCanvas("c_chan_slope", "T0 Slope vs. Channel");
  c_chan_slope->cd();

  ChanSlope->Draw("colz");





  if (tube_t0->GetMaximum() == 0) {
    tube_t0->SetMaximum(tube_t0->GetMaximum(0));
  }
  if (tube_t0->GetMinimum() == 0) {
    tube_t0->SetMinimum(tube_t0->GetMinimum(0));
  }



  static EventDisplay ed = EventDisplay();  
  ed.DrawTubeHist(geo, tube_t0);

  TCanvas* c_tube_t0 = new TCanvas("t_tube_t0", "T0 vs tube location");
  c_tube_t0->cd();
  tube_t0->Draw("colz");


  TH2D* isActive = new TH2D("isActive",    "T0 vs Tube position", Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5, Geometry::MAX_TUBE_LAYER, -0.5, Geometry::MAX_TUBE_LAYER-0.5);
  int hitL, hitC;
  for (int TDC = 0; TDC < Geometry::MAX_TDC; TDC++) {
    for (int Chan = 0; Chan < Geometry::MAX_TDC_CHANNEL; Chan++) {
      geo.GetHitLayerColumn(TDC, Chan, &hitL, &hitC);
      isActive->SetBinContent(hitC, hitL, geo.IsActiveTDC(TDC));
    }
  }
  ed.DrawTubeHist(geo, tube_t0);

}

