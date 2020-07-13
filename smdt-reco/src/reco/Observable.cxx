#include "MuonReco/Observable.h"

namespace MuonReco {
  Observable::Observable(double value, double error) {
    val = value;
    err = error;
  }

  Observable::~Observable() {

  }
  
  Observable Observable::operator+(Observable other) {
    double newVal, newErr;
    newVal = this->val + other.val;
    newErr = TMath::Sqrt(TMath::Power(this->err, 2) + TMath::Power(other.err, 2));
    return Observable(newVal, newErr);
  }

  Observable Observable::operator-(Observable other) {
    double newVal, newErr;
    newVal = this->val - other.val;
    newErr = TMath::Sqrt(TMath::Power(this->err, 2) + TMath::Power(other.err, 2));
    return Observable(newVal, newErr);
  }

  Observable Observable::operator*(Observable other) {
    double newVal, newErr;
    newVal = this->val * other.val;
    newErr = newVal * TMath::Sqrt(TMath::Power(this->err/this->val, 2) + TMath::Power(other.err/other.val, 2));
    return Observable(newVal, newErr);
  }

  Observable Observable::operator/(Observable other) {
    double newVal, newErr;
    newVal = this->val / other.val;
    newErr = newVal * TMath::Sqrt(TMath::Power(this->err/this->val, 2) + TMath::Power(other.err/other.val, 2));
    return Observable(newVal, newErr);
  }

  void Observable::operator+=(Observable other) {
    double newVal, newErr;
    newVal = this->val + other.val;
    newErr = TMath::Sqrt(TMath::Power(this->err, 2) + TMath::Power(other.err, 2));
    this->val = newVal;
    this->err = newErr;
  }

  void Observable::operator-=(Observable other) {
    double newVal, newErr;
    newVal = this->val - other.val;
    newErr = TMath::Sqrt(TMath::Power(this->err, 2) + TMath::Power(other.err, 2));
    this->val = newVal;
    this->err = newErr;
  }

  void Observable::operator*=(Observable other) {
    double newVal, newErr;
    newVal = this->val * other.val;
    newErr = newVal * TMath::Sqrt(TMath::Power(this->err/this->val, 2) + TMath::Power(other.err/other.val, 2));
    this->val = newVal;
    this->err = newErr;
  }

  void Observable::operator/=(Observable other) {
    double newVal, newErr;
    newVal = this->val / other.val;
    newErr = newVal * TMath::Sqrt(TMath::Power(this->err/this->val, 2) + TMath::Power(other.err/other.val, 2));
    this->val = newVal;
    this->err = newErr;
  }

  Observable Observable::power(double power) {
    double newVal, newErr;
    newVal = TMath::Power(this->val, power);
    newErr = TMath::Abs(newVal * power/this->val * this->err);
    return Observable(newVal, newErr);
  }
}

