#include "src/RecoObject.cpp"

#ifndef MUON_TRACK
#define MUON_TRACK

namespace Muon {
  class Track : public RecoObject {
  public:
    Track();
    Track(Double_t _slope, Double_t _y_int);

    Double_t Slope   ();
    Double_t YInt    ();
    Double_t Distance(double x, double y);
    void     SetSlope(Double_t _slope);
    void     SetYInt (Double_t _y_int);
    
    void     Draw    () override;

  private:
    Double_t slope, y_int;
  };


  Track::Track() {
    slope = 0; 
    y_int = 0;
  }
  
  Track::Track(Double_t _slope, Double_t _y_int) {
    slope = _slope;
    y_int = _y_int;
  }
  
  Double_t Track::Slope() {
    return slope;
  }
  
  Double_t Track::YInt() {
    return y_int;
  }

  void Track::SetSlope(Double_t _slope) {
    slope = _slope;
  }
  
  void Track::SetYInt(Double_t _y_int) {
    y_int = _y_int;
  }

  Double_t Track::Distance(double x, double y) {
    return TMath::Abs(x*slope + y_int - y)/TMath::Sqrt(slope*slope + 1);
  }

  void Track::Draw() {
    return;
  }
}

#endif
