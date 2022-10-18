#ifndef MuonReco_RTLinInterpParam
#define MuonReco_RTLinInterpParam

#include <bitset>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "TGraph.h"
#include "TString.h"
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"

#include "MuonReco/Optimizer.h"
#include "MuonReco/Parameterization.h"
#include "MuonReco/Callable.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/Hit.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/T0Fit.h"
#include "MuonReco/T0Reader.h"
#include "MuonReco/TubeMap.h"

namespace MuonReco {
  /*! \class RTLinInterpParam RTLinInterpParam.h "MuonReco/RTLinInterpParam.h"
   * \brief Parameterize \f$r(t)\f$ function as 100 points with linear interpolation
   *
   * Provides a cross check against the Chebyshev parameterization.  At the present
   * moment, it seems this parameterization requires more data to converge in auto
   * calibration.
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
  class RTLinInterpParam : public Optimizer, virtual public Parameterization {
  public:
    RTLinInterpParam();
    RTLinInterpParam(ConfigParser cp);
    ~RTLinInterpParam();

    void Initialize();
    void Initialize(TString t0path, TString decodedDataPath);
    
    double Eval (double time);
    double Eval (Hit h, double deltaT0=0, double slewScaleFactor=1.0, double sigPropSF=1.0) override;
    double NormalizedTime(double time, int layer, int column) override;
    double D(int index, Hit h) override;
    double Residual (Hit h) override;
    void Print() override;
    double Distance(Hit h) override;
    void operator += (Parameterization delta) override;
    

    void Write(TString tag = "");
    void Load (TFile* infile, TString tag = "");
    void SaveImage(TString outdir);
    bool IsMC() {return isMC;}
    void SetMC(bool mc) {isMC = mc;}

    static const int npar = 100;

  private:
    TGraph* gr;
    TString _t0path;
    TubeMap<double> t0;
    TubeMap<double> tF;

    std::vector<double> indexMinTime;
    std::vector<double> indexMaxTime;
    
    void SyncTGraphToParam();
    void SyncParamToTGraph();
    
    bool isMC = kFALSE;
  };
}

#endif
