#ifndef MUON_IO_UTILITY
#define MUON_IO_UTILITY

#include <sys/stat.h>

#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"

namespace MuonReco {
  /*! \class IOUtility IOUtility.h "MuonReco/IOUtility.h"
   *
   * \brief Static class to encapsulate the naming convention for output in the sMDT project
   * 
   * This class provides a method, join, which automatically creates subdirectories if they do not exist.  The static access methods (i.e. getRunOutputDir, etc.) recursively call join so that all subdirectories are recursively created if they do not exist.  Additionally, if the user is unhappy with the naming convention of output, it is simple to change in one place.
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
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
    static const TString FIT_RES_DIR;
    static const TString HIT_RES_DIR;
    static const TString RESOLUTION_DIR;
    static const TString T0_FILE_NAME;
    static const TString RT_TXT_DIR;
    static const TString TIME_SLEW_FILE;
    static const TString AUTOCAL_DIR;
    static const TString MDT_RT_FILE;
    static const TString TRACK_TREE_FILE;
    static const TString EFFICIENCY_DIR;
    static const TString TOMO_DIR;
    
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
    static TString getRunOutputDir         (int runN);
    static TString getDecodedOutputFilePath(int runN);
    static TString getTimeSlewFile         (int runN);
    static TString getRawInputFilePath     (const TString &inputFileName);
    static TString getAutoCalFilePath      (int runN, int minEvent, int nEvents);
    static TString getFitResidualPath      (int runN, TString systematic="Nominal");
    static TString getHitResidualPath      (int runN, TString systematic="Nominal");
    static TString getResidualOutputDir    (int runN, Bool_t hitResiduals, TString systematic="Nominal");
    static TString getTrackTreePath        (int runN, Bool_t hitResiduals, TString systematic="Nominal");
    static TString getAutoCalOutputDir     (int runN);
    static TString getT0FilePath           (int runN);
    static TString getRTTxtDirPath         (int runN);
    static TString getMDTRTFilePath        ();
    static TString getResolutionOutputDir  (int runN);
    static TString getEfficiencyOutputDir  (int runN);
    static TString getMCOutputFilePath     (int runN);
    static TString getMCEventTreeFilePath  (int runN);
    static TString getResolutionObservable (int runN, TString systematic="Nominal");
    static TString getResolutionCurveFile  (int runN);
    static TString getTomographyDir        (int runN);
    static TString getDBFile               (int runN, TString filename);
    
    static int     getRunN                 (const TString &inputFileName);

  private:
    static const  int  mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH;

  };

}
#endif
