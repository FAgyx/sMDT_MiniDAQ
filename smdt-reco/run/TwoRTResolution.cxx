#include <iostream>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include "MuonReco/TrackParam.h"
#include "MuonReco/Optimizer.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Event.h"
#include "MuonReco/Parameterization.h"
#include "MuonReco/EventDisplay.h"
#include "MuonReco/RTParam.h"
#include "MuonReco/ResolutionResult.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/RTAggregator.h"

using namespace MuonReco;

int main(int argc, char* argv[]) {

  //gROOT->SetBatch(kTRUE);

  TString inputFileName = "run00187985_20190607.dat";
  Bool_t hitResiduals = kFALSE;


  // get data from one event and store in a new tree
  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);

  TFile autocal(IOUtility::getAutoCalFilePath(inputFileName));

  Geometry   geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  TString name;

  RTParam rtp_1 = RTParam(geo);
  rtp_1.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp_1.Load(&autocal, "ML0");

  RTParam rtp_2 = RTParam(geo);
  rtp_2.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp_2.Load(&autocal, "ML1");

  RTAggregator rta = RTAggregator();
  rta.AddRT(&rtp_1);
  rta.AddRT(&rtp_2);
  rta.InitMultiLayer(geo);

  TrackParam tp  = TrackParam(geo);
  tp.SetRT(&rta);
  tp.setVerbose(kFALSE);
  tp.setTarget(t);

  ResolutionResult* rr = new ResolutionResult(geo);
  tp.setMaxResidual(3.0); // in sigma
  int maxhit;


  for (int i = 0; i < 100000; i++) {
    tp.setTarget(t);
    tp.setRangeSingle(i);
    tp.setIgnoreNone();
    tp.optimize();

    if (hitResiduals) {
      if (tp.getDOF() >= 4) rr->FillChiSq(tp);
      maxhit = tp.getMaxHitIndex();
      for (int hit = 0; hit < maxhit; hit++) {
	tp.setIgnoreSingle(hit);
	tp.optimize(kFALSE);
	if (tp.getDOF() >= 3) {
	  rr->FillResiduals(tp);
	}
      }
    }
    else {
      if (tp.getDOF() >= 4) {
	rr->FillChiSq    (tp);
	rr->FillResiduals(tp);
      }
    }
    std::cout << i << std::endl;
  }


  rr->Draw();
  rr->DrawADCPlots(IOUtility::getT0FilePath(inputFileName));

  if (hitResiduals) {
    TFile out(IOUtility::getHitResidualPath(inputFileName), "RECREATE");
    rr->Write();
    out.Write();
    out.Close();
  }
  else {
    TFile out(IOUtility::getFitResidualPath(inputFileName), "RECREATE");
    rr->Write();
    out.Write();
    out.Close();
  }
  rr->SaveImages(IOUtility::getResolutionOutputDir(inputFileName));

  return 0;
}