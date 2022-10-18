#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "TROOT.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "MuonReco/TrackParam.h"
#include "MuonReco/Optimizer.h"
#include "MuonReco/Event.h"
#include "MuonReco/Parameterization.h"
#include "MuonReco/EventDisplay.h"
#include "MuonReco/RTParam.h"
#include "MuonReco/ResolutionResult.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/RTLinInterpParam.h"
#include "MuonReco/RTAggregator.h"

using namespace MuonReco;

int main(int argc, char* argv[]) {

  SetAtlasStyle();
  
  ArgParser    ap  = ArgParser(argc, argv);
  ConfigParser cp  = ConfigParser(ap.getTString("--conf"));
  int runN         = cp.items("General").getInt("RunNumber");
  int minEvent     = cp.items("AutoCalibration").getInt("MinEvent", 0, 0);
  int nEvents      = cp.items("AutoCalibration").getInt("NEvents",  100000, 0);
  int isMC         = cp.items("General").getInt("IsMC");
  int nRT          = cp.items("AutoCalibration").getInt("NRT", 1, 0);
  double maxresid  = cp.items("AutoCalibration").getDouble("MaxResidual", 5.0);
  double tolerance = cp.items("AutoCalibration").getDouble("Tolerance", 0.001);
  bool constrainZero = cp.items("AutoCalibration").getInt("ConstrainZero", 0, 0);
  bool constrainEndpoint = cp.items("AutoCalibration").getInt("ConstrainEndpoint", 0, 0);
  bool useCorrection = kTRUE;
  int maxIter = (isMC) ? 0 : 10;

  if (ap.hasKey("--minEvent")) minEvent = ap.getInt("--minEvent");
  if (ap.hasKey("-t")) useCorrection = kFALSE;
  gROOT->SetBatch(kTRUE);
  
  Geometry geo = Geometry(cp);

  // get data from one event and store in a new tree
  TFile  f(IOUtility::getDecodedOutputFilePath(runN));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);

  TString name;

  Optimizer* rtp;
  Callable*  rt_interface;
  TrackParam tp  = TrackParam();
  TString algorithm = cp.items("AutoCalibration").getStr("Parameterization", "Chebyshev");
  if (!algorithm.CompareTo("Chebyshev") && nRT==1) {
    rtp = new RTParam(cp);
    ((RTParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
    ((RTParam*)rtp)->LoadTxt("raw/Rt_BMG_6_1.dat");
    ((RTParam*)rtp)->SetUseCorrection(useCorrection);
    ((RTParam*)rtp)->constrainZero = constrainZero;
    ((RTParam*)rtp)->constrainEndpoint = constrainEndpoint;
    rt_interface = rtp;
    ((RTParam*)rtp)->SetName("all");
  }
  else if (!algorithm.CompareTo("Chebyshev") && nRT > 1) {
    rt_interface = new RTAggregator();
    for (size_t irt=0; irt<nRT; ++irt) {
      std::cout << "NUmber " << irt << std::endl;
      Optimizer* localrtp = new RTParam(cp);
      ((RTParam*)localrtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
      ((RTParam*)localrtp)->LoadTxt("raw/Rt_BMG_6_1.dat");
      ((RTParam*)localrtp)->SetUseCorrection(useCorrection);
      ((RTParam*)localrtp)->constrainZero = constrainZero;
      ((RTParam*)localrtp)->constrainEndpoint = constrainEndpoint;
      localrtp->setVerbose(kTRUE);
      localrtp->setTarget(t);
      localrtp->setMaxResidual(maxresid);
      localrtp->setRange(minEvent, minEvent+nEvents);
      localrtp->setMaxIteration(maxIter);
      localrtp->addDependency(&tp);
      ((RTParam*)localrtp)->SetName(TString::Format("%d", irt));
      ((RTAggregator*)rt_interface)->AddRT((RTParam*)localrtp);
      if (irt != 0) ((RTAggregator*)rt_interface)->rts.at(0)->addSimultaneous(localrtp);
    }
    rtp = ((RTAggregator*)rt_interface)->rts.at(0);
    ((RTAggregator*)rt_interface)->InitMaxDriftTime(geo);
  }
  else if (!algorithm.CompareTo("LinearInterpolation")) {
    rtp = new RTLinInterpParam(cp);
    ((RTLinInterpParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
  }
  else {
    std::cout << "Parameterization algorithm must be either Chebyshev, LinearInterpolation" << std::endl;
    return 1;
  }
  
  tp.SetRT(rt_interface);
  tp.setTarget(t);

  EventDisplay ed = EventDisplay();
  ed.SetRT(rtp);

  rtp->addDependency(&tp);
  
  tp.setMaxResidual(maxresid);
  tp.setVerbose(kFALSE);

  rtp->setVerbose(kTRUE);
  rtp->setTarget(t);
  rtp->setMaxResidual(maxresid);
  rtp->setRange(minEvent, minEvent+nEvents);
  rtp->setMaxIteration(maxIter);
  rtp->setTolerance(tolerance);
  rtp->Print();
  rtp->optimize();
  rtp->Print();

  
  TFile out(IOUtility::getAutoCalFilePath(runN, minEvent, nEvents), "RECREATE");
  if (!algorithm.CompareTo("Chebyshev") && nRT == 1) {
    ((RTParam*)rtp)->Write();
    ((RTParam*)rtp)->SaveImage(IOUtility::getAutoCalOutputDir(runN));
  }
  else if (!algorithm.CompareTo("Chebyshev") && nRT>1) {
    for (size_t irt=0; irt<nRT; ++irt) {
      RTParam* localrtp = ((RTParam*)((RTAggregator*)rt_interface)->rts.at(irt));
      localrtp->Write(localrtp->GetName());
      localrtp->SaveImage(IOUtility::getAutoCalOutputDir(runN));
    }
  }
  else if (!algorithm.CompareTo("LinearInterpolation")) {
    ((RTLinInterpParam*)rtp)->Write();
    ((RTLinInterpParam*)rtp)->SaveImage(IOUtility::getAutoCalOutputDir(runN));
  }
  out.Write();
  out.Close();
}
