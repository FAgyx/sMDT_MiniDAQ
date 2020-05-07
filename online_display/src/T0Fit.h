/**********************************************************  
T0Fit.h => Contains functions to fit drift tube time and ADC spectra, and class T0fit to do the fitting.
***********************************************************/

#include <stdio.h>
#include <TMath.h>
#include <TPad.h>
#include <TH1F.h>   
#include <TF1.h>
#include <TPaveText.h>
#include <TStyle.h>

#ifndef MUON_T0_FIT
#define MUON_T0_FIT

//  Functions used for ROOT fits.  T0, Tmax, and ADC fit functions from Athena.
Double_t mt_t0_fermi(Double_t *x , Double_t *par);
Double_t mt_tmax_fermi(Double_t *x, Double_t *par);
Double_t skewnormal(Double_t *x, Double_t *par);

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
class T0Fit {
public:
  char     Filename[255];        //Prefix for PNG file names.
  char     Hisname[255];         //Prefix for histogram titles.
  Double_t FitData[NT0FITDATA];  //Array of results from fits

  T0Fit( const char *fname="", const char *hname="", const int notitle=0 );
// Fitting methods for histograms, fill data array, and optionally make PNG file
  int TdcFit(TH1F *h, Double_t pardata[NT0FITDATA], const int plot=1);
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
  Double_t CheckNaN( const Double_t x, const Double_t defaultx=0. );

  static constexpr Int_t T0_INDX        = 1;
  static constexpr Int_t MAX_DRIFT_INDX = 8;
  static constexpr Int_t ADC_PEAK_INDX  = 13;
  static constexpr Int_t ADC_WIDTH_INDX = 15;
};

