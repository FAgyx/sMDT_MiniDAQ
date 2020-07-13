#include "MuonReco/Parameterization.h"

namespace MuonReco {

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


  void Parameterization::setParams(std::vector<double> pNew) {
    if (pNew.size() != this->size()) {
      std::cout << "Attempting to setParam using Parameterizations of"
           << "different lengths.  Aborting." << std::endl;
    }
    param = pNew;
  }

  void Parameterization::setParam(int index, double val) {
    if (index >= this->size()) {
      std::cout << "Index out of bounds" << std::endl;
    }
    param[index] = val;
  }

  double Parameterization::operator [](int index) {
    return param.at(index);
  }

  void Parameterization::operator +=(Parameterization delta) {
    if (delta.size() != this->size()) {
      std::cout << "Attempting to update using Parameterizations of"
	   << "different lengths.  Aborting." << std::endl;
      return;
    }
    std::vector<double> temp(this->size());
    for (int i = 0; i != this->size(); i++) {
      temp[i] = param[i] + delta[i];
    }
    this->param = temp;
  }

}
