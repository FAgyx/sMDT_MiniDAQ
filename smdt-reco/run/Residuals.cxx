#include <iostream>

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

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "TString.h"
#include "TFile.h"
#include "TTree.h"

using namespace MuonReco;

int main(int argc, char* argv[]) {

  SetAtlasStyle();

  ArgParser    ap    = ArgParser(argc, argv);
  ConfigParser cp    = ConfigParser(ap.getTString("--conf"));
  int runN           = cp.items("General").getInt("RunNumber");
  Bool_t isMC        = cp.items("General").getBool("IsMC", 0, 0);
  int minHits        = cp.items("RecoUtility").getInt("MIN_HITS_NUMBER");
  int maxHits        = cp.items("RecoUtility").getInt("MAX_HITS_NUMBER");
  int minEventAC     = cp.items("AutoCalibration").getInt("MinEvent", 0, 0);
  int nEventsAC      = cp.items("AutoCalibration").getInt("NEvents",  100000, 0);
  int nRT            = cp.items("AutoCalibration").getInt("NRT", 1, 0);
  int minEvent       = (ap.hasKey("--minEvent")) ? ap.getInt("--minEvent") : cp.items("Residuals").getInt("MinEvent", 0, 0);
  int nEvents        = cp.items("Residuals").getInt("NEvents", 100000, 0);
  int useResForSysts = cp.items("Residuals").getInt("UseResForSysts", 0, 0);
  double maxresid    = cp.items("Residuals").getDouble("MaxResidual", 5.0);
  if (useResForSysts){// && ap.hasKey("--syst")) {
    if (gSystem->AccessPathName(IOUtility::getResolutionCurveFile(runN))) {
      std::cout << "WARNING: UNABLE TO LOAD FILE: " << std::endl;
      std::cout << IOUtility::getResolutionCurveFile(runN) << std::endl;
    }
    else {
      Hit::LoadRadiusErrorFunc(IOUtility::getResolutionCurveFile(runN));
    }
  }
  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry")); // sets global variables
  Bool_t hitResiduals;
  if (ap.hasKey("-h")) 
    hitResiduals = kTRUE;
  else 
    hitResiduals = kFALSE;
  Bool_t doTimeSlew = kFALSE;
  if (ap.hasKey("-t")) {
    doTimeSlew = kTRUE;
    std::cout << "Doing time slew correction" << std::endl;
  }
  Bool_t floatUp = kTRUE;
  if (ap.hasKey("-d")) {
    floatUp = kFALSE;
  }
  Bool_t biasSyst = kFALSE;
  if (ap.hasKey("-b")) {
    biasSyst = kTRUE;
  }
  Bool_t efficiency = kFALSE;
  if (ap.hasKey("-e")) {
    efficiency = kTRUE;
  }

  Bool_t anySystematic = kFALSE;
  Bool_t MCS = kFALSE; // monte carlo multiple coulomb scattering
  TString systName     = "Nominal";
  int systIndex = 0;
  int partitionIndex = 0;
  double approxError = 0;
  if (ap.hasKey("--syst")) {
    systName  = ap.getTString("--syst");
    if (!systName.CompareTo("t0") || !systName.CompareTo("slope") || 
	!systName.CompareTo("intercept") || !systName.CompareTo("slew") || 
	!systName.CompareTo("sigProp")) {

      if (!systName.CompareTo("t0")) {
	systIndex = TrackParam::DELTAT0;
	approxError = 10.0;
	ResolutionResult::MAXSHIFT = (floatUp) ? 4 : -4;
	ResolutionResult::SYSTNAME = "#Delta t_{0} [ns]";
      }
      else if (!systName.CompareTo("slope")) {
	systIndex = TrackParam::THETA;
	approxError = 0.005;
	ResolutionResult::MAXSHIFT = (floatUp) ? 1.5 : -1.5;
	ResolutionResult::SYSTNAME = "#theta [mrad]";
	ResolutionResult::SYSTSF   = 1000;
      }
      else if (!systName.CompareTo("intercept")) {
	systIndex = TrackParam::INTERCEPT;
	approxError = 0.1;
	ResolutionResult::MAXSHIFT = (floatUp) ? 0.1 : -0.1;
	ResolutionResult::SYSTNAME = "impact parameter [mm]";
      }
      else if (!systName.CompareTo("slew")) {
	systIndex = TrackParam::SLEWFACTOR;
	approxError = 0.25;
      }
      else if (!systName.CompareTo("sigProp")) {
	systIndex = TrackParam::SIGPROPFACTOR;
	approxError = 1.0;
      }

      if (!biasSyst) {
	if (floatUp) systName += "_up";
	else systName += "_down";
      }
      else systName += "_bias";
      anySystematic = kTRUE;
    }
    else if (!systName.CompareTo("mcs")) {
      MCS = kTRUE;
    }
    else if (systName.Contains("Partition")) {
      partitionIndex = TString(systName(systName.Length()-1, systName.Length())).Atoi();
    }
  }


  //gROOT->SetBatch(kTRUE);

  // get data from one event and store in a new tree
  TFile  f(IOUtility::getDecodedOutputFilePath(runN));
  TTree* t = (TTree*)f.Get(IOUtility::EVENT_TREE_NAME);
  TFile autocal(IOUtility::getAutoCalFilePath(runN, minEventAC+partitionIndex*nEventsAC, nEventsAC));

  Optimizer* rtp;
  Callable* rt_interface;
  TString algorithm = cp.items("AutoCalibration").getStr("Parameterization", "Chebyshev");
  if (!algorithm.CompareTo("Chebyshev") && nRT==1) {
    rtp = new RTParam(cp);
    ((RTParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
    ((RTParam*)rtp)->Load(&autocal);
    ((RTParam*)rtp)->Print();
    if (doTimeSlew) ((RTParam*)rtp)->SetUseCorrection(kFALSE);
    rt_interface = rtp;
  }
  else if (!algorithm.CompareTo("Chebyshev") && nRT > 1) {
    rt_interface = new RTAggregator();
    for (size_t irt=0; irt<nRT; ++irt) {
      Optimizer* localrtp = new RTParam(cp);
      ((RTParam*)localrtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
      ((RTParam*)localrtp)->SetName(TString::Format("%d", irt));
      ((RTParam*)localrtp)->Load(&autocal, ((RTParam*)localrtp)->GetName());
      ((RTAggregator*)rt_interface)->AddRT((RTParam*)localrtp);
    }
    ((RTAggregator*)rt_interface)->InitMaxDriftTime(geo);
  }
  else if (!algorithm.CompareTo("LinearInterpolation")) {
    rtp = new RTLinInterpParam(cp);
    ((RTLinInterpParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
    ((RTLinInterpParam*)rtp)->Load(&autocal);
    ((RTLinInterpParam*)rtp)->Print();
    rt_interface = rtp;
  }
  else {
    std::cout << "Parameterization algorithm must be either Chebyshev, LinearInterpolation" << std::endl;
    return 1;
  }

  TrackParam tp  = TrackParam();
  tp.SetRT(rt_interface);
  tp.setVerbose(kFALSE);
  tp.setTarget(t);

  EventDisplay ed = EventDisplay();
  ed.SetRT(rt_interface);
  ed.SetOutputDir(IOUtility::join(IOUtility::getRunOutputDir(runN), "events_with_tracks", kTRUE));

  ResolutionResult* rr = new ResolutionResult(cp);
  rr->SetDoMCMCS(MCS);
  tp.setMaxResidual(maxresid); // in sigma
  int maxhit;

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  
  int event_print = 1;
  int maxEntry = (minEvent+nEvents<t->GetEntries()) ? minEvent+nEvents : t->GetEntries();
  std::cout << "Calculating residuals for " << maxEntry-minEvent << " events [" << minEvent << ", " << maxEntry << "]" << std::endl;
  for (int i = minEvent; i < maxEntry; i++) {
    tp.setTarget(t);
    tp.setRangeSingle(i);
    tp.setIgnoreNone();
    tp.optimize();
    
    
    if (hitResiduals) {
      maxhit = tp.getMaxHitIndex();
      for (int hit = 0; hit < maxhit; hit++) {
	tp.setIgnoreSingle(hit);
	tp.optimize(kFALSE);

	if (tp.getDOF() >= minHits-TrackParam::NPARS-1 && tp.getDOF() <= maxHits-1) {
	  if (anySystematic) tp.pullParameter(systIndex, 1.0, floatUp, approxError, biasSyst);
	  rr->FillChiSq    (tp);
	  rr->FillResiduals(tp);
	}
	tp.RemoveSFs();
      }
    }
    else {
      if (efficiency) {
	for (int iLayer = 0; iLayer < Geometry::MAX_TUBE_LAYER; iLayer++) {
	  tp.setIgnoreNone();
	  tp.setIgnoreLayer(iLayer);
	  tp.optimize(kFALSE);
	  if (tp.getDOF() >= minHits-TrackParam::NPARS) {
	    rr->FillChiSq    (tp);
	    rr->FillResiduals(tp);
	  }
	}
      }
      else {
	if (tp.getDOF() >= minHits-TrackParam::NPARS && tp.getDOF() <= maxHits) {
	  if (anySystematic) tp.pullParameter(systIndex, 1.0, floatUp, approxError, biasSyst);
	  else {
	    if (i < 100) {
	      Event local = *(tp.getCurrentEvent());
	      local.AddTrack(Track(tp.slope(), tp.y_int()));
	      ed.DrawEvent(local, geo);
	      ed.DrawTrackZoom(local, geo, 0);
	      ed.DrawTrackZoom(local, geo, 1);
	      ed.Clear();
	    }
	  }
	  rr->FillChiSq    (tp);
	  rr->FillResiduals(tp);
	}
      }
    }
    if (i % event_print == 0) {
      std::cout << "Processing Event " << i << std::endl;
      if (TMath::Floor(TMath::Log10(i)) > TMath::Floor(TMath::Log10(event_print))) event_print*=10;
    }
    tp.RemoveSFs();
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

  rr->Draw();
  if (!isMC) rr->DrawADCPlots(IOUtility::getT0FilePath(runN));

  if (hitResiduals) {
    TFile out(IOUtility::getHitResidualPath(runN, systName), "RECREATE");
    rr->Write();
    out.Write();
    out.Close();
  }
  else {
    TFile out(IOUtility::getFitResidualPath(runN, systName), "RECREATE");
    rr->Write();
    out.Write();
    out.Close();
  }
  rr->SaveImages(IOUtility::getResidualOutputDir(runN, hitResiduals, systName), doTimeSlew);
  TFile f_tree(IOUtility::getTrackTreePath(runN, hitResiduals, systName), "RECREATE");
  rr->WriteTree();
  f_tree.Write();
  f_tree.Close();
  return 0;
}
