#include "MuonReco/Hit.h"

namespace MuonReco {

  float Hit::c0 = 0.24002;
  float Hit::c1 = -0.055802;
  float Hit::c2 = 0.0069947;
  float Hit::c3 = -0.000409;
  float Hit::c4 = 0.000009309;

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
    return c0 + c1*radius + c2*radius*radius + c3*radius*radius*radius + c4*radius*radius*radius*radius;
  }

  void Hit::LoadRadiusErrorFunc(TString path) {
    TFile* f = TFile::Open(path);
    TF1* func = (TF1*)(f->Get("fitfunc"));
    c0 = func->GetParameter(0)/1000.0;
    c1 = func->GetParameter(1)/1000.0;
    c2 = func->GetParameter(2)/1000.0;
    c3 = 0;
    c4 = 0;
  }

  double Hit::CavernRadiusError(double radius) {    
    return .24002 - 0.055802*radius + 0.0069947*radius*radius - 0.000409*radius*radius*radius
      +0.000009309*radius*radius*radius*radius;
  }

  void Hit::SetRadius(double r) {
    radius = r;
  }

  void Hit::Draw() {}
}
