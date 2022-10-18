#include <iostream>

#include "MuonReco/IOUtility.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/Geometry.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TH1.h"
#include "TGraph.h"
#include "TF1.h"

using namespace MuonReco;

int main(int argc, char* argv[]) {

  SetAtlasStyle();

  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");
  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry")); // sets global variables
  
  TString systName     = "Nominal";
  Bool_t  hitResiduals = kFALSE;

  TFile* f_tree = TFile::Open(IOUtility::getTrackTreePath(runN, hitResiduals, systName));
  TTree* tree   = (TTree*)(f_tree->Get("trackFitTree"));

  TCanvas* c1 = new TCanvas("c1", "", 800, 600);
  c1->cd();

  TString variable = "(impact_par_opt-hitX)/TMath::Tan(angle_optimized)-hitY";
  TString highAngleCut = "TMath::Abs(angle_optimized*TMath::RadToDeg())>30";

  TFile* f_out = TFile::Open(IOUtility::join(IOUtility::getTomographyDir(runN), "tomography.root"));  
  //TTree* t_out = new TTree("tomography", "");
  TTree* t_out = (TTree*)(f_out->Get("tomography"));
  int hitL = 0;
  int hitC = 0;
  double meanY = 0;
  /*
  t_out->Branch("column", &hitC);
  t_out->Branch("layer",  &hitL);
  t_out->Branch("meanY",  &meanY);

  for (hitL=0; hitL<Geometry::MAX_TUBE_LAYER; ++hitL) {
    for (hitC=0; hitC<Geometry::MAX_TUBE_COLUMN; ++hitC) {
      TString histName = TString::Format("hist_layer%d_column%d", hitL, hitC);
      TString layerColumnCut = TString::Format(" && (hitL==%d) && (hitC==%d)", hitL, hitC);
      tree->Draw(variable + ">>" + histName + "(400,-20,20)", highAngleCut + layerColumnCut);
      TH1* localHist = (TH1*)(gROOT->FindObject(histName));
      localHist->SetTitle(";Track Y position - Wire Y position [mm];Events/0.1mm");
      c1->Print(IOUtility::join(IOUtility::getTomographyDir(runN), 
				TString::Format("y_displacement_layer%d_column%d.png", hitL, hitC)));
      meanY = localHist->GetMean();
      t_out->Fill();
      std::cout << "L: " << hitL << " C: " << hitC << " meanY: " << meanY << std::endl;
    }
  }
  */
  
  // make plots of linear fit to mean Y position vs column
  t_out->SetBranchAddress("layer",  &hitL);
  t_out->SetBranchAddress("column", &hitC);
  t_out->SetBranchAddress("meanY", &meanY);
  for (int iter_hitL = 0; iter_hitL<Geometry::MAX_TUBE_LAYER; ++iter_hitL) {
    int n = t_out->Draw("meanY:column", TString::Format("layer==%d", iter_hitL));
    if (n) {
      std::vector<double> v_c = std::vector<double>();
      std::vector<double> v_y = std::vector<double>();
      for (int i = 0; i < t_out->GetEntries(); ++i) {
	t_out->GetEntry(i);
	if (iter_hitL == hitL && hitC>8 && hitC<62) {
	  v_c.push_back(hitC);
	  v_y.push_back(meanY);
	}
      }
      TGraph* g = new TGraph(v_c.size(), &v_c[0], &v_y[0]);
      g->Fit("pol1");
      g->SetLineWidth(0);
      g->GetFunction("pol1")->SetLineColor(kRed);
      g->SetTitle(";Tube Column;Mean Y position of track [mm]");
      c1->cd();
      g->Draw("AP");
      TPaveText* tpt = new TPaveText(0.7, 0.8, 0.9, 0.9, "NDC");
      tpt->AddText(TString::Format("%.6f * column + %.6f", g->GetFunction("pol1")->GetParameter(1),
				   g->GetFunction("pol1")->GetParameter(0)));
      tpt->Draw();
      c1->Print(IOUtility::join(IOUtility::getTomographyDir(runN),
                                TString::Format("linear_fit_layer%d.png", iter_hitL)));
    }
  }

  /*
  f_out->cd();
  t_out->Write();
  f_out->Write();
  f_out->Close();
  */
  return 0;
}
