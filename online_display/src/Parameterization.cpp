#include "src/Hit.cpp"
#include "src/Event.cpp"

#ifndef MUON_PARAMETERIZATION
#define MUON_PARAMETERIZATION

namespace Muon {

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

    void           setParams  (vector<double> pNew);
    void           setParam   (int index, double val);
    int            size       ();
    double         operator [](int index);
    virtual void           operator +=(Parameterization delta);



    using iterator       = vector<double>::iterator;
    using const_iterator = vector<double>::const_iterator;

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

    vector<double> param;

  };

  Parameterization::Parameterization(int length) {
    param.reserve(length);
    for (int i = 0; i < length; i++) {
      param.push_back(0);
    }
  }

  Parameterization::Parameterization(const Parameterization &p) {
    param = p.param;
  }
  

  int Parameterization::size() {
    return param.size();
  }


  void Parameterization::setParams(vector<double> pNew) {
    if (pNew.size() != this->size()) {
      cout << "Attempting to setParam using Parameterizations of"
           << "different lengths.  Aborting." << endl;
    }
    param = pNew;
  }

  void Parameterization::setParam(int index, double val) {
    if (index >= this->size()) {
      cout << "Index out of bounds" << endl;
    }
    param[index] = val;
  }

  double Parameterization::operator [](int index) {
    return param.at(index);
  }

  void Parameterization::operator +=(Parameterization delta) {
    if (delta.size() != this->size()) {
      cout << "Attempting to update using Parameterizations of"
	   << "different lengths.  Aborting." << endl;
      return;
    }
    vector<double> temp(this->size());
    for (int i = 0; i != this->size(); i++) {
      temp[i] = param[i] + delta[i];
    }
    this->param = temp;
  }

}

#endif
