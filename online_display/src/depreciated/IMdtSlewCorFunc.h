/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_IMDTSLEWCORFUNC_H
#define MUONCALIB_IMDTSLEWCORFUNC_H

#include "src/depreciated/CalibFunc.h"

namespace MuonCalib {

  /** generic interface for slewing correction functions */

class IMdtSlewCorFunc : public CalibFunc {
 public:
  explicit IMdtSlewCorFunc( const CalibFunc::ParVec& vec ) : CalibFunc(vec) {};
  virtual ~IMdtSlewCorFunc() {}
  virtual std::string typeName() const {
    return "IMdtSlewCorFunc"; 
  }
  /** Return time correction (ns). Input: t = drift-time (ns), adc = ADC count after ADC-calibration correction */
  virtual double correction(double t, double adc) const = 0;
};

}  //namespace MuonCalib

#endif
