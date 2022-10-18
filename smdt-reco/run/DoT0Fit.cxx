#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "MuonReco/T0Fit.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/T0Reader.h"
#include "MuonReco/EventDisplay.h"

#include "TSystem.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TTree.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TVectorD.h"

using namespace MuonReco;

int main(int argc, char* argv[]) {

  SetAtlasStyle();

  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");
  int useFullTDC  = cp.items("T0Fit").getInt("UseFullTDC", 0, 0);
  TString filename = cp.items("General").getStr("RawFileName");

  Geometry geo = Geometry(cp);

  TString input_dirname = IOUtility::join(getenv("SMDT_DIR"), "output");
  TString input_pathname;
  char directory_name[256];
  char hist_name1[256];
  char hist_name2[256];
  char temp_name1[256];
  char temp_name2[256];
  char fitVecName[256];
  TCanvas *plot_canvas = new TCanvas();

  input_pathname = IOUtility::getDecodedOutputFilePath(runN);
  
  // put .png plots in the output directory with all other data

  TString outname = (useFullTDC) ? "T0_full_tdc.root" : "T0.root";
  TString dirname = (useFullTDC) ? "T0Fits_mezz"      : "T0Fits";

  TFile *p_input_rootfile = TFile::Open(input_pathname.Data());
  p_input_rootfile->ls();
  T0Fit *t0fit = new T0Fit("TestData","Test Data");
  TFile f(IOUtility::join(IOUtility::getRunOutputDir(runN), outname), "recreate");  
  TTree* tree = new TTree("FitData", "TDC/ADC fit data");
  TVectorD fitData = TVectorD(NT0FITDATA);
  int tdc_id=0, ch_id=0, layer=0, column=0;
  tree->Branch("tdc", &tdc_id, "tdc/I");
  tree->Branch("channel", &ch_id, "channel/I");
  tree->Branch("layer", &layer, "layer/I");
  tree->Branch("column", &column, "column/I");
  for (int i = 0; i < NT0FITDATA; i++) {
    tree->Branch(fitDataNames[i], &fitData[i], fitDataNames[i] + "/D");
  }


  t0fit->_dir = IOUtility::join(IOUtility::getRunOutputDir(runN), "T0Fits", kTRUE);

  for(tdc_id = 0; tdc_id < Geometry::MAX_TDC; tdc_id++) {
    for (ch_id = -1; ch_id != Geometry::MAX_TDC_CHANNEL; ch_id++) {
      if (ch_id >= 0 && !geo.IsActiveTDCChannel(tdc_id, ch_id)) {continue;}

      sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id, Geometry::MAX_TDC);
      if (ch_id < 0) {
	sprintf(temp_name1, "tdc_%d_tdc_time_spectrum_corrected", tdc_id);
	sprintf(temp_name2, "tdc_%d_adc_time_spectrum", tdc_id);
      }
      else {
	sprintf(temp_name1, "tdc_%d_channel_%d_tdc_time_spectrum_corrected", tdc_id, ch_id);
	sprintf(temp_name2, "tdc_%d_channel_%d_adc_time_spectrum", tdc_id, ch_id);
      }
      strcpy(hist_name1, directory_name);
      strcat(hist_name1, "/");
      strcat(hist_name1, temp_name1);
      strcpy(hist_name2, directory_name);
      strcat(hist_name2, "/");
      strcat(hist_name2, temp_name2);
      TH1F *myTDChist;
      TH1F *myADChist;
      p_input_rootfile->GetObject(hist_name1, myTDChist);
      p_input_rootfile->GetObject(hist_name2, myADChist);
      if (!(myTDChist->GetEntries() && myADChist->GetEntries())) continue;

      plot_canvas->cd();
      myTDChist->Draw();
      myADChist->Draw();
      t0fit->TdcFit(myTDChist,0,1);
      t0fit->AdcFit(myADChist,1);
      
            
      for(Int_t i = 0; i < NT0FITDATA; i++) {
	fitData[i] = t0fit->FitData[i];
      }

      if (ch_id < 0) {
	column = -1;
	layer  = -1;
      }
      else geo.GetHitLayerColumn(tdc_id, ch_id, &layer, &column);      
      if (column < Geometry::MAX_TUBE_COLUMN) tree->Fill();

    }
  }
  tree->Write();
  f.Close();
  delete t0fit;
  delete p_input_rootfile;
  delete plot_canvas;

  // read t0 fit data and make tube histograms

  T0Reader* t0reader = T0Reader::GetInstance(IOUtility::getT0FilePath(runN));
  int tdc, ch, col;
  TVectorD* fitParams = new TVectorD(NT0FITDATA);
  TH2D* ADCHIST = new TH2D("adchist", "", Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5,Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);
  for (int _i = 1; _i <= Geometry::MAX_TUBE_COLUMN; _i++) {
    for (int _j = 1; _j <= Geometry::MAX_TUBE_LAYER; _j++) {
      ADCHIST->SetBinContent(_i, _j, 0);
    }
  }
  t0reader->SetBranchAddresses(&tdc, &ch, &layer, &col, fitParams);
  for (int iEntry = 0; iEntry < t0reader->GetEntries(); iEntry++) {
    t0reader->GetEntry(iEntry);
    ADCHIST->SetBinContent(col+1,layer+1,fitParams[0][13]);
    std::cout << "col: " << col << " lay: " << layer << " peak adc: " << fitParams[0][13] << std::endl;
  }
  
  EventDisplay ed = EventDisplay();
  ed.SetOutputDir(IOUtility::join(IOUtility::getRunOutputDir(runN), "T0Fits", kTRUE));
  ed.DrawTubeHist(geo, ADCHIST, NULL, kTRUE);

  return 0;
}
