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
#include "TGraphErrors.h"
#include "TNamed.h"

#include "MuonReco/Parameterization.h"
#include "MuonReco/Optimizer.h"
#include "MuonReco/Hit.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Track.h"
#include "MuonReco/Event.h"
#include "MuonReco/T0Fit.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/Callable.h"
#include "MuonReco/T0Reader.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/TubeMap.h"

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
  class RTParam : public Optimizer, virtual public Parameterization, public TNamed {
  public:
    RTParam();
    RTParam(ConfigParser cp);
    ~RTParam();

    void   Initialize    ();
    void   Initialize    (TString t0path, TString decodedDataPath);

    double Eval          (double time);
    double Eval          (Hit h, double deltaT0=0, double slewScaleFactor=1.0, double sigPropSF=1.0) override;
    double NormalizedTime(double time, int layer, int column) override;
    double D             (int index, Hit h)       override;
    double Residual      (Hit h)                  override;
    void   Print         ()                       override;
    double Distance      (Hit h)                  override;
    void   constrain     (TMatrixD* delta)        override;
    void   operator +=   (Parameterization delta) override;
    void   operator +=   (TF1* delta);

    TF1*   RTDifference  (RTParam* other);
    TF1*   GetFunction   ();

    void   Draw          (TString title=";Drift Time [ns];r(t) [mm]", Bool_t setMinMax = kTRUE);

    void   Write         (TString tag = "");
    void   Load          (TFile* infile, TString tag = "");
    void   LoadTxt       (TString fname);
    void   SaveImage     (TString fname, TString title = ";Drift Time [ns];r(t) [mm]", Bool_t setMinMax = kTRUE);

    void   WriteOutputTxt(TString outDir);

    void   SetIgnoreTDC  (int tdc);
    void   SetIgnoreTube (int layer, int column);
    void   SetIgnoreAll  ();
    void   ClearIgnore   ();
    void   SetActiveTube (int layer, int column);
    void   HardCodeT0TF(double tmin, double tmax);
    void   GetFirstActive(int *layerOut, int *columnOut);
    void   SetMC         (bool mc) {isMC = mc;}
    void   SetUseCorrection(bool b) {useCorrection = b;}
    bool   IsMC          () {return isMC;}

    void   PrintActive   ();

    static const int npar = 9;
    
    bool constrainZero = 0;
    bool constrainEndpoint = 0;

  private:
    TF1*       func;
    TF1*       der;
    TH1D*      cumul;

    TubeMap<double> t0;
    TubeMap<double> tF;
    TubeMap<bool>   ignoreTube;
    
    static constexpr double maxTime = 200;

    TString _t0path;
    bool    isMC = kFALSE;
    bool    useCorrection = kTRUE;
    bool    useFullCheby = kFALSE;

    void   SyncParamToTF1();
    void   SyncTF1ToParam();

    friend class RTAggregator;
  };
}

#endif
