#ifndef MUON_IO_UTILITY
#define MUON_IO_UTILITY

#include <sys/stat.h>

#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"

namespace MuonReco {
  
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
    static TString join(const TString &dir, const TString &sub, bool checkSubDir=kFALSE)
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
    static TString getMCOutputFilePath     (int runN);
    static TString getMCEventTreeFilePath  (int runN);

    static int     getRunN                 (const TString &inputFileName);

  private:
    static const  int  mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH;

  };

}
#endif
