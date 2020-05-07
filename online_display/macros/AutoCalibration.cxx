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

void AutoCalibration(TString inputFileName = "run00187985_20190607.dat") {
  //gROOT->SetBatch(kTRUE);


  // get data from one event and store in a new tree
  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);

  Geometry   geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  TString name;

  RTParam rtp = RTParam(geo);
  rtp.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp.LoadTxt("src/Rt_BMG_6_1.dat");

  
  TrackParam tp  = TrackParam(geo);
  tp.SetRT(&rtp);
  tp.setVerbose(kTRUE);
  tp.setTarget(t);

  EventDisplay ed = EventDisplay();
  ed.SetRT(&rtp);

  rtp.addDependency(&tp);
  
  tp.setMaxResidual(3.0);
  tp.setVerbose(kFALSE);

  rtp.setVerbose(kFALSE);
  rtp.setTarget(t);
  rtp.setMaxResidual(3.0);
  rtp.setRange(0, 10000);
  rtp.optimize();
  
  TFile out(IOUtility::getAutoCalFilePath(inputFileName), "RECREATE");
  rtp.Write();
  out.Write();
  out.Close();
  rtp.SaveImage(IOUtility::getAutoCalOutputDir(inputFileName));

}
