#ifndef MUON_OBSERVABLE
#define MUON_OBSERVABLE

#include <iostream>

#include "TMath.h"
#include "TString.h"
#include "TFile.h"
#include "TVectorD.h"
#include "TSystem.h"

namespace MuonReco {
  /*! \class Observable Observable.h "MuonReco/Observable.h"
   * \brief Class to carry out error propagation for the operations + - * / += -= *= /=
   * 
   * Any observable quantity with an error is described as a double precision value.  Useful for carrying out complex calculations with error propagation.
   * 
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date      7 July 2019
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

    void              Print();
    void              Write(TString path);
    static Observable Load (TString path);
  };

}

#endif
