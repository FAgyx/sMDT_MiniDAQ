#ifndef MUON_PARAMETERIZATION
#define MUON_PARAMETERIZATION

#include <vector>
#include <iostream>

#include "MuonReco/Hit.h"
#include "MuonReco/Event.h"
#include "MuonReco/Callable.h"

namespace MuonReco {
  /*! \class Parameterization Parameterization.h "MuonReco/Parameterization.h"
   * \brief Class to encapsulate all common methods to parameterizations in a truncated vector space.
   * 
   * Notable virtual method: D, which when overriden will compute the 
   derivative of the distance to this hit, specific to the parameterization 
   * 
   * Parameterizations can also be iterated over as a std::vector
   * 
   * \author        Kevin Nelson
   *                kevin.nelson@cern.ch 
   * \date          May 31, 2019
   * Last Modified: May 31, 2019
   */
  class Parameterization : public Callable {
  public:
    Parameterization() {};
    Parameterization(int length);
    Parameterization(const Parameterization &p);

    void           setParams  (std::vector<double> pNew);
    void           setParam   (int index, double val);
    int            size       ();
    double         operator [](int index);
    virtual void           operator +=(Parameterization delta);


    virtual double Eval(Hit h, double deltaT0=0, double slewScaleFactor=1.0, double sigPropSF=1.0) override {return 0;}
    virtual double NormalizedTime(double time, int tdc_id, int ch_id) override {return 0;}

    using iterator       = std::vector<double>::iterator;
    using const_iterator = std::vector<double>::const_iterator;

    iterator       begin ()       { return param.begin();  }
    iterator       end   ()       { return param.end();    }
    const_iterator begin () const { return param.begin();  }
    const_iterator end   () const { return param.end();    }
    const_iterator cbegin() const { return param.cbegin(); }
    const_iterator cend  () const { return param.cend();   }



    virtual double D          (int index, Hit h) {return 0;};
    virtual double Residual   (Hit h)            {return 0;};
    virtual double Distance   (Hit h)            {return 0;};
    virtual void   Print      ()                 {};
    virtual void   Initialize (Event* e)         {};

    std::vector<double> param; //< Underlying coefficients

  };

}
#endif
