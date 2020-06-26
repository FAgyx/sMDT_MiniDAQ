#include <iostream>
#include <cmath>

#include "Rtypes.h"

#include "AtlasUtils.h"
#include "AtlasStyle.h"
#include "AtlasLabels.h"
// #ifndef __CINT__
// these are not headers - do not treat them as such
// #include "AtlasStyle.C"
// #include "AtlasLabels.C"
// #endif

#include "TCanvas.h"
#include "TFile.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TGraphErrors.h"

using namespace std;

const Int_t GMAX=864;

const int nren=3;
static const double mur[nren] = {1.0,0.25,4.0};
static const double muf[nren] = {1.0,0.25,4.0};
const unsigned int NUMPDF=41;

TGraphErrors* GetGraph(Int_t ir, Int_t ifs,Int_t icut, Int_t ipdf);


void AtlasExample() 
{ 

#ifdef __CINT__
  gROOT->LoadMacro("AtlasUtils.C");
#endif

  SetAtlasStyle();

  Int_t icol1=5;
  Int_t icol2=5;

  TCanvas* c1 = new TCanvas("c1","single inclusive jets",50,50,600,600);
  // TCanvas* c1 = new TCanvas("c1","single inclusive jets");
  TPad* thePad = (TPad*)c1->cd();
  thePad->SetLogy();

  Double_t ymin=1.e-3;  Double_t ymax=2e7;
  Double_t xmin=60.00;  Double_t xmax=3500.;
  TH1F *h1 = thePad->DrawFrame(xmin,ymin,xmax,ymax);
  h1->SetYTitle("d#sigma_{jet}/dE_{T,jet} [fb/GeV]");
  h1->SetXTitle("E_{T,jet}  [GeV]");
  h1->GetYaxis()->SetTitleOffset(1.4);
  h1->GetXaxis()->SetTitleOffset(1.4);
  //h1->GetXaxis()->SetNdivisions(5);
  h1->Draw();

  const Int_t ncut=1;
  TGraphErrors *data[ncut];

  for (Int_t icut=0; icut<ncut; icut++) { // loop over cuts

    TGraphErrors *g1[nren][ncut];
    for (Int_t ir=0; ir<nren; ir++) { // loop over ren scale
      g1[ir][icut]= GetGraph(ir,ir,icut,0);
      if (g1[ir][icut]) 
	cout << g1[ir][icut]->GetTitle() << " found "  << g1[ir][icut]->GetName()  << endl;
      else { 
	cout << " g1 not  found " << endl; 
	return; 
      } 
      g1[ir][icut]->SetLineColor(1);
      g1[ir][icut]->SetMarkerStyle(0);
      //g1[ir][icut]->Draw("C");
    }

    char daname[100];
    sprintf(daname,"data_%d",icut); 
    data[icut]=(TGraphErrors*)g1[0][icut]->Clone(daname); 
    data[icut]->SetMarkerStyle(20);
    data[icut]->SetMarkerColor(1);

    // just invent some data
    for (Int_t i=0; i< data[icut]->GetN(); i++) {
      Double_t x1,y1,e,dx1=0.;
      data[icut]->GetPoint(i,x1,y1);
      Double_t r1 = 0.4*(gRandom->Rndm(1)+2);
      Double_t r2 = 0.4*(gRandom->Rndm(1)+2);
      //cout << " i= " << i << " x1= " << x1 << " y1= " << y1 << " r= " << r << endl;
      Double_t y;
      if (icut==0) y=r1*y1+r1*r2*r2*x1/50000.;
      else         y=r1*y1;
      e=sqrt(y*1000)/200;
      data[icut]->SetPoint(i, x1,y);
      data[icut]->SetPointError(i,dx1,e);
    }
    //data[icut]->Print();
  
    TGraphAsymmErrors* scale[ncut];
    TGraphAsymmErrors* scalepdf[ncut];

    scale[icut]=  myMakeBand(g1[0][icut],g1[1][icut],g1[2][icut]);
    //printf(" band1: \n");
    //scale->Print();

    scalepdf[icut]=(TGraphAsymmErrors* ) scale[icut]->Clone("scalepdf");

    TGraphErrors *gpdf[NUMPDF][ncut];
    for (Int_t ipdf=0; ipdf<NUMPDF; ipdf++) {
      gpdf[ipdf][icut]= GetGraph(0,0,icut,ipdf);
      if (gpdf[ipdf][icut]) 
	cout << gpdf[ipdf][icut]->GetTitle() << " found "  << gpdf[ipdf][icut]->GetName() << endl;
      else { 
	cout << " gpdf not  found " << endl; 
	return; 
      } 
      gpdf[ipdf][icut]->SetLineColor(2);
      gpdf[ipdf][icut]->SetLineStyle(1);
      gpdf[ipdf][icut]->SetMarkerStyle(0);
      myAddtoBand(gpdf[ipdf][icut],scalepdf[icut]); 
    }

    scalepdf[icut]->SetFillColor(icol2);
    scalepdf[icut]->Draw("zE2"); 
    scale[icut]->SetFillColor(icol1);
    scale[icut]->Draw("zE2");
    g1[0][icut]->SetLineWidth(3);
    g1[0][icut]->Draw("z");
    data[icut]->Draw("P");
    
  }

  myText(       0.3,  0.85, 1, "#sqrt{s}= 14 TeV");
  myText(       0.57, 0.85, 1, "|#eta_{jet}|<0.5");
  myMarkerText( 0.55, 0.75, 1, 20, "Data 2009",1.3);
  myBoxText(    0.55, 0.67, 0.05, icol1, "NLO QCD");

  //ATLAS_LABEL(0.2,0.2); myText( 0.37,0.2,1,"Preliminary");

  // new method for ATLAS labels. Use this!
  //ATLASLabel(0.2,0.2);
  ATLASLabel(0.2,0.2,"Preliminary");
  //ATLASLabel(0.2,0.2,"Work in progress");

  c1->Print("AtlasExample.eps");
  c1->Print("AtlasExample.png");
  c1->Print("AtlasExample.pdf");

}

