

#ifndef MUON_CALLABLE
#define MUON_CALLABLE

namespace Muon {
  class Callable {
  public:
    virtual double Eval          (Hit h) = 0;
    virtual double NormalizedTime(double time, int tdc_id, int ch_id) = 0;
  };
}

#endif
