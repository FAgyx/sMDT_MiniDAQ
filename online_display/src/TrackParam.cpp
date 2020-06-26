#include "src/Hit.cpp"
#include "src/Geometry.cpp"
#include "src/Parameterization.cpp"
#include "src/Optimizer.cpp"
#include "src/Event.cpp"
#include "src/RTParam.cpp"
#include "src/Callable.cpp"

#ifndef MUON_TRACK_PARAMETERIZATION
#define MUON_TRACK_PARAMETERIZATION

namespace Muon {

  /*****************************************
   * Parameterization of track in 2D       *
   * in terms of intercept and slope       *
   *                                       *
   * Can be optimized by a Muon::Optimizer *
   * object                                *
   *                                       *
   * Author:        Kevin Nelson           *
   * Date:          May 31, 2019           *
   * Last Modified: May 31, 2019           *
   *                                       *
   *****************************************
   */

  class TrackParam : public Optimizer, virtual public Parameterization {
  public:
    TrackParam(Geometry g);
    ~TrackParam();

    void     SetRT     (Callable* rtp);
    void     Initialize(Event *e)         override;
    void     Print     ();

    double   D         (int index, Hit h) override;
    double   Residual  (Hit h)            override;
    double   Distance  (Hit h)            override;

    Double_t LegendreLowerCurve(Double_t theta, Double_t x_0, Double_t y_0, Double_t r_0);
    Double_t LegendreUpperCurve(Double_t theta, Double_t x_0, Double_t y_0, Double_t r_0);

    double   LeastSquares(vector<double> x, vector<double> y,  vector<double> r, double* slopeOut, double* intOut);

    double slope();
    double y_int();

    static const int SLOPE     = 0;
    static const int INTERCEPT = 1;
    
  private:
    Geometry* geo;
    Callable* rtfunction;
    friend class ResolutionResult;
  };

  TrackParam::TrackParam(Geometry g) : Optimizer(), Parameterization(2) {
    geo = &g;
    param[SLOPE]     = 1;
    param[INTERCEPT] = 1;
  }

  TrackParam::~TrackParam() {
  }
  
  void TrackParam::SetRT(Callable* rtp) {
    rtfunction = rtp;
  }

  double TrackParam::slope() {
    return param[TrackParam::SLOPE];
  }

  double TrackParam::y_int() {
    return param[TrackParam::INTERCEPT];
  }

  double TrackParam::D(int index, Hit h) {
    double hitX, hitY;
    geo->GetHitXY(h.TDC(), h.Channel(), &hitX, &hitY);

    
    double arg = hitX*param[SLOPE] + param[INTERCEPT] - hitY;
    double mm1 = TMath::Sqrt(param[SLOPE]*param[SLOPE] + 1);
    if (index == SLOPE) {
      return (mm1*mm1*hitX*arg-param[SLOPE]*arg*arg)/(mm1*mm1*mm1*TMath::Abs(arg));
    }
    if (index == INTERCEPT) {
      return arg/(mm1*TMath::Abs(arg));
    }
    else cout << "Attempting to access invalid index" << endl;
    return 0;
  }


  double TrackParam::Residual(Hit h) {
    double hitX, hitY;
    geo->GetHitXY(h.TDC(), h.Channel(), &hitX, &hitY);
    double sign = ((hitY-param[INTERCEPT])/param[SLOPE] > hitX) ? 1.0 : 1.0;
    return sign * (Distance(h) - rtfunction->Eval(h));
  }


