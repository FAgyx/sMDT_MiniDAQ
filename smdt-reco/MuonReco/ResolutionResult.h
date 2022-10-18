#ifndef MUON_RESOLUTION_RESULT
#define MUON_RESOLUTION_RESULT

#include <iostream>
#include <string>
#include <math.h>

#include "TROOT.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TString.h"
#include "TPaveText.h"
#include "TVectorD.h"
#include "TMath.h"
#include "TH2D.h"
#include "TVirtualFFT.h"
#include "TRandom.h"
#include "TLegend.h"
#include "TGaxis.h"
#include "TPaletteAxis.h"

#include "MuonReco/Geometry.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/Observable.h"
#include "MuonReco/TrackParam.h"
#include "MuonReco/T0Fit.h"
#include "MuonReco/T0Reader.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/TimeCorrection.h"
#include "MuonReco/EventDisplay.h"

namespace MuonReco {
  /*! \class ResolutionResult ResolutionResult.h "MuonReco/ResolutionResult.h"
   * \brief Store many histograms relevant to resolution calculations
   * 
   * In addition to storing histograms for \f$\chi^2\f$, residual distributions, 
   * and \f$\Delta t_0\f$ for tracking, the class provides an interface to read
   * and write these histograms to a .root file for easy persistence
   *
   * The ResolutionResult class also allows the user to deconvolute the residuals 
   * from one ResolutionResult class with another.  This is useful for accounting
   * for mulitple scattering effects, where one can deconvolute a histogram containing
   * MC truth information for multiple scattering with an observed residual distribution
   * from data.
   * 
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
  class ResolutionResult {
  public:
    ResolutionResult(ConfigParser cp);
    ~ResolutionResult();

    TH1D*     residuals;
    TH1D*     chiSq;
    TH1D*     radius;
    TH1D*     hitsPerEvent;
    TH2D*     resVsHitRadius;
    TH2D*     resVsHitTime;
    TProfile* meanResVsHitTime;
    TH1D*     resolutionRadius;
    TH1D*     tubeResolutions;
    TH1D*     tubeMeans;
    TGraph*   tubeResVsMeanADC;
    TH1D*     deltaT0;
    TH1D*     t0Systematic;
    TH2D*     systVsAngle;
    TH1D*     xDisplacement;
    TH2D*     residualVsADC;
    TTree*    trackFitTree;
    TString deconvolutionSubDir = "Nominal";
    TString systName = "Nominal";

    TCanvas*  c_res;

    double    resolution;
    double    chiSqCut   = 1000;

    double    nTubesHit = 0;
    double    nTubesTrackThrough = 0;

    void      Draw         ();
    void      Clear        ();
    void      FillResiduals(TrackParam & tp);
    void      FillResidualByValue(double residual, double radius);
    void      FillChiSq    (TrackParam & tp);
    void      DrawADCPlots (TString t0path);

    TF1*      FitResVsTime (int npar);
    TH1D*     chiSqProb;

    void      SaveImages   (TString outdir, Bool_t writeSlewResults=kFALSE);

    bool    PassCuts     (Hit h, const Geometry & geo);

    void      Write        ();
    void      Load         (TString fname);
    void      WriteTree    ();

    static void   GetWidthAndError(double* wid, double* err, TH1* hist);
    void   GetMean         (double* mean, TH1* hist);

    TH1* Deconvolute(ResolutionResult &other, Bool_t hitResiduals=kFALSE, Bool_t reconv=kFALSE);
    static void    calculateResVsRadius(TH1* fitResiduals, TH1* hitResiduals);
    TH1*   throwRandomResidualDist(TF1* resVsRadius, int nResiduals, Bool_t fitResiduals=kTRUE);
    TH1* GetUncorrectedResolutionCurve(ResolutionResult & other, TString dir = ".");

    void SetDoMCMCS(Bool_t b) {doMCMCS = b;}

    double angular_cut = 90;

    void SetIsMC(Bool_t b) {isMC = b;}
  private:
    static TString   ROOT_OUTPUT_NAME;
    static TString   RESIDUALS;
    static TString   CHISQ;
    static TString   RADIUS;
    static TString   HITSPEREVENT;
    static TString   RESVSHITRADIUS;
    static TString   RESVSHITTIME;
    static TString   MEANRESVSHITTIME;
    static TString   RESOLUTIONRADIUS;
    static TString   DELTAT0;
    static TString   RESIDUALVSADC;
    static TString   SYSTEMATICT0SHIFT;
    static TString   XDISPLACEMENT;
    static TString   TUBEWIDTHMAP;
    static TString   TUBEMEANMAP;
    static TString   SYSTVSANGLE;
  public:
    static double    MAXSHIFT;
    static double    SYSTSF;
    static TString   SYSTNAME;

  private:
    static const int MAXHITS = 12;
    static bool init;
    static TF1* chi[12];

    static void InitChiSquare();
    static void SetChiSquarePars(TH1* nHitsDist);
    static double ChiSquareCalc(double *x, double* par);
    static TF1* ChiSquareDist(TH1* nHitsDist);    

    TubeMap<TH1D*>  tubeLevelRes = TubeMap<TH1D*>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
    TH2D* tubeDoubGausMean;
    TH2D* tubeDoubGausWidth;

    Bool_t doMCMCS = 0; //do monte carlo multiple coulomb scattering for efficiency measurement

    Geometry* geo;

    TF1 *doubGaus;
    TF1* singGaus;
    TF1* chiSquareDist;
    TString t0path;
    int runN;
    bool isMC = kFALSE;
    std::vector<TH1*> radialBinProjections;

    // tree variables
    double angle_pattern_recog = 0;
    double angle_optimized     = 0;
    int    number_iterations   = 0;
    double impact_par_opt      = 0;
    int    hitN                = 0;
    int    missedHitN          = 0;
    int    trackHitN           = 0;
    std::vector<double> hitX   = std::vector<double>();
    std::vector<double> hitY   = std::vector<double>();
    std::vector<int>    hitL   = std::vector<int>   ();
    std::vector<int>    hitC   = std::vector<int>   ();
    std::vector<double> hitR   = std::vector<double>();
    std::vector<double> hitRes = std::vector<double>();
    std::vector<double> missedHitR = std::vector<double>();
    std::vector<double> missedHitL = std::vector<double>();
    std::vector<double> missedHitC = std::vector<double>();
    std::vector<double> trackHitR  = std::vector<double>();
    std::vector<double> trackHitL  = std::vector<double>();
    std::vector<double> trackHitC  = std::vector<double>();
    
    // private methods
    static TPaveText* DoubGausLegend     (TH1* h);
    static TPaveText* GausLegend         (TH1* h);
    void       PopulateResVsRadius();
    void       Deconvolute        (TH1* g, TH1* h, TString name, Bool_t reconv=kFALSE, Bool_t sameSigma=kFALSE);
    void       fitTH1DoubleGaussian(TH1* hist, Bool_t sameSigma);
    void       resetDoubleGaussianPars(TH1* hist, Bool_t sameSigma);
    void       SetZeroMean(TH1* hist);
    
  };

}
#endif
