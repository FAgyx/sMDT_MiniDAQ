#include "MuonReco/TrackParam.h"

namespace MuonReco {

  TrackParam::TrackParam() : Optimizer(), Parameterization(3) {
    param[THETA]      = 0;
    param[INTERCEPT]  = 1;
    param[DELTAT0]    = 0;
  }

  TrackParam::~TrackParam() {
  }
  
  void TrackParam::SetRT(Callable* rtp) {
    rtfunction = rtp;
  }

  double TrackParam::slope() {
    return -1.0*TMath::Tan(TMath::Pi()/2 + param[THETA]);
  }

  double TrackParam::y_int() {
    return slope()*param[INTERCEPT];
  }

  double TrackParam::deltaT0() {
    return param[TrackParam::DELTAT0];
  }

  double TrackParam::getVerticalAngle() {
    return param[THETA];
  }

  double TrackParam::getImpactParameter() {
    return param[INTERCEPT];
  }

  void TrackParam::RemoveSFs() {
    if (this->size() > NPARS)
      param.erase(param.begin()+NPARS, param.end());
  }

  bool TrackParam::IsRight(Hit h) {
    double deltaX = h.Y()*TMath::Tan(param[THETA]);
    double sign   = (param[THETA]>0) ? -1.0 : 1.0;
    return (param[INTERCEPT]+sign*deltaX)>h.X();
  }

  double TrackParam::D(int index, Hit h) {
    double hitX = h.X();
    double hitY = h.Y();
    double c = TMath::Cos(param[THETA]);
    double s = TMath::Sin(param[THETA]);
    double sign = (c*(hitX-param[INTERCEPT]) + hitY*s > 0) ? 1.0 : -1.0;
    //sign *= (param[THETA]>0) ? -1.0 : 1.0;

    if (index == THETA) {
      return sign*(-s*(hitX-param[INTERCEPT]) + hitY*c);
    }
    else if (index == INTERCEPT) {
      return -sign*c;
    }
    else if (index == DELTAT0) {
      /*
      if (param[THETA]>0)
	return (rtfunction->Eval(h, param[DELTAT0]+1)-rtfunction->Eval(h, param[DELTAT0]));
	else */
      return (rtfunction->Eval(h, param[DELTAT0])-rtfunction->Eval(h, param[DELTAT0]+1));
    }
    else if (index == SLEWFACTOR) {
      return 0;
    }
    else std::cout << "Attempting to access invalid index" << std::endl;
    return 0;
  }


  double TrackParam::Residual(Hit h) {
    double hitX = h.X();
    double hitY = h.Y();

    if (this->size() <= SLEWFACTOR)
      return (Distance(h) - rtfunction->Eval(h, param[DELTAT0]));
    else if (this->size() <= SIGPROPFACTOR) {
      return (Distance(h) - rtfunction->Eval(h, param[DELTAT0], param[SLEWFACTOR]));    
    }
    else {
      return (Distance(h) - rtfunction->Eval(h, param[DELTAT0], param[SLEWFACTOR], 
					     param[SIGPROPFACTOR]));
    }
  }


  double TrackParam::Distance(Hit h) {
    double hitX = h.X();
    double hitY= h.Y();
    double c = TMath::Cos(param[THETA]);
    double s = TMath::Sin(param[THETA]);
    return TMath::Abs(c*(hitX-param[INTERCEPT]) + hitY*s);
  }


  void TrackParam::Initialize(Event* e) {
    // create lists of doubles to hold x, y, radius
    std::vector<double> x, y, r;
    double hitX, hitY;
    int npts = 0;
    for (Cluster c : e->Clusters()) {
      for (Hit h : c.Hits()) {
	hitX = h.X();
	hitY = h.Y();

	x.push_back(hitX);
	y.push_back(hitY);
	r.push_back(rtfunction->Eval(h));
	npts++;
      }      
    }

    // keep track of n pts
    // iterate over 2^n tries, doing a fit each time
    double bestChiSq = DBL_MAX;
    double chiSq;
    for (int bitmap = 0; bitmap < 1<<npts; bitmap++) {
      // declare containers
      std::vector<double> xtrial = std::vector<double>(x.size());
      for (int i = 0; i < npts; i++) {
	if (bitmap & 1<<i) {
	  // here the ith hit should be R
	  xtrial[i] = x[i] + r[i];
	}
	else {
	  // here the th hit should be L
	  xtrial[i] = x[i] - r[i];
	}
      }
      // do fitting
      double fitSlope, fitInt;
      chiSq = LeastSquares(xtrial, y, r, &fitSlope, &fitInt);
      if (chiSq < bestChiSq) {
	bestChiSq        = chiSq;
	param[THETA]     = -1.0*TMath::ATan(1.0/fitSlope);
	param[INTERCEPT] = -1.0*fitInt/fitSlope;
      }
    }
    param[DELTAT0] = 0;
    initialAngle = getVerticalAngle();    
    Print();
  }

  double TrackParam::LeastSquares(std::vector<double> x, std::vector<double> y, std::vector<double> r, double* slopeOut, double* intOut) {
    double xmean = 0.0;
    double ymean = 0.0;
    for (int i = 0; i < x.size(); i++) {
      xmean += x.at(i);
      ymean += y.at(i);
    }
    xmean /= x.size();
    ymean /= y.size();
    
    double cov = 0.0;
    double var = 0.0;
    for (int i = 0; i < x.size(); i++) {
      cov += (x[i]-xmean)*(y[i]-ymean);
      var += (x[i]-xmean)*(x[i]-xmean);
    }

    *slopeOut = cov/var;
    *intOut   = ymean - (*slopeOut)*xmean;
    
    double chiSq = 0.0;
    double dist;
    for (int i = 0; i < x.size(); i++) {
      dist   = TMath::Abs(x[i]*(*slopeOut) + (*intOut) - y[i])/TMath::Sqrt((*slopeOut)*(*slopeOut) + 1);
      chiSq += TMath::Power((dist)/(Hit::RadiusError(r[i])), 2);
    }
    return chiSq;
  }

  double TrackParam::LegendreUpperCurve(double theta, double x_0, double y_0, double r_0) {
    return x_0 * cos(theta) + y_0 * sin(theta) + r_0;
  }

  double TrackParam::LegendreLowerCurve(double theta, double x_0, double y_0, double r_0) {
    return x_0 * cos(theta) + y_0 * sin(theta) - r_0;
  }

  void TrackParam::Print() {
    std::cout << "Track Parameterization Object"   << std::endl;
    std::cout << "Theta:       " << param[THETA]     << std::endl;
    std::cout << "X Intercept: " << param[INTERCEPT] << std::endl;
    std::cout << "Delta T0:    " << param[DELTAT0]   << std::endl;
    std::cout << std::endl;
  }
    
}
