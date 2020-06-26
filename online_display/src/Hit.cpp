#include "src/RecoObject.cpp"

#ifndef MUON_HIT
#define MUON_HIT


namespace Muon {

  class Hit : public RecoObject {
  public:
    Hit();
    Hit(Double_t tdctime, Double_t adc_time, Double_t drift_time, Double_t corr_time, unsigned int _tdc, unsigned int chan);
    ~Hit() {};

    Double_t     TDCTime  ();
    Double_t     Radius   ();
    Double_t     ADCTime  ();
    Double_t     DriftTime();
    Double_t     CorrTime ();
    Double_t     TimeError();
    unsigned int TDC      ();
    unsigned int Channel  ();
    void         SetRadius(Double_t r);
    void         Draw     () override;

    static Double_t RadiusError(double radius);

  private:
    Double_t     time_in_ns;
    Double_t     radius;
    Double_t     adctime;
    Double_t     driftTime;
    Double_t     corrTime;
    unsigned int tdc;
    unsigned int Chan;

  };

  Hit::Hit() {
    time_in_ns = 0.0;
    tdc        = 0;
    Chan       = 0;
    radius     = 0;
  }
  
  Hit::Hit(Double_t tdctime, Double_t adc_time, Double_t drift_time, Double_t corr_time, unsigned int _tdc, unsigned int chan) {
    time_in_ns = tdctime;
    adctime    = adc_time;
    tdc        = _tdc;
    driftTime  = drift_time;
    corrTime   = corr_time;
    Chan       = chan;
    radius     = 0;
  }

  unsigned int Hit::Channel() {
    return Chan;
  }

  unsigned int Hit::TDC() {
    return tdc;
  }

  Double_t Hit::DriftTime() {
    return driftTime;
  }

  Double_t Hit::CorrTime() {
    return corrTime;
  }

  Double_t Hit::TDCTime() {
    return time_in_ns;
  }

  Double_t Hit::ADCTime() {
    return adctime;
  }
  
  Double_t Hit::Radius() {
    return radius;
  }

  Double_t Hit::TimeError() {
    if (corrTime < 90)
      return 0.11;
    else
      return 0.11-corrTime/3.0;
  }

  Double_t Hit::RadiusError(double radius) {

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

  void Hit::SetRadius(Double_t r) {
    radius = r;
  }

  void Hit::Draw() {}
}

#endif
