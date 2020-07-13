#include "MuonReco/IOUtility.h"

namespace MuonReco {

  const TString IOUtility::RAW_DATA_DIR             = "raw";
  const TString IOUtility::SRC_DIR                  = "src";
  const TString IOUtility::OUTPUT_DIR               = "output";
  const TString IOUtility::MACRO_DIR                = "macros";
  const TString IOUtility::INC_DIR                  = "MuonReco";
  const TString IOUtility::ROOT_OUT_SUBDIR          = "ROOT";
  const TString IOUtility::AUTOCAL_OUTPUT_ROOT_FILE = "autoCalibratedRT.root";
  const TString IOUtility::FIT_RESIDUAL_OUTPUT_FILE = "fitResiduals.root";
  const TString IOUtility::HIT_RESIDUAL_OUTPUT_FILE = "hitResiduals.root";
  const TString IOUtility::EVENT_TREE_NAME          = "eTree";
  const TString IOUtility::RESOLUTION_DIR           = "resolution";
  const TString IOUtility::AUTOCAL_DIR              = "autocalibration";
  const TString IOUtility::T0_FILE_NAME             = "T0.root";
  const TString IOUtility::RT_TXT_DIR               = "RT_txt";
  const TString IOUtility::MDT_RT_FILE              = "Rt_BMG_6_1.dat";
  
  TString IOUtility::getRunOutputDir(const TString &inputFileName) {
    return join(OUTPUT_DIR, TString(inputFileName + ".dir"), kTRUE);
  }
  
  TString IOUtility::getDecodedOutputFilePath(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), inputFileName + ".out.root");
  }

  TString IOUtility::getRawInputFilePath(const TString &inputFileName) {
    return join(RAW_DATA_DIR, inputFileName);
  }

  TString IOUtility::getAutoCalFilePath(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), AUTOCAL_OUTPUT_ROOT_FILE);
  }

  TString IOUtility::getFitResidualPath(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), FIT_RESIDUAL_OUTPUT_FILE);
  }

  TString IOUtility::getHitResidualPath(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), HIT_RESIDUAL_OUTPUT_FILE);
  }

  TString IOUtility::getResolutionOutputDir(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), RESOLUTION_DIR, kTRUE);
  }

  TString IOUtility::getAutoCalOutputDir(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), AUTOCAL_DIR, kTRUE);
  }

  TString IOUtility::getT0FilePath(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), T0_FILE_NAME);
  }

  TString IOUtility::getRTTxtDirPath(const TString &inputFileName) {
    return join(getRunOutputDir(inputFileName), RT_TXT_DIR, kTRUE);
  }

  TString IOUtility::getMDTRTFilePath() {
    return join(SRC_DIR, MDT_RT_FILE);
  }

  TString IOUtility::getMCOutputFilePath(int runN) {
    return join(join("output", TString::Format("mc_run%d", runN), kTRUE), "Histos");
  }
  
  TString IOUtility::getMCEventTreeFilePath(int runN) {
    return join(join("output", TString::Format("mc_run%d", runN), kTRUE), "Events.root");
  }

  int IOUtility::getRunN(const TString &inputFileName) {
    /*
     * Assumes the input string is of format:
     run########_YYYYMMDD.dat
     */
    return ((TObjString*)(TString(inputFileName(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  }

}
