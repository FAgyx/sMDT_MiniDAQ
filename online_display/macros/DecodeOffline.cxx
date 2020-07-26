
/*******************************************************************************
  file name: DecodeRawData.cxx
  author: Zhe Yang
  created: 01/25/2019
  last modified: 04/26/2019

  description:
  -Decode .raw data from HPTDC and save data to ntuple

  remark:
  -Learned basic decode method from Shuzhou Zhang, redeveloped and added new
  function for new HPTDC data format.

*******************************************************************************/
#define NEWTDC_NUMBER 9
#define WIDTH_RES 1
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
#include "src/HitCluster.cpp"

//#define DECODER_DEBUG // comment this line when debugging information is not needed
//#define SET_MAXWORDS // comment this line if you want to decode the whole data words
#define SAVE_TRACKS_OUT_OF_ROOT // comment this line if you don't need to save plots out of rootfile 

#define TOTAL_BIN_QUANTITY 1024 // set bin quantity of the plot 


using namespace std;
using namespace Muon;

int DecodeOffline(TString filename = "20200723_174803.dat") {
  gROOT->SetBatch(kTRUE); // set to batch mode to inprove the speed
  int maxEventCount = 1000000000;

  
  // open input file
  TString input_filename = "data/";
  TString fn = TString(filename);
  input_filename += filename;
  ifstream data_in_flow;
  data_in_flow.open(input_filename.Data());

  data_in_flow.seekg(0, data_in_flow.end);
  unsigned int data_in_flow_length = data_in_flow.tellg(); // get file size
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

  for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    // create output directories
    sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id,Geometry::MAX_TDC);
    tdc_directory[tdc_id] = p_output_rootfile->mkdir(directory_name);
    tdc_directory[tdc_id]->cd();
    if (mkdir(directory_name, 0777) == -1) {
      cerr << strerror(errno) << endl;
    }
    
    h_name.Form("tdc_%d_tdc_time_spectrum_corrected", tdc_id);
    p_tdc_tdc_time_corrected[tdc_id] = new TH1F(h_name, h_name,TOTAL_BIN_QUANTITY, -400, 400);
    p_tdc_tdc_time_corrected[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_tdc_time_corrected[tdc_id]->GetYaxis()->SetTitle("entries");
    
    h_name.Form("tdc_%d_adc_time_spectrum", tdc_id);
    p_tdc_adc_time[tdc_id] = new TH1F(h_name, h_name, TOTAL_BIN_QUANTITY / 2, 0, 400);
    p_tdc_adc_time[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_adc_time[tdc_id]->GetYaxis()->SetTitle("entries");
    
    
    for (Int_t channel_id = 0; channel_id != Geometry::MAX_TDC_CHANNEL; channel_id++) {
      h_name.Form("tdc_%d_channel_%d_tdc_time_spectrum_corrected", tdc_id, channel_id);
      p_tdc_time_corrected[tdc_id][channel_id] = new TH1F(h_name,h_name, TOTAL_BIN_QUANTITY,-400, 400);
      p_tdc_time_corrected[tdc_id][channel_id]->GetXaxis()->SetTitle("time/ns");
      p_tdc_time_corrected[tdc_id][channel_id]->GetYaxis()->SetTitle("entries");
      
      h_name.Form("tdc_%d_channel_%d_tdc_time_spectrum", tdc_id, channel_id);
      p_tdc_time[tdc_id][channel_id] = new TH1F(h_name,h_name, TOTAL_BIN_QUANTITY,-400, 400);
      p_tdc_time[tdc_id][channel_id]->GetXaxis()->SetTitle("time/ns");
      p_tdc_time[tdc_id][channel_id]->GetYaxis()->SetTitle("entries");

      h_name.Form("tdc_%d_channel_%d_adc_time_spectrum", tdc_id, channel_id);
      p_adc_time[tdc_id][channel_id] = new TH1F(h_name, h_name,TOTAL_BIN_QUANTITY / 2, 0, 400);
      p_adc_time[tdc_id][channel_id]->GetXaxis()->SetTitle("time/ns");
      p_adc_time[tdc_id][channel_id]->GetYaxis()->SetTitle("entries");
      
    }
  } // end for: all TDC



  TH2D* hitByLC = new TH2D("hitByLC", "All hits on tubes (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  hitByLC->SetStats(0);
  TH2D* badHitByLC = new TH2D("badHitByLC", "Hits on tubes outside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  badHitByLC->SetStats(0);
  TH2D* goodHitByLC = new TH2D("goodHitByLC", "Hits on tubes inside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  goodHitByLC->SetStats(0);

  int hitL, hitC;
  int current_track_group = 0;
  Bool_t first_signal_flag[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  int temp_track_group;
  bool pass_event_check;
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

  unsigned long total_triggers = 0;
  unsigned long total_events   = 0;
  unsigned long total_triggers_pass = 0;
  unsigned long total_events_pass = 0;
  unsigned long total_signals = 0;
  unsigned long total_signals_pass = 0;
  unsigned long total_events_fail  = 0;
  unsigned long event_print = 100;


  int nloop = 0;
  while (data_in_flow.read((char *) &word, sizeof(word))  && nloop<maxEventCount) {
    nloop++;
    header = word >> 28; // get the four bits header of this word
    header_type = static_cast<unsigned int>((header.to_ulong()));
    
    if (header_type == Signal::GROUP_HEADER || header_type == Signal::TDC_HEADER || header_type == Signal::TDC_TRAILER) {
      currEventID = EventID(word);

      // analyze data if we reached a header for a new event 
      if (currEventID.ID() != prevEventID.ID()) {
      	prevEventID = currEventID;
      	if (total_events % event_print == 0) {
          std::cout << "Processing Event " << total_events << std::endl;
          if (TMath::Floor(TMath::Log10(total_events)) > TMath::Floor(TMath::Log10(event_print))) event_print*=10;
        }

      	total_events++;
              event = Event(trigVec, sigVec, currEventID);
      	DoHitFinding(&event,    tc,0);
      	DoHitClustering(&event, geo);
      	pass_event_check = kTRUE;
      	//pass_event_check = CheckEvent(event, geo);
      	event.SetPassCheck(pass_event_check);
      	event.CheckClusterTime();

      	if (pass_event_check) {      
          eTree->Fill();
        }
          // for (Cluster c : event.Clusters()) {
          for (Hit h : event.WireHits()) {

            // for (Hit h : c.Hits()) {

              p_tdc_tdc_time_corrected[h.TDC()]->Fill(h.CorrTime());
              p_tdc_adc_time          [h.TDC()]->Fill(h.ADCTime()); 

              p_tdc_time_corrected[h.TDC()][h.Channel()]->Fill(h.CorrTime());
              p_tdc_time          [h.TDC()][h.Channel()]->Fill(h.DriftTime());
              p_adc_time          [h.TDC()][h.Channel()]->Fill(h.ADCTime());

              geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);
              hitByLC->Fill(hitC, hitL);
              if (h.CorrTime() <-200 || h.CorrTime()>200)
                badHitByLC->Fill(hitC, hitL);
              else
                goodHitByLC->Fill(hitC, hitL);
              p_hits_distribution[hitL]->Fill(hitC);
          }
          // }

          for (Hit h : event.TriggerHits()) {
            p_tdc_time_corrected[h.TDC()][h.Channel()]->Fill(h.DriftTime());
            p_adc_time          [h.TDC()][h.Channel()]->Fill(h.ADCTime());
          }
        // }

      	
      	for (Int_t i = 0; i != Geometry::MAX_TDC; i++) {
      	  for (Int_t j = 0; j != Geometry::MAX_TDC_CHANNEL; j++) {
      	    first_signal_flag[i][j] = kFALSE;
      	  }
      	}

      	if (pass_event_check) {
      	  total_events_pass++;
      	}
      	else {
      	  total_events_fail++;
      	}

      	// plot the event
      	// plot the first 10 events meeting and not meeting the pass event check criteria
      	if ((pass_event_check && total_events_pass < 10) || (!pass_event_check && total_events_fail < 10)) { 

      	  if (pass_event_check)
      	    sprintf(track_group_name, "events_passing");
      	  else
      	    sprintf(track_group_name, "events_failing");
      	  
      	  if ((pass_event_check && total_events_pass == 1) || (!pass_event_check&& total_events_fail == 1) )
      	    event_track[(int)pass_event_check] = p_output_rootfile->mkdir(track_group_name);

      	  
          #ifdef SAVE_TRACKS_OUT_OF_ROOT
      	  if (mkdir(track_group_name, 0777) == -1) {
      	    cerr << strerror(errno) << endl;
      	  }
          #endif

      	  event_track[(int)pass_event_check]->cd();
      	  // chdir(track_group_name);
      	  // ed.DrawEvent(event, geo, event_track[(int)pass_event_check]);
      	  // chdir("..");
      	  // ed.Clear();
      	} // end if: pass event check for first 100 events


      	// clean cache
      	sigVec.clear();
      	trigVec.clear();
      } // end if: got new event

    } // end if: dataword is a header with event number information
    else if (header_type == Signal::RISING || header_type == Signal::FALLING) {
      sig = Signal(word, currEventID);

      if (!first_signal_flag[sig.TDC()][sig.Channel()]) {
      	sig.SetIsFirstSignal(kTRUE);
      	first_signal_flag[sig.TDC()][sig.Channel()] = kTRUE;
      }

      if (sig.TDC() == geo.TRIGGER_MEZZ && sig.Channel() == geo.TRIGGER_CH) {
      	trigVec.push_back(sig);
      	if (header_type == Signal::RISING) total_triggers++;
      }
      else if (sig.TDC() != geo.TRIGGER_MEZZ) {
      	sigVec.push_back(sig);
      	if (header_type == Signal::RISING) total_signals++;
      }

    } // end else if: dataword is a rising or falling edge 
  } // end while: data in flow

  cout << "Decoding completed !" << endl;

  /* plot the time spectrum for leading and trailing edge */
  cout << "Making plots... " << endl;


  cout << endl << "Plotting total leading edge spectrum... " << endl;
  TCanvas *p_leading_canvas = new TCanvas("leading", "leading", 0, 0, 800,
                                          450);
  p_leading_canvas->cd();
  p_leading_time->GetXaxis()->SetTitle("time/ns");
  p_leading_time->Draw();

  cout << endl << "Plotting total trailing edge spectrum... " << endl;
  TCanvas *p_trailing_canvas = new TCanvas("trailing", "trailing", 0, 450,
                                           800, 450);
  p_trailing_canvas->cd();
  p_trailing_time->GetXaxis()->SetTitle("time/ns");
  p_trailing_time->Draw();

  cout << endl << "Plotting hits distribution... " << endl;
  TCanvas *p_hits_canvas[Geometry::MAX_TUBE_LAYER];
  TCanvas *p_hits_together_canvas = new TCanvas("layer_distribution", "layer_distribution");
  char canvas_name[256];
  for (Int_t layer_id = Geometry::MAX_TUBE_LAYER - 1; layer_id != -1; layer_id--) {
    sprintf(canvas_name, "layer_%d_hits_distribution", layer_id);
    //p_hits_canvas[layer_id] = new TCanvas(canvas_name, canvas_name);
    //p_hits_canvas[layer_id]->cd();
    //p_hits_distribution[layer_id]->Draw();
    p_hits_together_canvas->cd();
    p_hits_distribution[layer_id]->SetLineColor(layer_id + 1);
    p_hits_distribution[layer_id]->Draw("same");
  }

  p_hits_together_canvas->BuildLegend(0.75, 0.75, 1, 1);
  p_hits_together_canvas->SaveAs("layer_distributions.png");



  // TDirectory* metaPlots = p_output_rootfile->mkdir("composite");
  // ed.DrawTubeHist(geo, hitByLC,     metaPlots);
  // ed.Clear();
  // cout << "1" << endl;
  // ed.DrawTubeHist(geo, goodHitByLC, metaPlots);
  // ed.Clear();
  // cout << "2" << endl;
  // ed.DrawTubeHist(geo, badHitByLC,  metaPlots);
  // ed.Clear();
  // cout << "3" << endl;
  // p_output_rootfile->cd();
  // eTree->Write();
  // hitByLC->Write();
  // badHitByLC->Write();
  // goodHitByLC->Write();
  // p_output_rootfile->Write();

  // export data to output directory
  #ifdef SAVE_TRACKS_OUT_OF_ROOT
  TCanvas *p_output_canvas = new TCanvas("", "");
  p_output_canvas->cd();
  for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    if (geo.IsActiveTDC(tdc_id)) {
      sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id, Geometry::MAX_TDC);
      chdir(directory_name);

      p_tdc_tdc_time_corrected[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d_tdc_time_spectrum_corrected.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);
      
      p_tdc_adc_time[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d__adc_time_spectrum.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);

      for (Int_t channel_id = 0; channel_id != Geometry::MAX_TDC_CHANNEL;channel_id++) {
      	if (geo.IsActiveTDCChannel(tdc_id, channel_id)||(tdc_id==geo.TRIGGER_MEZZ)) {

      	  p_tdc_time_corrected[tdc_id][channel_id]->Draw();
      	  sprintf(output_filename,"tdc_%d__channel_%d__tdc_time_spectrum_corrected.png",tdc_id, channel_id);
      	  p_output_canvas->SaveAs(output_filename);
      	  
      	  p_tdc_time[tdc_id][channel_id]->Draw();
          sprintf(output_filename,"tdc_%d__channel_%d__tdc_time_spectrum.png",tdc_id, channel_id);
          p_output_canvas->SaveAs(output_filename);

      	  p_adc_time[tdc_id][channel_id]->Draw();
      	  sprintf(output_filename, "tdc_%d__channel_%d__adc_time_spectrum.png",tdc_id, channel_id);
      	  p_output_canvas->SaveAs(output_filename);
      	}
      }
      chdir("..");
    } // end if: active TDC
  }
  #endif

  int nEntries = eTree->GetEntries();
  
  delete p_output_rootfile;
  //delete track_base;
  delete p_leading_canvas;
  delete p_trailing_canvas;
  

  cout << endl;
  cout << "Total Triggers: " << total_triggers << endl;
  cout << "Pass  Triggers: " << total_triggers_pass << endl;
  cout << endl;
  cout << "Total Events:   " << total_events   << endl;
  cout << "Pass  Events:   " << total_events_pass << endl;
  cout << endl;
  cout << "Total Signals:  " << total_signals  << endl;
  cout << "Pass  Signals:  " << total_signals_pass << endl;
  cout << endl;
  cout << "N tree entries: " << nEntries << endl;


  
  //gROOT->SetBatch(kFALSE);
  return 0;
}
    
