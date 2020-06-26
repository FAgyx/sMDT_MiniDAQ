#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <string>

#include "src/T0Fit.h"
#include "src/Geometry.cpp"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"



int DoT0Fit(TString filename = "run00187985_20190607.dat") {

  using namespace Muon;

  int runN = ((TObjString*)(TString(filename(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  Geometry geo = Geometry();
  geo.SetRunN(runN);

  TString input_dirname = "/atlas/data19/kevnels/sMDT/output/";
  TString input_pathname;
  char directory_name[256];
  char hist_name1[256];
  char hist_name2[256];
  char temp_name1[256];
  char temp_name2[256];
  char fitVecName[256];
  char input_directory_name[256];
  char input_path_name[256];
  char output_directory_name[256];
  char output_path_name[256];
  TCanvas *plot_canvas = new TCanvas();

  input_pathname = input_dirname + filename;
  input_pathname += ".dir/";
  input_dirname  = input_pathname;
  input_pathname += filename;
  input_pathname += ".out.root";
  
  // put .png plots in the output directory with all other data
  gSystem->cd(input_dirname.Data()); 

  TFile *p_input_rootfile = TFile::Open(input_pathname.Data());
  p_input_rootfile->ls();
  T0Fit *t0fit = new T0Fit("TestData","Test Data");
  TFile f("T0.root", "recreate");


  // save fits to a new directory under 
  gSystem->Exec("mkdir T0Fits");
  gSystem->cd("T0Fits");


  for(Int_t tdc_id = 0; tdc_id < Geometry::MAX_TDC; tdc_id++) {
    for (Int_t ch_id = 0; ch_id != Geometry::MAX_TDC_CHANNEL; ch_id++) {
      if (!geo.IsActiveTDCChannel(tdc_id, ch_id)) {continue;}

      sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id, Geometry::MAX_TDC);
      //sprintf(temp_name1, "tdc_%d_tdc_time_spectrum_original", tdc_id);
      //sprintf(temp_name2, "tdc_%d_adc_time_spectrum", tdc_id);
      sprintf(temp_name1, "tdc_%d_channel_%d_tdc_time_spectrum_corrected", tdc_id, ch_id);
      sprintf(temp_name2, "tdc_%d_channel_%d_adc_time_spectrum", tdc_id, ch_id);
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
      plot_canvas->cd();
      myTDChist->Draw();
      myADChist->Draw();
      t0fit->TdcFit(myTDChist,0,1);
      t0fit->AdcFit(myADChist,1);
      
      
      
      TVectorD fitData = TVectorD(NT0FITDATA);
      for(Int_t i = 0; i < NT0FITDATA; i++) {
	fitData[i] = t0fit->FitData[i];
      }
      sprintf(fitVecName, "FitData_tdc_%d_channel_%d", tdc_id, ch_id);
      fitData.Write(fitVecName);
      

	

    }
  }
  f.Close();
  delete t0fit;
  delete p_input_rootfile;
  delete plot_canvas;
  return 0;
}
