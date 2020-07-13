#ifndef MUON_TRACK
#define MUON_TRACK

#include "TMath.h"

#include "MuonReco/RecoObject.h"

namespace MuonReco {
  class Track : public RecoObject {
  public:
    Track();
    Track(double _slope, double _y_int);

    double Slope   ();
    double YInt    ();
    double Distance(double x, double y);
    void     SetSlope(double _slope);
    void     SetYInt (double _y_int);

    void     Draw    () override;

  private:
    double slope, y_int;
  };
}
#endif
