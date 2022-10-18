#include "MuonReco/TimeCorrection.h"

namespace MuonReco {
  
  TimeCorrection::TimeCorrection() {
    // initialize the table of corrections using default values
    coefficient = 35.59;
    scale = 0.0163;
    ResetTable();
  }

  TimeCorrection::TimeCorrection(ConfigParser cp) : TimeCorrection() {
    runN   = cp.items("General").getInt("RunNumber");
    isASD2 = cp.items("TimeCorrection").getInt("IsASD2");
    if (isASD2) {
      coefficient *= 1.33;
    }
    ResetTable();
  }
  
  TimeCorrection::TimeCorrection(int runNumber) : TimeCorrection() {
    runN = runNumber;
    Read();
  }

  void TimeCorrection::ResetTable() {
    adcTable.clear();
    for (int i = 0; i != 400; i++) {
      adcTable.push_back(coefficient/ (exp( scale*WidthToCharge(i) )) );
    }
  }

  double TimeCorrection::SlewCorrection(double width) {
    if ( width >= 400.0 || width < 0) return 0;
    else return adcTable.at(static_cast<int>(width));    
  }
  
  /**
     Optional function to convert width to charge using Wilkinson response curve
     However, it is more agnostic to data contitions to use raw ADC
   */
  double TimeCorrection::WidthToCharge(double width) {    
    return (isASD2) ? 0.8*width : width;
  }

  void TimeCorrection::Read() {
    TFile* f = TFile::Open(IOUtility::getTimeSlewFile(runN));
    if (f) {
      TVectorD* data;
      data      = (TVectorD*)f->Get("TimeCorrection");
      scale     = data->operator[](0);
      coefficient = data->operator[](1);
      ResetTable();
      std::cout << "Successfully read time slew for run " << runN << std::endl;
      std::cout << "Scale: " << scale << std::endl;
      std::cout << "Coefficient: " << coefficient << std::endl;
    }
    else {
      std::cout << "Time slew does not yet exist for this run. " << std::endl 
		<< "Please create time slew before measuring resolution" << std::endl;
    }
  }

  void TimeCorrection::Write() {
    TFile f(IOUtility::getTimeSlewFile(runN), "RECREATE");
    TVectorD data = TVectorD(2);
    data[0] = scale;
    data[1] = coefficient;
    data.Write("TimeCorrection");
    f.Close();
    std::cout << "Done writing to " << IOUtility::getTimeSlewFile(runN) << std::endl;
  }
  
}
