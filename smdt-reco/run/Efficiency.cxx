#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/Observable.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TLegend.h"

#include <iostream>

using namespace MuonReco;

Observable efficiency(int runN, TTree* tree, double maxR=Geometry::column_distance/2, 
		      int Layer=-1, TH1D* layerHist=0, int Column=-1, int* nHit=0, int* nMiss=0) {
  int nBins = 25;
  TH1D* h_hits = new TH1D("h_hits", "hits",   nBins, 0, Geometry::max_drift_dist);
  TH1D* h_miss = new TH1D("h_miss", "misses", nBins, 0, Geometry::max_drift_dist);
  TH1D* h_eff  = new TH1D("h_eff",  ";Drift radius [mm];Efficiency", nBins, 0, Geometry::max_drift_dist);
  TCanvas* c1 = new TCanvas("c1", "", 800, 800);

  TString cutStr1 = TString::Format("trackHitR<%f", maxR);
  TString cutStr2 = TString::Format("missedHitR<%f", maxR);
  TString outfile = "efficiency";
  if (Layer != -1 && Layer >= 0 && Layer < Geometry::MAX_TUBE_LAYER) {
    cutStr1 += TString::Format(" && trackHitL == %i", Layer);
    cutStr2 += TString::Format(" && missedHitL == %i", Layer);
    outfile += TString::Format("_layer_%i", Layer);
  }
  if (Column != -1 && Column >= 0 && Column < Geometry::MAX_TUBE_COLUMN) {
    cutStr1 += TString::Format(" && trackHitC == %i", Column);
    cutStr2 += TString::Format(" && missedHitC == %i", Column);
    outfile += TString::Format("_column_%i", Column);
  }
  outfile += ".png";
  tree->Draw("trackHitR>>h_hits", cutStr1);
  tree->Draw("missedHitR>>h_miss", cutStr2);
  std::cout << "Track cut string:  " << cutStr1 << std::endl;
  std::cout << "Missed cut string: " << cutStr2 << std::endl;
  Observable tothit = Observable(h_hits->GetEntries(),
				 TMath::Sqrt(h_hits->GetEntries()));
  Observable totmiss = Observable(h_miss->GetEntries(),
				  TMath::Sqrt(h_miss->GetEntries()));
  Observable toteff = tothit / (tothit + totmiss);
  toteff.Print();

  if (nHit)  *nHit  = tothit.val;
  if (nMiss) *nMiss = totmiss.val;

  h_hits->Sumw2();
  h_miss->Sumw2();
  h_miss->Add(h_hits);
  std::cout << "Entries: " << h_hits->GetEntries() << ", " << h_miss->GetEntries() << std::endl;
  h_eff->Divide(h_hits, h_miss, 1, 1, "B");
  h_eff->SetStats(0);
  h_eff->Draw();
  c1->Print(IOUtility::join(IOUtility::getEfficiencyOutputDir(runN), outfile));

  if (layerHist) {
    layerHist->SetBinContent(8-Layer, toteff.val);
    layerHist->SetBinError  (8-Layer, toteff.err);
  }

  delete c1;
  delete h_eff;
  delete h_miss;
  delete h_hits;
  return toteff;
}

int main(int argc, char* argv[]) {

  SetAtlasStyle();

  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");

  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry"));
  
  Bool_t hitResiduals;
  if (ap.hasKey("-h"))
    hitResiduals = kTRUE;
  else
    hitResiduals = kFALSE;
  Bool_t floatUp = kTRUE;
  if (ap.hasKey("-d")) {
    floatUp = kFALSE;
  }
  Bool_t biasSyst = kFALSE;
  if (ap.hasKey("-b")) {
    biasSyst = kTRUE;
  }
  TString systName     = "Nominal";
  if (ap.hasKey("--syst")) {
    systName  = ap.getTString("--syst");
    if (!systName.CompareTo("mcs")) {}
    else {
      if (!biasSyst) {
	if (floatUp) systName += "_up";
	else systName += "_down";
      }
      else systName += "_bias";
    }
  }
  TFile* f_tree = TFile::Open(IOUtility::getTrackTreePath(runN, hitResiduals, systName), "READ");
  TTree* tree = 0;
  f_tree->GetObject("trackFitTree", tree);
  
  Observable toteff = efficiency(runN, tree);

  TF1* f_const = new TF1("f_const", "[0]", -0.5,7.5);
  f_const->SetLineColor(kRed);

  TH1D* h_layerEff = new TH1D("h_layerEff", ";Number Layers Penetrated;Efficiency", 8, -0.5, 7.5);
  for (int iL = 0; iL < Geometry::MAX_TUBE_LAYER; iL++) {
    efficiency(runN, tree, 7.1, iL, h_layerEff);
  }
  TCanvas* c2 = new TCanvas("c2", "", 800, 800);
  c2->cd();
  c2->SetLeftMargin(0.2);
  h_layerEff->SetStats(0);
  //h_layerEff->Fit(f_const);  
  h_layerEff->GetYaxis()->SetTitleOffset(2.0);
  h_layerEff->Draw();

  std::cout << "Layer efficiency: " << std::endl;
  toteff.Print();
  toteff = efficiency(runN, tree, 7.1);
  std::cout << "Tube efficiency: " << std::endl;
  toteff.Print();
  // draw legend and error bars
  double x[2] = {-0.5,7.5};
  double y[2] = {toteff.val, toteff.val};
  double ex[2] = {0,0};
  double ey[2] = {toteff.err, toteff.err};
  TGraphErrors* gr = new TGraphErrors(2, x, y, ex, ey);
  gr->SetLineColor(9);
  gr->SetLineWidth(2);
  gr->SetFillColorAlpha(29, 0.5);
  gr->SetFillStyle(1001);
  gr->Draw("E3 L same");

  auto leg = new TLegend(0.2,0.2,0.6,0.45);
  leg->AddEntry(gr, TString::Format("#splitline{Best fit efficiency}{%.3f#pm%.3f}", 
				    toteff.val, toteff.err), "lf");
  leg->Draw();

  c2->Print(IOUtility::join(IOUtility::getEfficiencyOutputDir(runN), "layerEfficiency.png"));

  
  // save total efficiency to a file
  Observable tempEff = Observable(0, 0);
  int layer, column, nHits, nMiss;
  double tEff;
  TFile* f_output = TFile::Open(IOUtility::join(IOUtility::getEfficiencyOutputDir(runN), "tubeEfficiency.root"), "RECREATE");
  TTree* tube_efficiency = new TTree("tube_efficiency", "tube_efficiency");
  tube_efficiency->Branch("Layer", &layer);
  tube_efficiency->Branch("Column", &column);
  tube_efficiency->Branch("nHits", &nHits);
  tube_efficiency->Branch("nMiss", &nMiss);
  tube_efficiency->Branch("efficiency", &tEff);
  
  for (layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
    for (column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
      tempEff = efficiency(runN, tree, 7.1, layer, 0, column, &nHits, &nMiss);
      tEff = tempEff.val;
      tube_efficiency->Fill();
    }
  }

  tube_efficiency->Write();
  f_output->Close();

  toteff = efficiency(runN, tree);
  toteff.Write(IOUtility::join(IOUtility::getEfficiencyOutputDir(runN), "totalEfficiency.root"));
}
