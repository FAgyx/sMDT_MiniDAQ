
#include "AtlasStyle.h"
#include "AtlasUtils.h"

#include "TCanvas.h"
#include "TPad.h"

void TestLabel()
{

  SetAtlasStyle();

  TCanvas* test = new TCanvas("test","",0,0,800,600);

#ifdef __CINT__
  gROOT->LoadMacro("AtlasUtils.C");
#endif

  ATLAS_LABEL(0.2,0.8); myText(0.34,0.8,1,"Preliminary");

  myText(0.2,0.7,1,"2008 Cosmic muons");

}


#ifndef __CINT__

int main() { 
  TestLabel();
  gPad->Print("label.png");
  return 0;
}

#endif
