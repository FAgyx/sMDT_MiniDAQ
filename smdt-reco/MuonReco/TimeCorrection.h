#ifndef MUON_TIMECORRECTION
#define MUON_TIMECORRECTION

#include <vector>
#include <cmath>

namespace MuonReco {

  /*******************************************
   *                                         *
   * A class to encapsulate the hardcoded    *
   * slew correction.                        * 
   *                                         *
   * To speed up processing, this class      * 
   * initializes a lookup table of           *
   * corrections and implements a singleton  *
   * pattern to prevent redudant instances.  * 
   *                                         *
   * Originally developed by Felix Rauscher, *
   * adapted to sMDTs by Kevin Nelson.       *
   *                                         *
   * For further documentation on ADC chip,  *
   * see ATL-MUON-2002-003 on CDS.           *
   *                                         *
   *******************************************
   */
  class TimeCorrection {
  public:
    TimeCorrection();
    double SlewCorrection(double width);
  private:
    double WidthToCharge(double width);
    std::vector<double> adcTable;
  };
}
#endif
