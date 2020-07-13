
#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "TROOT.h"

#include "MuonReco/TrackParam.h"
#include "MuonReco/Optimizer.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Event.h"
#include "MuonReco/Parameterization.h"
#include "MuonReco/EventDisplay.h"
#include "MuonReco/RTParam.h"
#include "MuonReco/ResolutionResult.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"

using namespace MuonReco;

int main(int argc, char* argv[]) {
  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));

  TString inputFileName = cp.items("General").getStr("RawFileName");

  gROOT->SetBatch(kTRUE);

  // get data from one event and store in a new tree
  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);

  Geometry   geo = Geometry();
  geo.Configure(cp.items("Geometry"));

  TString name;

  RTParam rtp = RTParam(geo);
  rtp.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp.LoadTxt("raw/Rt_BMG_6_1.dat");

  
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
  rtp.setRange(0, 100000);
  rtp.optimize();
  
  TFile out(IOUtility::getAutoCalFilePath(inputFileName), "RECREATE");
  rtp.Write();
  out.Write();
  out.Close();
  rtp.SaveImage(IOUtility::getAutoCalOutputDir(inputFileName));

}
