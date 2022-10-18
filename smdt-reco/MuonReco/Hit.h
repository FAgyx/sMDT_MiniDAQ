#ifndef MUON_HIT
#define MUON_HIT

#include "TString.h"
#include "TFile.h"
#include "TF1.h"

#include "MuonReco/RecoObject.h"

namespace MuonReco {
  /*! \class Hit Hit.h "MuonReco/Hit.h"
   *
   * \brief Container for timing, position information of a single pulse on an active wire
   * 
   * The Hit interface also includes a static function RadiusError, which is the nominal resolution function of the hits.  This is important for tracking, where hits that are greater than some cut (say 5 sigma) away from a track are ignored.  This is especially helpful for pileup.
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
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
    static double CavernRadiusError(double radius);
    static void   LoadRadiusErrorFunc(TString path);

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

    static float c0;
    static float c1;
    static float c2;
    static float c3;
    static float c4;
  };
}
#endif
