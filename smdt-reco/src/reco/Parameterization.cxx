#include "MuonReco/Parameterization.h"

namespace MuonReco {

  /*! \brief Constructor
   * 
   * If length is provided, reserve space in the vector
   */
  Parameterization::Parameterization(int length) {
    param.reserve(length);
    for (int i = 0; i < length; i++) {
      param.push_back(0);
    }
  }

  /*! \brief Copy constructor
   */
  Parameterization::Parameterization(const Parameterization &p) {
    param = p.param;
  }
  

  /*! Return number of parameters
   */
  int Parameterization::size() {
    return param.size();
  }


  /*! \brief Set parameters using a new vector
   *  Will abort if number of parameters is a mismatch
   */ 
  void Parameterization::setParams(std::vector<double> pNew) {
    if (pNew.size() != this->size()) {
      std::cout << "Attempting to setParam using Parameterizations of"
           << "different lengths.  Aborting." << std::endl;
    }
    param = pNew;
  }

  /*! \brief Set the value of a particular parameter
   *  Will abort if index is out of bounds
   */
  void Parameterization::setParam(int index, double val) {
    if (index >= this->size()) {
      std::cout << "Index out of bounds" << std::endl;
    }
    param[index] = val;
  }

  /*! Element access
   */
  double Parameterization::operator [](int index) {
    return param.at(index);
  }

  /*! \brief Vector addition in this space
   *  Will abort number of parameters is a mismatch
   */
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
