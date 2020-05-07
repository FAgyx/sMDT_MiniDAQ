#include <sys/stat.h>

#ifndef MUON_IO_UTILITY
#define MUON_IO_UTILITY

namespace Muon {
  /*
   *
   */
  class IOUtility {
  public:
    static const TString RAW_DATA_DIR;
    static const TString SRC_DIR;
    static const TString OUTPUT_DIR;
    static const TString MACRO_DIR;
    static const TString INC_DIR;
    static const TString ROOT_OUT_SUBDIR;
    static const TString AUTOCAL_OUTPUT_ROOT_FILE;
    static const TString FIT_RESIDUAL_OUTPUT_FILE;
    static const TString HIT_RESIDUAL_OUTPUT_FILE;
    static const TString EVENT_TREE_NAME;
    static const TString RESOLUTION_DIR;
    static const TString T0_FILE_NAME;
    static const TString RT_TXT_DIR;
    static const TString AUTOCAL_DIR;
    static const TString MDT_RT_FILE;

    /*
      Do not move: I believe there is an error in the ROOT 
      cling interpreter that causes a seg fault when this 
      function is moved out of class declaration
      -Kevin Nelson, July 5th, 2019
     */
    static TString join(const TString &dir, const TString &sub, Bool_t checkSubDir=kFALSE) 
    {
      struct stat info;
      TString path = dir + "/" + sub;
      if (stat(dir, &info) != 0 || !S_ISDIR(info.st_mode)) {
	mkdir(dir, mode);
      }
      if (checkSubDir) {
	if (stat(path, &info) != 0 || !S_ISDIR(info.st_mode)) {
	  mkdir(path, mode);
	}
      }
      return path;
    }

    static TString getRunOutputDir         (const TString &inputFileName);
    static TString getDecodedOutputFilePath(const TString &inputFileName);
    static TString getRawInputFilePath     (const TString &inputFileName);
    static TString getAutoCalFilePath      (const TString &inputFileName);
    static TString getFitResidualPath      (const TString &inputFileName);
    static TString getHitResidualPath      (const TString &inputFileName);
    static TString getResolutionOutputDir  (const TString &inputFileName);
    static TString getAutoCalOutputDir     (const TString &inputFileName);
    static TString getT0FilePath           (const TString &inputFileName);
    static TString getRTTxtDirPath         (const TString &inputFileName);
    static TString getMDTRTFilePath        ();

    static int     getRunN                 (const TString &inputFileName);

  private:
    static const  int  mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH;

  };



  const TString IOUtility::RAW_DATA_DIR             = "raw";
  const TString IOUtility::SRC_DIR                  = "src";
  const TString IOUtility::OUTPUT_DIR               = "output";
  const TString IOUtility::MACRO_DIR                = "macros";
  const TString IOUtility::INC_DIR                  = "inc";
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

  int IOUtility::getRunN(const TString &inputFileName) {
    /*
     * Assumes the input string is of format:
     run########_YYYYMMDD.dat
     */
    return ((TObjString*)(TString(inputFileName(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  }

}


#endif
