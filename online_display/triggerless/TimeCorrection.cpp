
#ifndef MUON_TIMECORRECTION
#define MUON_TIMECORRECTION

namespace Muon {
  
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
    vector<double> adcTable;
  };

  TimeCorrection::TimeCorrection() {
    // initialize the table of corrections
    adcTable.reserve(400);
    for (Int_t i = 0; i != 400; i++) {
      adcTable.push_back(109./WidthToCharge(i));
    }
  }

  double TimeCorrection::SlewCorrection(double width) {
    if ( width >= 400.0 || width < 0) return 0;
    else return adcTable.at(static_cast<int>(width));    
  }
  
  double TimeCorrection::WidthToCharge(double width) {
    return std::exp(1.11925e+00 + 2.08708e-02 * (width*25.0/32.0));
  }
  
}

#endif
