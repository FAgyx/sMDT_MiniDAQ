#ifndef MUON_RESOLUTION_RESULT
#define MUON_RESOLUTION_RESULT

#include <iostream>
#include <string>

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

#include "MuonReco/Geometry.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/Observable.h"
#include "MuonReco/TrackParam.h"
#include "MuonReco/T0Fit.h"

namespace MuonReco {
  class ResolutionResult {
  public:
    ResolutionResult(Geometry & geometry);
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
    TGraph*   tubeResVsMeanADC;

    TCanvas*  c_res;

    double    resolution;
    double    chiSqCut   = 1000;

    double    nTubesHit = 0;
    double    nTubesTrackThrough = 0;

    void      Draw         ();
    void      Clear        ();
    void      FillResiduals(TrackParam & tp);
    void      FillChiSq    (TrackParam & tp);
    void      DrawADCPlots (TString t0path);

    TF1*      FitResVsTime (int npar);
    TH1D*     chiSqProb;

    void      SaveImages   (TString outdir);

    bool    PassCuts     (Hit h, const Geometry & geo);

    void      Write        ();
    void      Load         (TString fname);

    void   GetWidthAndError(double* wid, double* err, TH1* hist);
    void   GetMean         (double* mean, TH1* hist);


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

    static const int MAXHITS = 12;
    static bool init;
    static TF1* chi[12];

    static void InitChiSquare();
    static void SetChiSquarePars(TH1* nHitsDist);
    static double ChiSquareCalc(double *x, double* par);
    static TF1* ChiSquareDist(TH1* nHitsDist);

    TH1D* tubeLevelRes[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];


    Geometry* geo;

    TF1 *doubGaus;
    TF1* chiSquareDist;


    TPaveText* DoubGausLegend     (TH1* h);
    void       PopulateResVsRadius();
  };

}
#endif
