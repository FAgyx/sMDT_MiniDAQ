#ifndef MUON_TIMECORRECTION
#define MUON_TIMECORRECTION

#include <vector>
#include <cmath>

#include "TFile.h"
#include "TVectorD.h"

#include "MuonReco/ConfigParser.h"
#include "MuonReco/IOUtility.h"

namespace MuonReco {

  /*! \class TimeCorrection TimeCorrection.h "MuonReco/TimeCorrection.h"
   * 
   * \brief A class to encapsulate the slew correction
   * 
   * To speed up processing, this class initializes a lookup table of corrections
   * 
   * Originally developed by Felix Rauscher,
   * adapted to sMDTs by Kevin Nelson.
   *
   * For further documentation on ADC chip,
   * see ATL-MUON-2002-003 on CDS.
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
  class TimeCorrection {
  public:
    TimeCorrection();
    TimeCorrection(int runNumber);
    TimeCorrection(ConfigParser cp);

    double SlewCorrection(double width);
    void   Read ();
    void   Write();
    void   ResetTable();

    void   SetScale(double _scale) {scale = _scale;}
    void   SetCoefficient(double _coefficient) {coefficient = _coefficient;}
    
  private:
    double WidthToCharge(double width);
    std::vector<double> adcTable;
    int runN;
    int isASD2 = 0;
    double scale = 0;
    double coefficient = 0;
  };
}
#endif
