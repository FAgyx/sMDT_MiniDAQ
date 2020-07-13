#ifndef MUON_RT_PARAMETERIZATION
#define MUON_RT_PARAMETERIZATION

#include <bitset>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "TROOT.h"
#include "TString.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1D.h"
#include "TVectorD.h"
#include "TCanvas.h"

#include "MuonReco/Parameterization.h"
#include "MuonReco/Optimizer.h"
#include "MuonReco/Hit.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Track.h"
#include "MuonReco/Event.h"
#include "MuonReco/T0Fit.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/Callable.h"

namespace MuonReco {
  /*****************************************  
   * Parameterization of RT function       *
   * as a 10 degree Chebyshev              *
   * polynomial.                           * 
   *                                       *
   * Can be optimized by a Muon::Optimizer *
   * object                                *
   *                                       *
   * Author:        Kevin Nelson           *
   * Date:          June 4, 2019           *
   * Last Modified: June 4, 2019           * 
   *                                       *
   *****************************************
   */
  class RTParam : public Optimizer, virtual public Parameterization, public Callable {
  public:
    RTParam(Geometry g);
    ~RTParam();

    void   Initialize    ();
    void   Initialize    (TString t0path, TString decodedDataPath);

    double Eval          (double time);
    double Eval          (Hit h)                  override;
    double NormalizedTime(double time, int tdc_id, int ch_id) override;
    double D             (int index, Hit h)       override;
    double Residual      (Hit h)                  override;
    void   Print         ()                       override;
    double Distance      (Hit h)                  override;
    void   operator +=   (Parameterization delta) override;
    void   operator +=   (TF1* delta);

    TF1*   RTDifference  (RTParam* other);
    TF1*   GetFunction   ();

    void   Draw          ();

    void   Write         (TString tag = "");
    void   Load          (TFile* infile, TString tag = "");
    void   LoadTxt       (TString fname);
    void   SaveImage     (TString fname);

    void   WriteOutputTxt(TString outDir);

    void   SetIgnoreTDC  (int tdc);
    void   SetIgnoreTube (int tdc, int chan);
    void   SetIgnoreAll  ();
    void   ClearIgnore   ();
    void   SetActiveTube (int tdc, int chan);
    void   GetFirstActive(int *tdcOut, int *chanOut);

    void   PrintActive   ();

    static const int npar = 9;
  private:
    TF1*       func;
    TF1*       der;
    TH1D*      cumul;
    Geometry*  geo;

    double t0[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];
    double tF[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];

    std::bitset<Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL> ignoreTube;

    static constexpr double maxTime = 200;

    void   SyncParamToTF1();
    void   SyncTF1ToParam();
  };
}

#endif