  double TrackParam::Distance(Hit h) {
    double hitX, hitY;
    geo->GetHitXY(h.TDC(), h.Channel(), &hitX, &hitY);

    return TMath::Abs(hitX*param[SLOPE] + param[INTERCEPT] - hitY)/TMath::Sqrt(param[SLOPE]*param[SLOPE] + 1);
  }
  /*
  void TrackParam::Initialize(Event *e) {

    
    double maxThetaPoints = 10000;
    
    Bool_t previousBatchStatus = gROOT->IsBatch();
    gROOT->SetBatch(kTRUE);

    Double_t theta, r, x_0, y_0;
    TH2F plot_map = TH2F("plot_map", "plot_map", 800, 0, 4, 180, -900, 900);

    for (auto c : e->Clusters()) {
      for (auto h : c.Hits()) {
	geo->GetHitXY(h.TDC(), h.Channel(), &x_0, &y_0);
	for (Int_t theta_id = 0; theta_id != maxThetaPoints; theta_id++) {
	  theta = TMath::Pi() * theta_id / maxThetaPoints;
	  r = LegendreUpperCurve(theta, x_0, y_0, rtfunction->Eval(h));
	  plot_map.Fill(theta, r);
	  r = LegendreLowerCurve(theta, x_0, y_0, rtfunction->Eval(h));
	  plot_map.Fill(theta, r);
	}
      }
    }
    Int_t max_bin_theta, max_bin_r, max_bin_z;
    plot_map.GetMaximumBin(max_bin_theta, max_bin_r, max_bin_z);
    Double_t line_para_m, line_para_b;
    line_para_m = -1 / tan(max_bin_theta * 4.0 / 800.0);
    line_para_b = (-900 + max_bin_r * 1800 / 180.0) / sin(max_bin_theta * 4.0 / 800.0);
    */
    /*
    theta = 0;
    r = 0;
    x_0 = 0;
    y_0 = 0;
    Double_t min_theta_limit, max_theta_limit, fill_weight;
    Double_t min_r_limit, max_r_limit;
    min_theta_limit = (max_bin_theta - 10) * 4.0 / 800.0;
    if (min_theta_limit < 0) min_theta_limit = 0;
    max_theta_limit = (max_bin_theta + 10) * 4.0 / 800.0;
    if (max_theta_limit > 4) max_theta_limit = 4;
    min_r_limit = -900 + (max_bin_r - 2) * 1800 / 180.0;
    if (min_r_limit < -800) min_r_limit = -800;
    max_r_limit = -900 + (max_bin_r + 2) * 1800 / 180.0;
    if (max_r_limit > 800) max_r_limit = 800;

    TH2F plot_map_accurate = TH2F("plot_map_accurate", "plot_map_accurate", 40, min_theta_limit, max_theta_limit, 50, min_r_limit, max_r_limit);
    for (auto c : e->Clusters()) {
      for (auto h : c.Hits()) {
	geo->GetHitXY(h.TDC(), h.Channel(), &x_0, &y_0);
	for (Int_t theta_id = 0; theta_id != maxThetaPoints; theta_id++) {
          theta = min_theta_limit + theta_id * (max_theta_limit - min_theta_limit) / 10000;
          //fill_error = 250 - 18.75 * drift_distance[signal_id];
	  fill_weight = 1;

          r = LegendreUpperCurve(theta, x_0, y_0,rtfunction->Eval(h));
          plot_map_accurate.Fill(theta, r, fill_weight);
          r = LegendreLowerCurve(theta, x_0, y_0,rtfunction->Eval(h));
          plot_map_accurate.Fill(theta, r, fill_weight);
	}
      }
    }
    max_bin_theta = 0;
    max_bin_r = 0;
    max_bin_z = 0;
    plot_map_accurate.GetMaximumBin(max_bin_theta, max_bin_r, max_bin_z);

    line_para_m = -1 / tan(min_theta_limit + max_bin_theta * (max_theta_limit - min_theta_limit) / 40);
    line_para_b = (min_r_limit + max_bin_r * (max_r_limit - min_r_limit) / 50) / sin(min_theta_limit + max_bin_theta * (max_theta_limit - min_theta_limit) / 40);
    






    //param[SLOPE]     = line_para_m;
    //param[INTERCEPT] = line_para_b;

    //plot_map.Draw("colz");
    
    //gROOT->SetBatch(previousBatchStatus);
    param[SLOPE]     = 1;
    param[INTERCEPT] = 1;
    }*/

  void TrackParam::Initialize(Event* e) {
    // create lists of doubles to hold x, y, radius
    vector<double> x, y, r;
    double hitX, hitY;
    int npts = 0;
    for (Cluster c : e->Clusters()) {
      for (Hit h : c.Hits()) {
	geo->GetHitXY(h.TDC(), h.Channel(), &hitX, &hitY);
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
      vector<double> xtrial = vector<double>(x.size());
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
	param[SLOPE]     = fitSlope;
	param[INTERCEPT] = fitInt;
      }
    }
  }

  double TrackParam::LeastSquares(vector<double> x, vector<double> y, vector<double> r, double* slopeOut, double* intOut) {
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

  Double_t TrackParam::LegendreUpperCurve(Double_t theta, Double_t x_0, Double_t y_0, Double_t r_0) {
    return x_0 * cos(theta) + y_0 * sin(theta) + r_0;
  }

  Double_t TrackParam::LegendreLowerCurve(Double_t theta, Double_t x_0, Double_t y_0, Double_t r_0) {
    return x_0 * cos(theta) + y_0 * sin(theta) - r_0;
  }

  void TrackParam::Print() {
    cout << "Track Parameterization Object"   << endl;
    cout << "Slope:     " << param[SLOPE]     << endl;
    cout << "Intercept: " << param[INTERCEPT] << endl;
    cout << endl;
  }
    
}

#endif
