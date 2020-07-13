#ifndef MUON_HIT
#define MUON_HIT

#include "MuonReco/RecoObject.h"

namespace MuonReco {

  class Hit : public RecoObject {
  public:
    Hit();
    Hit(double tdctime, double adc_time, double drift_time, double corr_time, unsigned int _tdc, unsigned int chan, int layer, int column, double _x, double _y);
    ~Hit() {};

    double       TDCTime  ();
    double       Radius   ();
    double       ADCTime  ();
    double       DriftTime();
    double       CorrTime ();
    double       TimeError();
    unsigned int TDC      ();
    unsigned int Channel  ();
    int          Layer    () {return layer;}
    int          Column   () {return column;}
    void         SetRadius(double r);
    void         Draw     () override;
    double       X        () {return x;}
    double       Y        () {return y;}

    static double RadiusError(double radius);

  private:
    double       time_in_ns;
    double       radius;
    double       adctime;
    double       driftTime;
    double       corrTime;
    unsigned int tdc;
    unsigned int Chan;
    int          layer;
    int          column;
    double       x;
    double       y;
  };
}
#endif
