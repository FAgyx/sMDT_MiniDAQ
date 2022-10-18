#include "MuonReco/T0Fit.h"

TString fitDataNames[NT0FITDATA] = {"NEvents",
                                    "T0",
                                    "T0Error",
                                    "T0Slope",
                                    "T0SlopeError",
                                    "Background",
                                    "BackgroundError",
                                    "T0Chi2",
                                    "TMax",
                                    "TMaxError",
                                    "TMaxSlope",
                                    "TMaxSlopeError",
                                    "TMaxChi2",
                                    "ADCPeak",
                                    "ADCPeakError",
                                    "ADCWidth",
                                    "ADCWidthError",
                                    "ADCSkew",
                                    "ADCSkewError",
                                    "ADCChi2"};

TString fitDataUnits[NT0FITDATA] = {"", 
                                    " [ns]",
                                    " [ns]",
                                    " [ns]",
                                    " [ns]",
                                    "",
                                    "",
                                    "",
                                    " [ns]",
                                    " [ns]",
                                    " [ns]",
                                    " [ns]",
                                    "",
                                    " [ns]",
                                    " [ns]",
                                    " [ns]",
                                    " [ns]",
                                    "",
                                    "",
                                    ""};

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
int T0Fit::TdcFit(TH1F *h, double pardata[NT0FITDATA], const int plot ) {
  double t0,slope,ampl,xmin,xmax,maxval=130.;
  double tmax,tmslope,tmback=0.,A=3.,B=-0.0015;
 
  clearfitdata();

  // Protect for null histograms and low statistics time spectra
  if( h == 0 || h->GetEntries()<1000. ) return -1;
  printf("Fitting %s\n",h->GetName());

  //gStyle->SetOptStat("e"); 

  // Reset histogram title if desired
  if( Hisname[0] != '\0' ) {
    h->SetTitle(Form("%s %s",Hisname,h->GetTitle()));
  }

  FitData[0] = CheckNaN(h->GetEntries());
  
//  default text box locations  
  TPaveText *t0PT = new TPaveText(0.55,0.78,0.95,0.95,"NDC");
  TPaveText *tmPT = new TPaveText(0.55,0.63,0.95,0.78,"NDC");
  TPaveText *tdPT = new TPaveText(0.55,0.55,0.95,0.63,"NDC");

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
  xmax = t0 + 35;

  printf("Estimated parameters t0=%.1f slope=%.1f back=%.1f ampl=%.1f xmin=%.1f xmax=%.1f maxval=%.1f\n",t0,slope,0.,ampl,xmin,xmax,maxval);

    // Define TF1 function for T0 fit
  TF1 *ft0 = new TF1("mt_t0_fermi",mt_t0_fermi,xmin,xmax,4);
  ft0->SetParameters(t0,slope,0.,ampl);
  //changed
  ft0->SetLineColor(kRed);
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
    tmax = t0+180;

    xmin = tmax - 80.;
    xmax = tmax + 100.;
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
    double dtmax  = ftm->GetParameter(0) - ft0->GetParameter(0);
    double dtmaxe = TMath::Sqrt( ftm->GetParError(0)*ftm->GetParError(0)+
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
    if( Filename[0] != '\0' ) gPad->Print(IOUtility::join(_dir, Form("%s_%s.png",Filename,h->GetName())));
    else                      gPad->Print(IOUtility::join(_dir, Form("%s.png",h->GetName())));
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

  //gStyle->SetOptStat("emr"); 
  TPaveText *adcPT = new TPaveText(0.55,0.63,0.95,0.95,"NDC");

//  Refit ADC
  double n    = h->GetEntries();
  double rms  = h->GetRMS();
  double mean = h->GetMean();
  double ymin = mean-2*rms;
  if( ymin < 60. ) ymin=60.;
  double ymax = mean+1.5*rms;
  //Define TF1 function for skew normal fit of ADC spectrum 
  TF1 *fadc = new TF1("adc_fun",skewnormal,ymin,ymax,4);
  fadc->SetParameters(n,mean,rms,1.5);
  fadc->SetParLimits(1,mean-rms,mean+0.5*rms);
  fadc->SetParLimits(2,rms*0.5,rms*1.5);
  fadc->SetParLimits(3,1.,4.);
  fadc->SetLineColor(kRed);
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
    if( Filename[0] != '\0' ) gPad->Print(IOUtility::join(_dir, Form("%s_%s.png",Filename,h->GetName())));
    else                   gPad->Print(IOUtility::join(_dir, Form("%s.png",h->GetName())));
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
  double t0[2]={0.,0.};
  double tmax[2]={0.,0.};
  for( int ml=0; ml<2; ml++ ) {
    if( h[ml] == NULL || h[ml]->GetEntries() == 0. ) continue;
    if( ml==0 ) {
      keyPT[1]->Draw();
      enPT[0] = new TPaveText(0.66,0.75,0.82,0.90,"NDC");
      t0PT[0] = new TPaveText(0.66,0.45,0.82,0.75,"NDC");
      enPT[0]->SetTextColor(4);
      t0PT[0]->SetTextColor(50);
    } else {
      enPT[1] = new TPaveText(0.82,0.75,0.90,0.90,"NDC");
      t0PT[1] = new TPaveText(0.82,0.45,0.95,0.75,"NDC");
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
        tdPT  = new TPaveText(0.53,0.38,0.95,0.45,"NDC");
	tdPT->AddText(Form("#DeltaTmaxML = %.1lf ns",tmax[0]-tmax[1]));
	if( TMath::Abs(tmax[0]-tmax[1]) > 5. ) tdPT->SetTextColor(2);
	tdPT->Draw();
      }
    }
  }     //end loop over ML

  if( Filename[0] != '\0' ) gPad->Print(IOUtility::join(_dir, Form("%s_%s_TimeML.png",Filename,htitle)));
  else                      gPad->Print(IOUtility::join(_dir, Form("%s_TimeML.png",htitle)));
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
      enPT[1] = new TPaveText(0.82,0.60,0.95,0.90,"NDC");
      fitPT[1] = new TPaveText(0.82,0.375,0.95,0.60,"NDC");
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

  if( Filename[0] != '\0' ) gPad->Print(IOUtility::join(_dir, Form("%s_%s_ADCML.png",Filename,htitle)));
  else                      gPad->Print(IOUtility::join(_dir, Form("%s_ADCML.png",htitle)));
  // Restore original histogram title
  h[0]->SetTitle(htitle_save);
  return 0;
}    //end T0Fit::AdcMLPlot

// Convert NaNs and Infs to some default value 
double T0Fit::CheckNaN( const double x, const double defaultx ) {
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

