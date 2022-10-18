/**********************************************************  
T0Fit.h => Contains functions to fit drift tube time and ADC spectra, and class T0fit to do the fitting.
***********************************************************/
#ifndef MUON_T0_FIT
#define MUON_T0_FIT

#include <stdio.h>
#include <TMath.h>
#include <TPad.h>
#include <TH1F.h>   
#include <TF1.h>
#include <TPaveText.h>
#include <TStyle.h>

#include "MuonReco/IOUtility.h"
using namespace MuonReco;

//  Functions used for ROOT fits.  T0, Tmax, and ADC fit functions from Athena.
//double mt_t0_fermi(double *x , double *par);
//double mt_tmax_fermi(double *x, double *par);
//double skewnormal(double *x, double *par);

/* T0Fit class which fits time and ADC histograms, optionally makes PNG file, and saves fit results in array FitData
PNG plots made the default gPad, so open a TCanvas if you don't want the default canvas size for plots.

Usage of T0fit class:

// Create instance of class.  Mostly the class is a namespace containing the fit functions.
// Parameter 1 is text to be preprended to PNG file names.  (default PNG name: <histogram name>.png)
// Parameter 2 is text to be preprended to histogram titles in PNG plots 
t0fit = T0Fit("TestData","Test Data");

// Set gStyle parameters for PNG files
// Put in separate function in case you make other plots which change gStyle parameters
t0fit.T0fitstyle();

// Call fit functions to fit all the time and ADC histograms.
htime = <a TDC or time histogram>
hadc  = <an ADC histogram>

// Fit the histograms and make PNG plots of fits. 
t0fit.TdcFit(htime,0,1);    
t0fit.AdcFit(hadc,1);    

Fit results are written to array FitData, filled by TdcFit and AdcFit:
Filled by TdcFit:
FitData[0] = Events in time spectrum
FitData[1] = T0 from fit  
FitData[2] = T0 fit error
FitData[3] = T0 Slope 
FitData[4] = T0 Slope Error 
FitData[5] = Background 
FitData[6] = Background Error 
FitData[7] = T0 chi^{2} 
FitData[8] = Tmax 
FitData[9] = Tmax Error 
FitData[10] = Tmax Slope 
FitData[11] = Tmax Slope Error 
FitData[12] = Tmax chi^{2} 
filled by AdcFit
FitData[13] = ADC peak  
FitData[14] = ADC peak Error 
FitData[15] = ADC width 
FitData[16] = ADC width Error
FitData[17] = ADC skew 
FitData[18] = ADC skew Error
FitData[19] = ADC chi^{2}

data can be written out in a stats file defined by statsfile.h

Edward Diehl  18-Feb-2019
***********************************************************/
const int NT0FITDATA=20;  //number of data items in FitData array
extern TString fitDataNames[NT0FITDATA];
extern TString fitDataUnits[NT0FITDATA];

class T0Fit {
public:
  char     Filename[255];        //Prefix for PNG file names.
  char     Hisname[255];         //Prefix for histogram titles.
  double FitData[NT0FITDATA];  //Array of results from fits
  TString _dir =".";

  T0Fit( const char *fname="", const char *hname="", const int notitle=0 );
// Fitting methods for histograms, fill data array, and optionally make PNG file
  int TdcFit(TH1F *h, double pardata[NT0FITDATA], const int plot=1);
  int AdcFit(TH1F *h, const int plot=1);

// Methods to draw ML1+ML2 Time or ADC spectra on same plot.
  int TdcMLPlot(TH1F *h[2]);
  int AdcMLPlot(TH1F *h[2]);

//  Set ROOT style parameters for plots - call before calling TdcFit, AdcFit
  void T0fitstyle( const int notitle=0);

// Some helper functions to reset the filename and histogram title prefixes.
  inline void setfilename(const char *fname="") { sprintf(Filename,"%s",fname); }
  inline void sethisname(const char *hname="") { sprintf(Hisname,"%s",hname); }
  inline void clearfitdata() { for( int i=0; i<NT0FITDATA; i++ ) FitData[i]=0.; }

// Check for NaNs, if NaN found return defaultx.  This prevents FitData[] from containing NaNs
  double CheckNaN( const double x, const double defaultx=0. );

  static constexpr Int_t T0_INDX        = 1;
  static constexpr Int_t MAX_DRIFT_INDX = 8;
  static constexpr Int_t ADC_PEAK_INDX  = 13;
  static constexpr Int_t ADC_WIDTH_INDX = 15;


// T0 fit function
//  par[0] = T0
//  par[1] = T0 slope
//  par[2] = background
//  par[3] = amplitude
static double mt_t0_fermi(double *x , double *par) {
  const double &t(x[0]), &t_0(par[0]), &T(par[1]), &back(par[2]),&A(par[3]);
  return (back + A/(1+exp(-(t-t_0)/T)));
 //  return (par[2] + par[3]/(1+exp(-(x[0]-par[0])/par[1])));
}	

// MTT0 Tmax fit function:
//  par[0] = Tmax
//  par[1] = Tmax slope
//  par[2] = background
//  par[3] = A
//  par[4] = B
//  par[5] = T0
static double mt_tmax_fermi(double *x, double *par) {
  double &t(x[0]);
  double &t_max(par[0]), &T(par[1]), &back(par[2]), &a(par[3]), &b(par[4]), &t_0(par[5]);
  return (back + (exp(a+b*(t-t_0)))/(1+exp((t-t_max)/T)));
  //  return (par[2] + (exp(par[4]+par[3]*(x[0]-par[5])))/(1+exp((x[0]-par[0])/par[1])));
}
// Skewnormal distribution from MTT0
static double skewnormal(double *x, double *par) {
//par[0] = skew gauss norm
//par[1] = skew gauss mean (i.e. mu)
//par[2] = skew gauss sigma (i.e sigma)
//par[3] = skew factor  (i.e. alpha)
// Numeric constants - hardcode these to improve speed
//  invsq2pi = 0.3989422804014327    //1/sqrt(2*pi)
//  twoPi    = 6.2831853071795862
  double delta_value  = par[3]/(TMath::Sqrt(1.+par[3]*par[3]));
  double omega_square = (par[2]*par[2])/(1. - delta_value*delta_value*0.63661977236758137);  //0.63.. = 4/twopi
  double omega_value  = TMath::Sqrt(omega_square);
  double xi_value     = par[1] - delta_value*omega_value*0.7978845608028654;   //*0.79..=2*invsq2pi
  double Gauss_part   = (0.3989422804014327/omega_value)*TMath::Exp(-((x[0] - xi_value)*(x[0] - xi_value))/(2.0*omega_square));  //phi(x)
  double Erf_part     = 1. + TMath::Erf(par[3]*(x[0]-xi_value)/omega_value);
  return par[0]*Gauss_part*Erf_part;
}
};
#endif
