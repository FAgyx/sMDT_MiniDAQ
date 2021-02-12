
/*******************************************************************************
  file name: TotalHit.cxx
  author: Yuxiang Guo
  created: 01/25/2019
  last modified: 04/26/2019

  description:
  -Decode .raw data from HPTDC and save data to ntuple

  remark:
  -Learned basic decode method from Shuzhou Zhang, redeveloped and added new
  function for new HPTDC data format.

*******************************************************************************/
#include "macros/GlobalIncludes.h"
#include <stdio.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

// ROOT includes
#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"

// Geometry includes
#include "src/CheckEvent.cpp"

// Muon Reconstruction includes
#include "src/Signal.cpp"
#include "src/EventID.cpp"
#include "src/Event.cpp"
#include "src/EventDisplay.cpp"
#include "src/Geometry.cpp"
#include "src/Hit.cpp"
#include "src/Cluster.cpp"
#include "src/TimeCorrection.cpp"
#include "src/HitFinder.cpp"
#include "src/HitFinder_BCID.cpp"
#include "src/HitCluster.cpp"

//#define DECODER_DEBUG // comment this line when debugging information is not needed
//#define SET_MAXWORDS // comment this line if you want to decode the whole data words
#define SAVE_TRACKS_OUT_OF_ROOT // comment this line if you don't need to save plots out of rootfile 


using namespace std;
using namespace Muon;

int TotalHit(TString filename = "20200723_174803.dat") {
  gROOT->SetBatch(kTRUE); // set to batch mode to inprove the speed
  int maxEventCount = 1000000000;
  // int maxEventCount = 100;
  gStyle->SetOptStat(10); //only print entries
  gStyle->SetTitleX(999.);//hist no title
  gStyle->SetTitleY(999.);
  gStyle->SetStatY(0.9);                
	// Set y-position (fraction of pad size)
	gStyle->SetStatX(0.9);                
	// Set x-position (fraction of pad size)
	gStyle->SetStatW(0.25);                
	// Set width of stat-box (fraction of pad size)
	gStyle->SetStatH(0.25);                
	// Set height of stat-box (fraction of pad size)

  
  // open input file
  TString input_filename = "data/";
  TString fn = TString(filename);
  input_filename += filename;
  ifstream data_in_flow;
  data_in_flow.open(input_filename.Data());

  data_in_flow.seekg(0, data_in_flow.end);
  unsigned int data_in_flow_length = data_in_flow.tellg(); // get file size
  if (data_in_flow_length ==0) {
    printf("file name incorrect!\n");
    return 1;
  }
  data_in_flow.seekg(0, data_in_flow.beg);

  // create output file
  system("mkdir output");
  chdir("output");
  char output_directoryname[256];
  strcpy(output_directoryname, filename);
  strcat(output_directoryname, ".dir");
  if (mkdir(output_directoryname, 0777) == -1) {
    cerr << strerror(errno) << endl;
  }
  chdir(output_directoryname);
  char output_filename[256];
  strcpy(output_filename, filename);
  strcat(output_filename, ".out");

  char output_root_filename[200];
  strcpy(output_root_filename, output_filename);
  strcat(output_root_filename, ".root");
  TFile *p_output_rootfile = new TFile(output_root_filename, "RECREATE");


  // prepare file structure for event display
  TDirectory *event_track[2];
  char track_group_name[128];

  int runN = ((TObjString*)(TString(fn(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  Geometry geo = Geometry();
  geo.SetRunN(runN);
  static TimeCorrection tc = TimeCorrection();
  // static EventDisplay   ed = EventDisplay();


  TH1F *p_leading_time = new TH1F("leading time spectrum", "leading time spectrum", 100, 0, 1000);
  TH1F *p_trailing_time = new TH1F("trailing time spectrum", "trailing time spectrum", 100, 0, 1000);
  TH1F *p_hits_distribution[Geometry::MAX_TUBE_LAYER];
  char histogram_name[256];
  for (Int_t layer_id = 0; layer_id != Geometry::MAX_TUBE_LAYER; layer_id++) {
    sprintf(histogram_name, "layer_%d_hits_distribution", layer_id);
    p_hits_distribution[layer_id] = new TH1F(histogram_name, histogram_name, Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5);
  }
  TH1F *p_tdc_time[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_tdc_time_original[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_tdc_time_corrected[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_tdc_time_selected[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_adc_time[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_tdc_tdc_time[Geometry::MAX_TDC];
  TH1F *p_tdc_tdc_time_original[Geometry::MAX_TDC];
  TH1F *p_tdc_tdc_time_corrected[Geometry::MAX_TDC];
  TH1F *p_tdc_tdc_time_selected[Geometry::MAX_TDC];
  TH1F *p_tdc_adc_time[Geometry::MAX_TDC];
  TH1F *p_tdc_channel[Geometry::MAX_TDC];
  TH2F *p_adc_vs_tdc[Geometry::MAX_TDC];
  TDirectory *tdc_directory[Geometry::MAX_TDC];
  char directory_name[256];

  TString h_name;
  unsigned int word;
  unsigned int header_type;
  EventID currEventID;
  EventID prevEventID = EventID(0x00000000);
  vector<Signal> trigVec;
  vector<Signal>  sigVec;
  bitset<4> header;
  Signal sig;
  Event  event = Event();
  TTree* eTree = new TTree("eTree", "eTree");
  eTree->Branch("event", "Event", &event);
  cout << "Processing..." << endl;

  int total_hit[MAX_TDC][MAX_TDC_CHANNEL];
  memset(total_hit,0,sizeof(total_hit));
  int nloop = 0;
  while (data_in_flow.read((char *) &word, sizeof(word))  && nloop<maxEventCount) {
    nloop++;
    header = word >> 28; // get the four bits header of this word
    header_type = static_cast<unsigned int>((header.to_ulong()));

    if (header_type == Signal::RISING){
      sig = Signal(word, 0);
      total_hit[sig.TDC()][sig.Channel()]++
    }
  }

  TCanvas *rate_canvas;
  geo = Geometry();
  TGraph *p_tdc_hit_rate_graph[Geometry::MAX_TDC];
  int pad_num;
  for (int i = 0; i < Geometry::MAX_TDC_CHANNEL; i++){
    p_tdc_hit_rate_x[i] = i;
  }
  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    if (geo.IsActiveTDC(tdc_id)) {
      if (tdc_id == geo.TRIGGER_MEZZ){
      }
      else{
        pad_num = geo.TDC_COL[tdc_id]+6*(1-geo.TDC_ML[tdc_id]);
        rate_canvas->cd(pad_num);
        // gPad->SetLogy();
        h_name.Form("tdc_%d_hit_rate", tdc_id);
        p_tdc_hit_rate_graph[tdc_id] = new TGraph(Geometry::MAX_TDC_CHANNEL, p_tdc_hit_rate_x, total_hit[tdc_id]);
        p_tdc_hit_rate_graph[tdc_id]->SetFillColor(4);
        p_tdc_hit_rate_graph[tdc_id]->SetTitle(h_name);
        p_tdc_hit_rate_graph[tdc_id]->GetXaxis()->SetTitle("Channel No.");       
        p_tdc_hit_rate_graph[tdc_id]->GetXaxis()->SetLimits(-0.5,23.5);
        p_tdc_hit_rate_graph[tdc_id]->GetYaxis()->SetTitle("Rate(kHz)");          
        p_tdc_hit_rate_graph[tdc_id]->Draw("AB");
        //if (gSystem->ProcessEvents()) break;
      }
    }
  }
  
  //gROOT->SetBatch(kFALSE);
  return 0;
}
    
