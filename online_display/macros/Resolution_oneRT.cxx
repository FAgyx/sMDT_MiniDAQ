#include "src/TrackParam.cpp"
#include "src/Optimizer.cpp"
#include "src/Geometry.cpp"
#include "src/Event.cpp"
#include "src/Parameterization.cpp"
#include "src/EventDisplay.cpp"
#include "src/RTParam.cpp"
#include "src/ResolutionResult.cpp"
#include "src/IOUtility.cpp"

using namespace Muon;

void Resolution(TString inputFileName = "run00187985_20190607.dat", Bool_t hitResiduals = kFALSE) {
  //gROOT->SetBatch(kTRUE);

  // get data from one event and store in a new tree
  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);

  TFile autocal(IOUtility::getAutoCalFilePath(inputFileName));

  Geometry   geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  TString name;

  RTParam rtp = RTParam(geo);
  rtp.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp.Load(&autocal);
  //rtp.LoadTxt("src/Rt_BMG_6_1.dat");
  rtp.Draw();

  TrackParam tp  = TrackParam(geo);
  tp.SetRT(&rtp);
  tp.setVerbose(kFALSE);
  tp.setTarget(t);

  EventDisplay ed = EventDisplay();
  ed.SetRT(&rtp);

  rtp.addDependency(&tp);
  rtp.setTarget(t);
  rtp.setRangeSingle(0);
  rtp.setMaxResidual(1);

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
    cout << i << endl;
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

}
