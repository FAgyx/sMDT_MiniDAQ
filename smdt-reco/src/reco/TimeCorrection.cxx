#include "MuonReco/TimeCorrection.h"

namespace MuonReco {
  
  TimeCorrection::TimeCorrection() {
    // initialize the table of corrections
    adcTable.reserve(400);
    for (int i = 0; i != 400; i++) {
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
