#include <iostream>

#include "MuonReco/RTParam.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/RTLinInterpParam.h"
#include "MuonReco/Optimizer.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/ResolutionResult.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TRandom.h"
#include "TMath.h"

int main(int argc, char* argv[]) {

  SetAtlasStyle();
  
  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");
  int minEvent    = cp.items("AutoCalibration").getInt("MinEvent", 0, 0);
  int nEvents     = cp.items("AutoCalibration").getInt("NEvents",  100000, 0);

  TFile autocal(IOUtility::getAutoCalFilePath(runN, minEvent+nEvents, nEvents));
  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry"));

  Optimizer* rtp;
  TString algorithm = cp.items("AutoCalibration").getStr("Parameterization", "Chebyshev");
  if (!algorithm.CompareTo("Chebyshev")) {
    rtp = new RTParam(cp);
    ((RTParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
    ((RTParam*)rtp)->Load(&autocal);
    ((RTParam*)rtp)->Print();
  }
  else if (!algorithm.CompareTo("LinearInterpolation")) {
    rtp = new RTLinInterpParam(cp);
    ((RTLinInterpParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
    ((RTLinInterpParam*)rtp)->Load(&autocal);
    ((RTLinInterpParam*)rtp)->Print();
  }
  else {
    std::cout << "Parameterization algorithm must be either Chebyshev, LinearInterpolation" << std::endl;
    return 1;
  }

  ResolutionResult* rr = new ResolutionResult(cp);
  rr->SetIsMC(1);

  TH1D* h_sigPropRes = new TH1D("h_sigPropRes", "; Signal Propagation Residual [#mu m];Number of hits", 100, -150, 150);

  TFile f_out("tree.root", "RECREATE");
  TTree* spTree = new TTree("spTree", "Tree holding signal propagation information");

  int nResiduals = 10000;
  
  Bool_t accepted = kFALSE;
  double _x, _y, theta, nom_radius, nom_ntime, r_nom_time, 
    layer_height, total_height, dummy, sigPropDist, sign;
  double sigPropSpeed = 0.77*0.3; // in m per nanosecond
  double sigPropTime, sigPropResidual;
  geo.GetHitXY(Geometry::MAX_TUBE_LAYER-1,0, &dummy, &total_height);
  double midplane_height = total_height/2.0;
  int iLayer, iColumn = 25;

  spTree->Branch("nom_radius", &nom_radius);
  spTree->Branch("nom_ntime", &nom_ntime);
  spTree->Branch("r_nom_time", &r_nom_time);
  spTree->Branch("sigPropDist", &sigPropDist);
  spTree->Branch("sigPropTime", &sigPropTime);
  spTree->Branch("sigPropResidual", &sigPropResidual);
  spTree->Branch("theta", &theta);
  spTree->Branch("iLayer", &iLayer);

  for (int i = 0; i < nResiduals; i++) {
    accepted = kFALSE;
    
    while (!accepted) {
      _x = (gRandom->Uniform()-0.5)*TMath::Pi()*2*0.42;
      _y = gRandom->Uniform();
      if (TMath::Cos(_x)*TMath::Cos(_x) > _y) {
	accepted = kTRUE;
	theta = _x;
      }
    }

    for (iLayer = 0; iLayer < Geometry::MAX_TUBE_LAYER; iLayer++ ) {
      nom_radius = gRandom->Uniform()*Geometry::max_drift_dist;
      nom_ntime  = ((RTParam*)rtp)->GetFunction()->GetX(nom_radius);

      geo.GetHitXY(iLayer, 0, &dummy, &layer_height);
      sigPropDist = (layer_height - midplane_height)/1000.0*TMath::Tan(theta);
      sigPropTime = sigPropDist/sigPropSpeed;

      r_nom_time = ((RTParam*)rtp)->Eval(nom_ntime);
      sign = (gRandom->Uniform() < 0.5) ? 1.0 : -1.0;
      sigPropResidual = (r_nom_time - ((RTParam*)rtp)->Eval(nom_ntime - sigPropTime/100))*1000.0*sign;
      spTree->Fill();
      h_sigPropRes->Fill(sigPropResidual);
      rr->FillResidualByValue(sigPropResidual, nom_radius);
    } // end for: each layer in this event
    
  } // end for: simulated events

  spTree->CloneTree()->Write();
  f_out.Write();
  f_out.Close();

  TCanvas* c1 = new TCanvas("c1", "", 800, 600);
  c1->cd();
  c1->SetLeftMargin(0.15);
  h_sigPropRes->Draw();
  c1->Print("SigPropRes.png");

  TFile out(IOUtility::getFitResidualPath(13, "Nominal"), "RECREATE");
  rr->Write();
  out.Write();
  out.Close();

  return 0;
}