TGraphErrors* GetGraph(Int_t ir, Int_t ifs,Int_t icut, Int_t ipdf) 
{ 
 
  const bool debug=0;

  const char *cuts[5] = 
    {"0.0 <= |eta| < 0.5",
     "0.5 <= |eta| < 1.0",
     "1.0 <= |eta| < 1.5",
     "1.5 <= |eta| < 2.0",
     "2.0 <= |eta| < 3.0"};
  
  const double mur[] = {1.0,0.25,4.0};
  const double muf[] = {1.0,0.25,4.0};

  TFile *f1 = TFile::Open("nlofiles.root");
  if (f1&&debug) cout << " file opened " << endl;

  char gname[100];
  char tname[100];

  if (debug) cout << " mur= " << mur[ir] << " muf= " << muf[ifs] 
		  << " iut= " << icut << " ipdf= " << ipdf << endl;

  if (ipdf>=0)
    sprintf(tname," E_T (mu_r=%g, mu_f=%g);%s Pdf: %d",mur[ir],muf[ifs],cuts[icut],ipdf);
  else
    sprintf(tname," E_T %s Ms= %d",cuts[icut],-ipdf);

  if (debug) cout << "**GetGraph: Look for: " << tname << endl;

  TGraphErrors* g1 = 0;

  for (int i=1; i<=GMAX; i++){ 
    sprintf(gname,"full_%d",i);
    // sprintf(gname,"born_%d",i);
    sprintf(gname,"full_%d",i);
    g1=(TGraphErrors*) f1->Get(gname);
    if (!g1) { cout << gname << "  not found " << endl; return g1=0; } 
    
    const char *title=g1->GetTitle();
    
    if (debug){ 
      cout << " Title= " << title << endl;
      cout << " tname= " << tname << endl;
      cout << " strcmp: " << strcmp(title,tname) << endl;
    }

    if (strcmp(title,tname)==0) break;
    g1=0;
  }

  if (!g1) return 0;
  if (debug) cout << " found: " << g1->GetTitle() << endl;  
  
  return g1;
} 



#ifndef __CINT__

int main()  { 
  
  AtlasExample();

  return 0;
}

#endif
