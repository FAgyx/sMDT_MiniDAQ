#include "MuonReco/Hit.h"

namespace MuonReco {
  Hit::Hit() {
    time_in_ns = 0.0;
    tdc        = 0;
    Chan       = 0;
    radius     = 0;
  }
  
  Hit::Hit(double tdctime, double adc_time, double drift_time, double corr_time, unsigned int _tdc, unsigned int chan, int _layer, int _column, double _x, double _y) {
    time_in_ns = tdctime;
    adctime    = adc_time;
    tdc        = _tdc;
    driftTime  = drift_time;
    corrTime   = corr_time;
    Chan       = chan;
    radius     = 0;
    layer      = _layer;
    column     = _column;
    x          = _x;
    y          = _y;
  }

  unsigned int Hit::Channel() {
    return Chan;
  }

  unsigned int Hit::TDC() {
    return tdc;
  }

  double Hit::DriftTime() {
    return driftTime;
  }

  double Hit::CorrTime() {
    return corrTime;
  }

  double Hit::TDCTime() {
    return time_in_ns;
  }

  double Hit::ADCTime() {
    return adctime;
  }
  
  double Hit::Radius() {
    return radius;
  }

  double Hit::TimeError() {
    if (corrTime < 90)
      return 0.11;
    else
      return 0.11-corrTime/3.0;
  }

  double Hit::RadiusError(double radius) {

    /*
    if (radius <3.5) 
      return .25-.028*radius;//(.2-.0400*radius+.00475*radius*radius);
    else
      return .15;
    */

    //return .2-.034*radius + .00384*radius*radius;
    //return 1.3*(.172-.0377*radius+.003815*radius*radius);
    // return .217-0.053*radius+0.0053*radius*radius
    //return .237-.068*radius+.007*radius*radius;
    //return .256-.081*radius+0.00856*radius*radius;
    
    // nominal resolution:
    return .226-0.025*radius+0.001248*radius*radius;
  }

  void Hit::SetRadius(double r) {
    radius = r;
  }

  void Hit::Draw() {}
}
