/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_CALIBFUNC_H
#define MUONCALIB_CALIBFUNC_H


#include <vector>
#include <string>

namespace MuonCalib {

/** generic interface for a calibration function.

    The class caches the parameters and offers routines to access them.
    All correction functions using parametrisations should inherit from them.

    Two functions have to be implemented by the user:
    typeName(): returning a string with the type of function.
                The default is to use the name of the interface.
		For example:
		The implementation of the slewing correction

		IMdtSlewCorFunc::typeName() { return "IMdtSlewCorFunc"; }

    name():     returning the name of the concrete implementation
                
		MdtSlewCorFunc0::name() { return "MdtSlewCorFunc0"; }

    typeName() and name() are used to uniquely indentify the correction 
    function. This is needed from communication with the database.
*/
  
class CalibFunc {
 public:
  typedef std::vector<double> ParVec;
  inline explicit CalibFunc( const ParVec& vec ) : m_parameters( vec ) {}
  inline virtual  ~CalibFunc() {};
  inline unsigned int   nPar() const { return m_parameters.size(); }
  inline const ParVec&  parameters() const { return m_parameters; }
  inline double par(unsigned int index) const { 
    if(index < nPar()) return m_parameters[index];
    else               return 0.;
  }
  virtual std::string typeName() const = 0;
  virtual std::string name() const = 0;

 private:
  ParVec   m_parameters;
};

}  //namespace MuonCalib

#endif

