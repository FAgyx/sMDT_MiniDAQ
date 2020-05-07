/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_MdtSlewCorFuncHardcoded_H
#define MUONCALIB_MdtSlewCorFuncHardcoded_H

#include "src/depreciated/IMdtSlewCorFunc.h"
#include "vector"

namespace MuonCalib {

  /** implementation of a slewing correction function */

  class MdtSlewCorFuncHardcoded : public IMdtSlewCorFunc {
  public:
    //  explicit MdtSlewCorFuncHardcoded( const CalibFunc::ParVec& vec ) : IMdtSlewCorFunc(vec){}
    explicit MdtSlewCorFuncHardcoded( const CalibFunc::ParVec& vec );
    
    inline virtual std::string name() const { return "MdtSlewCorFuncHardCoded"; }
    inline static unsigned int nUsedPar() { return 0; }
    virtual double correction(double t, double adc) const;
    double calibrated_p(const double &adc) const;
    
    //  static std::map<short, float> m_LUT;
  private:
    static std::vector<double> m_LUT;
  };

}  //namespace MuonCalib

#endif
