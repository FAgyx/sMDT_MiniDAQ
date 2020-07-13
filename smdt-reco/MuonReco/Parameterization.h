#ifndef MUON_PARAMETERIZATION
#define MUON_PARAMETERIZATION

#include <vector>
#include <iostream>

#include "MuonReco/Hit.h"
#include "MuonReco/Event.h"

namespace MuonReco {
  /*******************************************
   * Class to encapsulate all common methods * 
   * to parameterizations in a truncated     *
   * vector space.                           *
   *                                         *
   * Notable virtual method: D, which when   *
   * overriden will compute the derivative   *
   * of the distance to this hit, specific   *
   * to the parameterization                 *
   *                                         *
   * Author:        Kevin Nelson             *
   * Date:          May 31, 2019             *
   * Last Modified: May 31, 2019             *
   *                                         *
   *******************************************
   */
  class Parameterization {
  public:
    Parameterization() {};
    Parameterization(int length);
    Parameterization(const Parameterization &p);

    void           setParams  (std::vector<double> pNew);
    void           setParam   (int index, double val);
    int            size       ();
    double         operator [](int index);
    virtual void           operator +=(Parameterization delta);



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

    std::vector<double> param;

  };

}
#endif
