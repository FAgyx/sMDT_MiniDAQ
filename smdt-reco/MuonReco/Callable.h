#ifndef MUON_CALLABLE
#define MUON_CALLABLE

namespace MuonReco {
  class Hit; // forward declaration

  /*! \class Callable Callable.h "MuonReco/Callable.h"
   * \brief Interface for a function of a Hit object.
   * 
   * Defines abstract interface for any function that is evaluated 
   * with respect to a Hit object.  Useful for extending the r(t) function
   * to different parameterizations
   */
  class Callable {
  public:
    virtual double Eval          (Hit h, double deltaT0=0, double slewScaleFactor=1.0, double sigPropSF=1.0) = 0;
    virtual double NormalizedTime(double time, int tdc_id, int ch_id) = 0;
  };
}

#endif
