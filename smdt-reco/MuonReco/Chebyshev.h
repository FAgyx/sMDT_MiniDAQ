#ifndef MUON_CHEBYSHEV
#define MUON_CHEBYSHEV

#include <vector>

namespace MuonReco {
  class Chebyshev {
  public:
    Chebyshev(int n, double xmin, double xmax) :
      fA(xmin), fB(xmin),
      fT(std::vector<double>(n)) {}

    double operator() (const double * xx, const double *p);

  private:
    double fA;
    double fB;
    std::vector<double> fT; // polynomial
    std::vector<double> fC; // coefficients
  };
}

#endif
