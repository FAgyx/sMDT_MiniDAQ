#include "MuonReco/Chebyshev.h"

namespace MuonReco {
    double Chebyshev::operator() (const double * xx, const double *p) {
      double x = (2.0 * xx[0] - fA - fB)/(fB-fA);
      int order = fT.size();
      if (order == 1) return p[0];
      if (order == 2) return p[0] + x*p[1];
      // build the polynomials
      fT[0] = 1;
      fT[1] = x;
      for (int i = 1; i < order; ++i) {
	fT[i+1] = 2*x*fT[i] - fT[i-1];
      }
      double sum = p[0]*fT[0];
      for (int i = 1; i <= order; ++i) {
	sum += p[i] * fT[i];
      }
      return sum;
    }
}
