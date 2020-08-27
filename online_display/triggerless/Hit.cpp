
#ifndef MUON_HIT
#define MUON_HIT


namespace Muon {

  class Hit : {
  public:
    Hit();
    Hit(int l_edge, int adc_bin, int drift_bin, double corr_time, int tdc_, int chan_);
    ~Hit() {};

    int       LEdge    ();  //lEdge
    int       ADCbin   ();  //lEdge - tEdge
    int       DriftBin ();  //lEdge for trigger, Signal_lEdge - Trigger_lEdge for signal            
    double    CorrTime ();
    double    TimeError();
    double    Radius   ();
    int       TDC      ();
    int       Channel  ();
    void      SetRadius(double r);
    void      Draw     () override;

    static double RadiusError(double radius);

  private:
    int     ledge;    
    int     adcbin;
    int     driftbin;
    double  corrTime;
    double  radius;
    int     tdc;
    int     chan;
  };

  Hit::Hit() {
    ledge      = 0;
    adcbin     = 0;
    driftbin   = 0;
    corrTime   = 0.0;
    tdc        = 0;
    chan       = 0;
    radius     = 0;
  }
  
  Hit::Hit(int l_edge, int adc_bin, int drift_bin, double corr_time, int tdc_, int chan_){
    ledge      = l_edge;
    adcbin     = adc_bin;
    driftbin   = drift_bin;
    corrTime   = corr_time;
    tdc        = tdc_;    
    chan       = chan_;
    radius     = 0;
  }

  int Hit::LEdge() {
    return ledge;
  }

  int Hit::ADCbin() {
    return adcbin;
  }

  int Hit::DriftBin() {
    return driftbin;
  }
  
  double Hit::CorrTime() {
    return corrTime;
  }

  int Hit::TDC() {
    return tdc;
  }

  int Hit::Channel() {
    return chan;
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

#endif
