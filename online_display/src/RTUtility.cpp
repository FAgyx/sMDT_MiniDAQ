#include "src/Geometry.cpp"

#include <Math/ChebyshevApprox.h>
#include <Math/IFunction.h>

#ifndef MUON_RT_UTILITY
#define MUON_RT_UTILITY

namespace Muon {

  class RTUtility {

  public:
    static void Initialize(Geometry geo);
    static void Update();
    static void Call(unsigned int tdc, unsigned int ch);
  private:
    static ChebyshevApprox*[Geometry::MAX_TDC*Geometry::MAX_TDC_Channel];

  };


}

#endif
