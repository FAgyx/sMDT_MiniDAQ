#ifndef MUON_OBSERVABLE
#define MUON_OBSERVABLE

#include "TMath.h"

namespace MuonReco {
  /****************************************
   * Class to carry out error propagation *
   * for the operations + - * /           *
   * and += -= *= /=                      *
   *                                      *
   * Author:   Kevin Nelson               *
   * Date:     7 July 2019                *
   * Modified: 7 July 2019                *
   *                                      *
   ****************************************
   */
  class Observable {
  public:
    Observable(double value, double error);
    ~Observable();

    double val;
    double err;

    Observable operator+(Observable other);
    Observable operator-(Observable other);
    Observable operator*(Observable other);
    Observable operator/(Observable other);

    void operator+=(Observable other);
    void operator-=(Observable other);
    void operator*=(Observable other);
    void operator/=(Observable other);

    Observable power(double power);
  };

}

#endif
