#include "MuonReco/IOUtility.h"

namespace MuonReco {

  const TString IOUtility::RAW_DATA_DIR             = "raw";
  const TString IOUtility::SRC_DIR                  = "src";
  const TString IOUtility::OUTPUT_DIR               = "output";
  const TString IOUtility::MACRO_DIR                = "macros";
  const TString IOUtility::INC_DIR                  = "MuonReco";
  const TString IOUtility::ROOT_OUT_SUBDIR          = "ROOT";
  const TString IOUtility::AUTOCAL_OUTPUT_ROOT_FILE = "autoCalibratedRT_%i_%i.root";
  const TString IOUtility::FIT_RESIDUAL_OUTPUT_FILE = "fitResiduals.root";
  const TString IOUtility::HIT_RESIDUAL_OUTPUT_FILE = "hitResiduals.root";
  const TString IOUtility::EVENT_TREE_NAME          = "eTree";
  const TString IOUtility::FIT_RES_DIR              = "fitResiduals";
  const TString IOUtility::HIT_RES_DIR              = "hitResiduals";
  const TString IOUtility::RESOLUTION_DIR           = "resolution";
  const TString IOUtility::AUTOCAL_DIR              = "autocalibration";
  const TString IOUtility::T0_FILE_NAME             = "T0.root";
  const TString IOUtility::TIME_SLEW_FILE           = "TimeSlew.root";
  const TString IOUtility::RT_TXT_DIR               = "RT_txt";
  const TString IOUtility::MDT_RT_FILE              = "Rt_BMG_6_1.dat";
  const TString IOUtility::TRACK_TREE_FILE          = "trackDiagnostics.root";
  const TString IOUtility::EFFICIENCY_DIR           = "efficiency";
  const TString IOUtility::TOMO_DIR                 = "tomography";
  
  TString IOUtility::getRunOutputDir(int runN) {
    return join(OUTPUT_DIR, TString::Format("run%d", runN), kTRUE);
  }
  
  TString IOUtility::getDecodedOutputFilePath(int runN) {
    return join(getRunOutputDir(runN), "Events.root");
  }

  TString IOUtility::getTimeSlewFile(int runN) {
    return join(getRunOutputDir(runN), TIME_SLEW_FILE);
  }

  TString IOUtility::getRawInputFilePath(const TString& inputFileName) {
    return join(RAW_DATA_DIR, inputFileName);
  }

  TString IOUtility::getAutoCalFilePath(int runN, int minEvent, int nEvents) {
    return join(getAutoCalOutputDir(runN), TString::Format(AUTOCAL_OUTPUT_ROOT_FILE, minEvent, minEvent+nEvents));
  }

  TString IOUtility::getFitResidualPath(int runN, TString systematic/*="Nominal"*/) {
    return join(getResidualOutputDir(runN, kFALSE, systematic), FIT_RESIDUAL_OUTPUT_FILE);
  }

  TString IOUtility::getHitResidualPath(int runN, TString systematic/*="Nominal"*/) {
    return join(getResidualOutputDir(runN, kTRUE, systematic), HIT_RESIDUAL_OUTPUT_FILE);
  }

  TString IOUtility::getTrackTreePath(int runN, Bool_t hitResiduals, TString systematic/*="Nominal"*/) {
    return join(getResidualOutputDir(runN, hitResiduals, systematic), TRACK_TREE_FILE);
  }

  TString IOUtility::getResidualOutputDir(int runN, Bool_t hitResiduals, TString systematic/*="Nominal"*/) {
    if (hitResiduals)
      return join(join(getRunOutputDir(runN), HIT_RES_DIR, kTRUE), systematic, kTRUE);
    else
      return join(join(getRunOutputDir(runN), FIT_RES_DIR, kTRUE), systematic, kTRUE);
  }

  TString IOUtility::getEfficiencyOutputDir(int runN) {
    return join(getRunOutputDir(runN), EFFICIENCY_DIR);
  }

  TString IOUtility::getResolutionOutputDir(int runN) {
    return join(getRunOutputDir(runN), RESOLUTION_DIR, kTRUE);
  }

  TString IOUtility::getAutoCalOutputDir(int runN) {
    return join(getRunOutputDir(runN), AUTOCAL_DIR, kTRUE);
  }

  TString IOUtility::getT0FilePath(int runN) {
    return join(getRunOutputDir(runN), T0_FILE_NAME);
  }

  TString IOUtility::getRTTxtDirPath(int runN) {
    return join(getRunOutputDir(runN), RT_TXT_DIR, kTRUE);
  }

  TString IOUtility::getMDTRTFilePath() {
    return join(RAW_DATA_DIR, MDT_RT_FILE);
  }

  TString IOUtility::getMCOutputFilePath(int runN) {
    return join(getRunOutputDir(runN), "Histos");
  }
  
  TString IOUtility::getMCEventTreeFilePath(int runN) {
    return join(join("output", TString::Format("mc_run%d", runN), kTRUE), "Events.root");
  }

  TString IOUtility::getResolutionObservable(int runN, TString systematic/*="Nominal"*/) {
    return join(join(join(getResolutionOutputDir(runN), systematic), "Nominal"), "Observable.root");
  }

  TString IOUtility::getResolutionCurveFile(int runN) {
    return join(getResolutionOutputDir(runN), "fitfunc.root");
  }

  TString IOUtility::getTomographyDir(int runN) {
    return join(getRunOutputDir(runN), TOMO_DIR, kTRUE);
  }

  TString IOUtility::getDBFile(int runN, TString name) {
    return join(getRunOutputDir(runN), name + ".root");
  }

  int IOUtility::getRunN(const TString& inputFileName) {
    /*
     * Assumes the input string is of format:
     run########_YYYYMMDD.dat
     */
    return ((TObjString*)(TString(inputFileName(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  }

}
