#include "src/TrackParam.cpp"
#include "src/Optimizer.cpp"
#include "src/Geometry.cpp"
#include "src/Event.cpp"
#include "src/Parameterization.cpp"
#include "src/EventDisplay.cpp"
#include "src/RTParam.cpp"
#include "src/ResolutionResult.cpp"
#include "src/IOUtility.cpp"
#include "src/RTAggregator.cpp"

using namespace Muon;

void TwoRTAutoCalibration(TString inputFileName = "run00187985_20190607.dat") {
  gROOT->SetBatch(kTRUE);



  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);

  Geometry   geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  TString name;

  RTParam rtp_1 = RTParam(geo);
  rtp_1.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));


  
  TrackParam tp  = TrackParam(geo);
  tp.setVerbose(kFALSE);
  tp.setTarget(t);

  rtp_1.setVerbose(kTRUE);
  rtp_1.addDependency(&tp);
  rtp_1.setMaxResidual(3.0);

  tp.setMaxResidual(3.0);
  tp.setVerbose(kFALSE);

  RTParam rtp_2 = RTParam(geo);
  rtp_2.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp_2.setVerbose(kTRUE);
  rtp_2.addDependency(&tp);
  rtp_2.setMaxResidual(3.0);




  RTAggregator rta = RTAggregator();
  rta.AddRT(&rtp_1);
  rta.AddRT(&rtp_2);
  rta.InitMultiLayer(geo);
  

  tp.SetRT(&rta);

  rtp_1.addSimultaneous(&rtp_2);
  rtp_1.setTarget(t);
  rtp_1.setRange(0, 100000);
  rtp_1.optimize();

  
  TFile out(IOUtility::getAutoCalFilePath(inputFileName), "RECREATE");
  rtp_1.Write("ML0");
  rtp_2.Write("ML1");
  out.Write();
  out.Close();
  rtp_1.SaveImage(IOUtility::getAutoCalOutputDir(inputFileName));

  
}