// T0 fit function
//  par[0] = T0
//  par[1] = T0 slope
//  par[2] = background
//  par[3] = amplitude
Double_t mt_t0_fermi(Double_t *x , Double_t *par) {
  const Double_t &t(x[0]), &t_0(par[0]), &T(par[1]), &back(par[2]),&A(par[3]);
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
Double_t mt_tmax_fermi(Double_t *x, Double_t *par) {
  Double_t &t(x[0]);
  Double_t &t_max(par[0]), &T(par[1]), &back(par[2]), &a(par[3]), &b(par[4]), &t_0(par[5]);
  return (back + (exp(a+b*(t-t_0)))/(1+exp((t-t_max)/T)));
  //  return (par[2] + (exp(par[4]+par[3]*(x[0]-par[5])))/(1+exp((x[0]-par[0])/par[1])));
}
// Skewnormal distribution from MTT0
Double_t skewnormal(Double_t *x, Double_t *par) {
//par[0] = skew gauss norm
//par[1] = skew gauss mean (i.e. mu)
//par[2] = skew gauss sigma (i.e sigma)
//par[3] = skew factor  (i.e. alpha)
// Numeric constants - hardcode these to improve speed
//  invsq2pi = 0.3989422804014327    //1/sqrt(2*pi)
//  twoPi    = 6.2831853071795862
  Double_t delta_value  = par[3]/(TMath::Sqrt(1.+par[3]*par[3]));
  Double_t omega_square = (par[2]*par[2])/(1. - delta_value*delta_value*0.63661977236758137);  //0.63.. = 4/twopi
  Double_t omega_value  = TMath::Sqrt(omega_square);
  Double_t xi_value     = par[1] - delta_value*omega_value*0.7978845608028654;   //*0.79..=2*invsq2pi
  Double_t Gauss_part   = (0.3989422804014327/omega_value)*TMath::Exp(-((x[0] - xi_value)*(x[0] - xi_value))/(2.0*omega_square));  //phi(x)
  Double_t Erf_part     = 1. + TMath::Erf(par[3]*(x[0]-xi_value)/omega_value);
  return par[0]*Gauss_part*Erf_part;
}

// Constructor.  Silly really, the T0fit class is really just a namespace for file and histogram names
T0Fit::T0Fit( const char *fname, const char *hname, const int notitle ) {
  sprintf(Filename,"%s",fname);
  sprintf(Hisname,"%s",hname);
  T0fitstyle(notitle);
  clearfitdata();
}

/************************************************************
*  Fit Time spectrum for T0 and Tmax and optionally create PNG
*  h       = Drift Time spectrum histogram to fit (pointer)
*  pardata = array of fit data used to set initial values of parameters (optional - if pointer to array is NULL, defaults will be determined from histogram)
*            If you fit a chamber-level time spectra first, can use parameters from chamber fit for fits of smaller units, e.g. individual tube histograms.
*  plot    = Make PNG of fit histogram.  Default name: <h->GetName()>.png.  Default gPad is used 
************************************************************/
int T0Fit::TdcFit(TH1F *h, Double_t pardata[NT0FITDATA], const int plot ) {
  Double_t t0,slope,ampl,xmin,xmax,maxval=130.;
  Double_t tmax,tmslope,tmback=0.,A=3.,B=-0.0015;
 
  clearfitdata();

  // Protect for null histograms and low statistics time spectra
  if( h == 0 || h->GetEntries()<1000. ) return -1;
  printf("Fitting %s\n",h->GetName());

  gStyle->SetOptStat("e"); 

  // Reset histogram title if desired
  if( Hisname[0] != '\0' ) {
    h->SetTitle(Form("%s %s",Hisname,h->GetTitle()));
  }

  FitData[0] = CheckNaN(h->GetEntries());
  
//  default text box locations  
  TPaveText *t0PT = new TPaveText(0.55,0.65,0.98,0.82,"NDC");
  TPaveText *tmPT = new TPaveText(0.55,0.50,0.98,0.65,"NDC");
  TPaveText *tdPT = new TPaveText(0.55,0.42,0.98,0.50,"NDC");

//  Fit T0.  Used pardata for initial values if available, else estimate from histogram data
  maxval = h->GetMaximum();
  if( pardata ) {
    t0    = pardata[1];
    slope = pardata[3];
  } else {
    t0     = h->GetBinCenter(h->FindFirstBinAbove(0.45*maxval));
    slope  = 2.5;
  }
  //  ampl  = h->GetEntries()/4000.;
  ampl  = maxval/1.1;
  xmin  = t0 - 300.;
  //if( xmin < -200. ) xmin = -200.;
  xmax = h->GetBinCenter(h->GetMaximumBin());

  printf("Estimated parameters t0=%.1f slope=%.1f back=%.1f ampl=%.1f xmin=%.1f xmax=%.1f maxval=%.1f\n",t0,slope,0.,ampl,xmin,xmax,maxval);

    // Define TF1 function for T0 fit
  TF1 *ft0 = new TF1("mt_t0_fermi",mt_t0_fermi,xmin,xmax,4);
  ft0->SetParameters(t0,slope,0.,ampl);
  //changed
  ft0->SetParLimits(2,0.,ampl/1.0);  //Do not allow negative noise
  h->Fit("mt_t0_fermi","R");

  printf("Noise=%.1f\n",ft0->GetParameter(2));
  if( ft0->GetParameter(2) < 0. ) {
    printf("Noise<0; Refit %s\n",h->GetName());
    ft0->FixParameter(2,0.);
    h->Fit("mt_t0_fermi","R");
  }

// Code to refit pathalogical time spectra.
// refit with a smaller range if problems: 
//  slope too small refit 
//  fitted T0 very different from estimated T0
//  if( ft0->GetParameter(1) < 1. || TMath::Abs(ft0->GetParameter(0)-t0) > 5. ) {
//    printf("Refit %s\n",h->GetName());
//    refit++;
//    ft0 = new TF1("mt_t0_fermi",mt_t0_fermi,xmin,xmax-35.,4);
//    ft0->SetParameters(t0,2.,0.,ampl);
//    h->Fit("mt_t0_fermi","r");
//  }
// if still slope fix slope=2
//  if( ft0->GetParameter(1) < 1. ) {
//    printf("Refit AGAIN %s\n",h->GetName());
//    refit++;
//    ft0 = new TF1("mt_t0_fermi",mt_t0_fermi,xmin,xmax-20.,4);
//    ft0->SetParameters(t0,2.,0.,ampl);
//    ft0->SetParLimits(2,2.,2.);     //force slope=2
//    h->Fit("mt_t0_fermi","r");
//  }

  t0PT->AddText(Form("T0 %.2lf #pm %.2lf ns",ft0->GetParameter(0),ft0->GetParError(0)));
  t0PT->AddText(Form("Slope %.2lf #pm %.2lf /ns",ft0->GetParameter(1),ft0->GetParError(1)));
  t0PT->SetTextColor(2);
  t0PT->Draw();
  FitData[1] = CheckNaN(ft0->GetParameter(0));
  FitData[2] = CheckNaN(ft0->GetParError(0));
  FitData[3] = CheckNaN(ft0->GetParameter(1));
  FitData[4] = CheckNaN(ft0->GetParError(1));
  FitData[5] = CheckNaN(ft0->GetParameter(2));
  FitData[6] = CheckNaN(ft0->GetParError(2));
  double dof = ft0->GetNDF();
  if( dof ) FitData[7] = CheckNaN(ft0->GetChisquare()/dof);

  // Only do Tmax fit if there are more than 2000 entries
  if( h->GetEntries() >2000. ) {
  // Do Tmax fit, first estimate input parameters
    t0 = FitData[1];
    if( pardata ) {
      tmax    = pardata[8]+t0;
      tmslope = pardata[10];
      tmback  = FitData[5];    //use background from T0 fit
    } else {
      tmax    = h->GetBinCenter(h->FindLastBinAbove(0.1*maxval));
      tmslope = 8.;
      tmback  = FitData[5];    //use background from T0 fit
    }

    xmin = tmax - 80.;
    xmax = tmax + 200.;
    if( xmax > 900. ) xmax=900.;
    printf("Estimated Tmax parameters tmax=%.1f slope=%.1f back=%.1f A=%.1f B=%.5f xmin=%.1f xmax=%.1f\n",
	   tmax,tmslope,tmback,A,B,xmin,xmax);
    // Define TF1 function for Tmax fit
    TF1 *ftm = new TF1("mt_tmax_fermi",mt_tmax_fermi,xmin,xmax,6);
    ftm->SetParameters(tmax,tmslope,tmback,A,B,t0);
    ftm->SetParLimits(6,t0,t0);     //Use t0 from ft0 fit
    ftm->SetParLimits(3,0.,1000.);  //Tmax limits
    ftm->SetLineColor(8);
    h->Fit("mt_tmax_fermi","r+");   //the "+" is to add the function to the list
    
    ftm->SetLineColor(8);
    tmPT->AddText(Form("Tmax %.1lf #pm %.1lf ns",  ftm->GetParameter(0),ftm->GetParError(0)));
    tmPT->AddText(Form("Slope %.1lf #pm %.1lf /ns",ftm->GetParameter(1),ftm->GetParError(1)));
    tmPT->SetTextColor(8);
    tmPT->Draw();

    // Compute max drift time of time spectra: Tmax-T0
    Double_t dtmax  = ftm->GetParameter(0) - ft0->GetParameter(0);
    Double_t dtmaxe = TMath::Sqrt( ftm->GetParError(0)*ftm->GetParError(0)+
				  ft0->GetParError(0)*ft0->GetParError(0) );
    tdPT->AddText(Form("DTmax %.1lf #pm %.1lf ns",dtmax,dtmaxe));
    tdPT->SetTextColor(1);
    tdPT->Draw();
    FitData[8]  = CheckNaN(dtmax);
    FitData[9]  = CheckNaN(dtmaxe);
    FitData[10] = CheckNaN(ftm->GetParameter(1));
    FitData[11] = CheckNaN(ftm->GetParError(1));
    dof = ftm->GetNDF();
    if( dof ) FitData[12] = CheckNaN(ftm->GetChisquare()/dof);
  }

  t0PT->Draw();  //should not be needed, done above, but does not appear if you don't
  if( plot ) {
    if( Filename[0] != '\0' ) gPad->Print(Form("%s_%s.png",Filename,h->GetName()));
    else                      gPad->Print(Form("%s.png",h->GetName()));
  }
  return 0;
}  //end T0Fit::TdcFit

/************************************************************
*  Fit ADC spectrum
*  h       = Drift Time spectrum histogram to fit (pointer)
*  plot    = Make PNG of fit histogram.  Default name: <h->GetName()>.png.  Default gPad is used 
************************************************************/
int T0Fit::AdcFit(TH1F *h, const int plot ) {

  // Protect for null histograms
  if( h == 0 || h->GetEntries()<1000. ) return -1;
  printf("Fitting %s\n",h->GetName());
  // Reset histogram title if desired
  if( Hisname[0] != '\0' ) {
    h->SetTitle(Form("%s %s",Hisname,h->GetTitle()));
  }

  gStyle->SetOptStat("emr"); 
  TPaveText *adcPT = new TPaveText(0.55,0.315,0.98,0.635,"NDC");

//  Refit ADC
  Double_t n    = h->GetEntries();
  Double_t rms  = h->GetRMS();
  Double_t mean = h->GetMean();
  Double_t ymin = mean-2*rms;
  if( ymin < 60. ) ymin=60.;
  Double_t ymax = mean+1.5*rms;
  //Define TF1 function for skew normal fit of ADC spectrum 
  TF1 *fadc = new TF1("adc_fun",skewnormal,ymin,ymax,4);
  fadc->SetParameters(n,mean,rms,1.5);
  fadc->SetParLimits(1,mean-rms,mean+0.5*rms);
  fadc->SetParLimits(2,rms*0.5,rms*1.5);
  fadc->SetParLimits(3,1.,4.);
  h->Fit("adc_fun","r"); 
//  If skew is too high refit with fixed skew
  if( fadc->GetParameter(3) > 2.5 ) {
    printf("REFIT %s\n",h->GetName());
    fadc->SetParameter(3,1.6);
    fadc->SetParLimits(3,1.6,1.6);
    h->Fit("adc_fun","r") ;
  }
// if have enough hits use fit results  
  if( n > 1000 ) {    
    FitData[13] = CheckNaN(fadc->GetParameter(1));
    FitData[14] = CheckNaN(fadc->GetParError(1));
    FitData[15] = CheckNaN(fadc->GetParameter(2));
    FitData[16] = CheckNaN(fadc->GetParError(2));
    FitData[17] = CheckNaN(fadc->GetParameter(3));
    FitData[18] = CheckNaN(fadc->GetParError(3));
    double dof = fadc->GetNDF();
    if( dof > 0. ) FitData[19] = CheckNaN(fadc->GetChisquare()/dof);
    adcPT->AddText(Form("Peak %.1lf #pm %.1lf",FitData[13],FitData[14]));
    adcPT->AddText(Form("Width %.1lf #pm %.1lf",FitData[15],FitData[16]));
    adcPT->AddText(Form("Skew %.2lf #pm %.2lf",	FitData[17],FitData[18]));
    if( dof > 0. ) adcPT->AddText(Form("#Chi^{2}/DoF     %.2lf",FitData[19]));
    adcPT->SetTextColor(2);
    adcPT->Draw();
// if have too few hits use fit stats instead of fit results  
  } else {
    FitData[12] = CheckNaN(mean);
    FitData[13] = CheckNaN(h->GetMeanError());
    FitData[14] = CheckNaN(rms);
    FitData[15] = CheckNaN(h->GetRMSError());
    FitData[16] = CheckNaN(h->GetSkewness());
  }
  if( plot ) {
    if( Filename[0] != '\0' ) gPad->Print(Form("%s_%s.png",Filename,h->GetName()));
    else                   gPad->Print(Form("%s.png",h->GetName()));
  }
  return 0;
}    //end T0Fit::AdcFit

/************************************************************
*  Draw ML1+ML2 Time spectrum on same plot.
*  h = array with TDC histograms from both ML.
*  Presumes that ML histograms have already been fit with TdcFit
*  i.e. that they contain fit functions whose results are printed on plot.
*  Plots named <Filename>_<chamber>_TimeML.png or <chamber>_TimeML.png
************************************************************/
int T0Fit::TdcMLPlot(TH1F *h[2]) {
  char htitle[80],htitle_save[80]; 

  gStyle->SetOptStat(0);

  // Protect for null histograms
  if( h[0] == NULL ) return -1;

  // Save original histogram title
  sprintf(htitle_save,"%s",h[0]->GetTitle());

  // Assume histogram name starts with chamber name
  sprintf(htitle,"%s",h[0]->GetName());
  htitle[7] = '\0';  //truncate htitle so be just chamber name
  // Reset histogram title if desired
  if( Hisname[0] == '\0' ) {
    h[0]->SetTitle(Form("%s ML",htitle));
  } else {
    h[0]->SetTitle(Form("%s %s ML",Hisname,htitle));
  }

  h[0]->SetStats(0);
  if( h[1] != NULL ) {
    h[1]->SetLineColor(8);
    h[1]->SetStats(0);
    //Reset h[0] scale if needed so h[1] does not get cut off
    if( h[0]->GetMaximum() < h[1]->GetMaximum() ) {
      h[0]->SetMaximum(h[1]->GetMaximum()*1.05);
    }
    h[0]->Draw();
    h[1]->Draw("same");
  } else {
    h[0]->Draw();
  }

//  text box locations  
  TPaveText *keyPT[2];
  keyPT[0] = new TPaveText(0.53,0.75,0.66,0.90,"NDC");
  keyPT[0]->AddText(" ");
  keyPT[0]->AddText("Entry");
  keyPT[0]->Draw();
  keyPT[1] = new TPaveText(0.53,0.45,0.66,0.75,"NDC");
  keyPT[1]->AddText("T0");
  keyPT[1]->AddText("Slope");
  keyPT[1]->AddText("Tmax");
  keyPT[1]->AddText("TMslp");

// Grab parameters from histogram fit functions
  TPaveText *enPT[2],*t0PT[2], *tdPT;
  Double_t t0[2]={0.,0.};
  Double_t tmax[2]={0.,0.};
  for( int ml=0; ml<2; ml++ ) {
    if( h[ml] == NULL || h[ml]->GetEntries() == 0. ) continue;
    if( ml==0 ) {
      keyPT[1]->Draw();
      enPT[0] = new TPaveText(0.66,0.75,0.82,0.90,"NDC");
      t0PT[0] = new TPaveText(0.66,0.45,0.82,0.75,"NDC");
      enPT[0]->SetTextColor(4);
      t0PT[0]->SetTextColor(50);
    } else {
      enPT[1] = new TPaveText(0.82,0.75,0.98,0.90,"NDC");
      t0PT[1] = new TPaveText(0.82,0.45,0.98,0.75,"NDC");
      enPT[1]->SetTextColor(8);
      t0PT[1]->SetTextColor(7);
    }
    enPT[ml]->AddText(Form("ML%i",ml+1));
    enPT[ml]->AddText(Form("%.0f",h[ml]->GetEntries()));
    enPT[ml]->Draw();
    //Get T0 fit
    TF1 *ft0 = h[ml]->GetFunction("mt_t0_fermi");
    if( ft0 == NULL ) continue;
    if( ml==0 ) ft0->SetLineColor(50);
    else        ft0->SetLineColor(7);
    t0[ml] = ft0->GetParameter(0);
    t0PT[ml]->AddText(Form("%.2lf",t0[0]));
    t0PT[ml]->AddText(Form("%.2lf",ft0->GetParameter(1)));
    //Get Tmax fit
    TF1 *ftm = h[ml]->GetFunction("mt_tmax_fermi");
    if( ftm == NULL ) {
      t0PT[ml]->AddText(" ");
      t0PT[ml]->AddText(" ");
      t0PT[ml]->Draw();
      continue;
    }
    tmax[ml] = ftm->GetParameter(0)-t0[ml];
    t0PT[ml]->AddText(Form("%.2lf",tmax[ml]));
    t0PT[ml]->AddText(Form("%.2lf",ftm->GetParameter(1)));
    t0PT[ml]->Draw();
    if( ml==0 ) {
      ftm->SetLineColor(50);
    } else {
      ftm->SetLineColor(7);
      if( tmax[0]!=0. && tmax[1]!=0. ) {
        tdPT  = new TPaveText(0.53,0.38,0.98,0.45,"NDC");
	tdPT->AddText(Form("#DeltaTmaxML = %.1lf ns",tmax[0]-tmax[1]));
	if( TMath::Abs(tmax[0]-tmax[1]) > 5. ) tdPT->SetTextColor(2);
	tdPT->Draw();
      }
    }
  }     //end loop over ML

  if( Filename[0] != '\0' ) gPad->Print(Form("%s_%s_TimeML.png",Filename,htitle));
  else                      gPad->Print(Form("%s_TimeML.png",htitle));
  // Restore original histogram title
  h[0]->SetTitle(htitle_save);

  return 0;
}  //end T0Fit::TdcMLPlot

/************************************************************
*  Draw ML1+ML2 ADC spectrum on same plot
*  h = array with ADC histograms from both ML.
*  Presumes that ML histograms have already been fit with AdcFit
*  i.e. that they contain fit functions whose results are printed on plot.
*  Plots named <Filename>_<chamber>_ADCML.png or <chamber>_ADCML.png
************************************************************/
int T0Fit::AdcMLPlot(TH1F *h[2] ) {
  char htitle[80],htitle_save[80]; 

  gStyle->SetOptStat(0);

  // Protect for null histograms
  if( h[0] == NULL ) return -1;

  // Save original histogram title
  sprintf(htitle_save,"%s",h[0]->GetTitle());
  // Assume histogram name starts with chamber name
  sprintf(htitle,"%s",h[0]->GetName());
  htitle[7] = '\0';  //truncate htitle so be just chamber name
  // Reset histogram title if desired
  if( Hisname[0] == '\0' ) {
    h[0]->SetTitle(Form("%s ML",htitle));
  } else {
    h[0]->SetTitle(Form("%s %s ML",Hisname,htitle));
  }

  h[0]->SetStats(0);
  if( h[1] != NULL ) {
    h[1]->SetLineColor(8);
    h[1]->SetStats(0);
    //Reset h[0] scale if needed so h[1] does not get cut off
    if( h[0]->GetMaximum() < h[1]->GetMaximum() ) {
      h[0]->SetMaximum(h[1]->GetMaximum()*1.05);
    }
    h[0]->Draw();
    h[1]->Draw("same");
  } else {
    h[0]->Draw();
  }

//  text box locations  
  TPaveText *keyPT[2];
  keyPT[0] = new TPaveText(0.53,0.60,0.66,0.90,"NDC");
  //  keyPT[0]->SetTextAlign(11);
  keyPT[0]->AddText(" ");
  keyPT[0]->AddText("Entry");
  keyPT[0]->AddText("Mean");
  keyPT[0]->AddText("RMS");
  keyPT[0]->Draw();
  keyPT[1] = new TPaveText(0.53,0.375,0.66,0.60,"NDC");
  keyPT[1]->AddText("Peak");
  keyPT[1]->AddText("Width");
  keyPT[1]->AddText("Skew");

// Grab parameters from histogram fit functions
  TPaveText *enPT[2],*fitPT[2];
  for( int ml=0; ml<2; ml++ ) {
    if( h[ml] == NULL || h[ml]->GetEntries() == 0. ) continue;
    if( ml==0 ) {
      keyPT[1]->Draw();
      enPT[0] = new TPaveText(0.66,0.60,0.82,0.90,"NDC");
      fitPT[0] = new TPaveText(0.66,0.375,0.82,0.60,"NDC");
      enPT[0]->SetTextColor(4);
      fitPT[0]->SetTextColor(50);
    } else {
      enPT[1] = new TPaveText(0.82,0.60,0.98,0.90,"NDC");
      fitPT[1] = new TPaveText(0.82,0.375,0.98,0.60,"NDC");
      enPT[1]->SetTextColor(8);
      fitPT[1]->SetTextColor(7);
    }
    enPT[ml]->AddText(Form("ML%i",ml+1));
    enPT[ml]->AddText(Form("%.0f",h[ml]->GetEntries()));
    enPT[ml]->AddText(Form("%.1f",h[ml]->GetMean()));
    enPT[ml]->AddText(Form("%.1f",h[ml]->GetRMS()));
    enPT[ml]->Draw();
    //Get ADC fit
    TF1 *fadc = h[ml]->GetFunction("adc_fun");
    if( fadc == NULL ) continue;
    if( ml==0 ) fadc->SetLineColor(50);
    else        fadc->SetLineColor(7);
    fitPT[ml]->AddText(Form("%.1lf",fadc->GetParameter(1)));
    fitPT[ml]->AddText(Form("%.1lf",fadc->GetParameter(2)));
    fitPT[ml]->AddText(Form("%.2lf",fadc->GetParameter(3)));
    fitPT[ml]->Draw();
  }

  if( Filename[0] != '\0' ) gPad->Print(Form("%s_%s_ADCML.png",Filename,htitle));
  else                      gPad->Print(Form("%s_ADCML.png",htitle));
  // Restore original histogram title
  h[0]->SetTitle(htitle_save);
  return 0;
}    //end T0Fit::AdcMLPlot

// Convert NaNs and Infs to some default value 
Double_t T0Fit::CheckNaN( const Double_t x, const Double_t defaultx ) {
  if( isnan(x) || isinf(x) ) return defaultx;
  return x;
}

//  Set ROOT style parameters for plots - set notitle=1 to remove histogram titles from plots
void T0Fit::T0fitstyle( const int notitle ) {
  gStyle->SetTitleH(0.12);
  gStyle->SetTitleX(0.55);
  gStyle->SetTitleY(1.0);
  gStyle->SetStatW(0.48);
  gStyle->SetStatX(0.98);
  gStyle->SetStatY(0.90);
  gStyle->SetStatH(0.35);
  if( notitle ) {
    gStyle->SetTitleX(999.);
    gStyle->SetTitleY(999.);
  }
}

#endif
