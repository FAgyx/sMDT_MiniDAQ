
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
#include "TString.h"

// Muon Reconstruction includes
#include "MuonReco/RecoUtility.h"
#include "MuonReco/Signal.h"
#include "MuonReco/EventID.h"
#include "MuonReco/Event.h"
#include "MuonReco/EventDisplay.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Hit.h"
#include "MuonReco/Cluster.h"
#include "MuonReco/TimeCorrection.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/IOUtility.h"

//#define DECODER_DEBUG // comment this line when debugging information is not needed
//#define SET_MAXWORDS // comment this line if you want to decode the whole data words
#define SAVE_TRACKS_OUT_OF_ROOT // comment this line if you don't need to save plots out of rootfile 

#define TOTAL_BIN_QUANTITY 1024 // set bin quantity of the plot 

using namespace std;
using namespace MuonReco;

int main(int argc, char* argv[]) {

  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");

  TString filename = cp.items("General").getStr("RawFileName");
  
  gROOT->SetBatch(kTRUE); // set to batch mode to inprove the speed

  
  // open input file
  ifstream data_in_flow;
  data_in_flow.open(IOUtility::getRawInputFilePath(filename).Data());

  data_in_flow.seekg(0, data_in_flow.end);
  unsigned int data_in_flow_length = data_in_flow.tellg(); // get file size
  data_in_flow.seekg(0, data_in_flow.beg);

  // create output file
  TFile *p_output_rootfile = new TFile(IOUtility::getDecodedOutputFilePath(runN), "RECREATE");


  // prepare file structure for event display
  TDirectory *event_track[2];
  char track_group_name[128];
  char output_filename[200];

  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry"));

  for (int iTDC = 0; iTDC < Geometry::MAX_TDC; iTDC++) {
    int hitL, hitC;
    geo.GetHitLayerColumn(iTDC, 0, &hitL, &hitC);
    std::cout << "TDC " << iTDC << " channel " << 0 << " mapped to " << " l=" << hitL << " c=" << hitC << std::endl;
  }
  TimeCorrection tc = TimeCorrection(cp);
  tc.Read();
  static EventDisplay   ed = EventDisplay();
  RecoUtility ru = RecoUtility(cp.items("RecoUtility"));

  TH1F *p_leading_time = new TH1F("leading time spectrum", "leading time spectrum", 100, 0, 1000);
  TH1F *p_trailing_time = new TH1F("trailing time spectrum", "trailing time spectrum", 100, 0, 1000);
  TH1F *p_hits_distribution[Geometry::MAX_TUBE_LAYER];
  char histogram_name[256];
  for (Int_t layer_id = 0; layer_id != Geometry::MAX_TUBE_LAYER; layer_id++) {
    sprintf(histogram_name, "layer_%d_hits_distribution", layer_id);
    p_hits_distribution[layer_id] = new TH1F(histogram_name, histogram_name, Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5);
  }

  char directory_name[256];
  TDirectory *tdc_directory     [Geometry::MAX_TDC];

  TH1F *p_tdc_tdc_time_raw      [Geometry::MAX_TDC];
  TH1F *p_tdc_tdc_time_corrected[Geometry::MAX_TDC];
  TH1F *p_tdc_tdc_time_selected [Geometry::MAX_TDC];
  TH1F *p_tdc_adc_time          [Geometry::MAX_TDC];
  TH1F *p_tdc_adc_time_selected [Geometry::MAX_TDC];

  TH1F *p_tdc_time_raw          [Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_tdc_time_corrected    [Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_tdc_time_selected     [Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_adc_time              [Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  TH1F *p_adc_time_selected     [Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];

  TString h_name;

  for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id,Geometry::MAX_TDC);
    tdc_directory[tdc_id] = p_output_rootfile->mkdir(directory_name);
    tdc_directory[tdc_id]->cd();

    h_name.Form("tdc_%d_tdc_time_spectrum_raw", tdc_id);
    p_tdc_tdc_time_raw[tdc_id] = new TH1F(h_name, h_name+";Time [ns];Entries/0.78125 ns",TOTAL_BIN_QUANTITY, -400, 400);

    h_name.Form("tdc_%d_tdc_time_spectrum_corrected", tdc_id);
    p_tdc_tdc_time_corrected[tdc_id] = new TH1F(h_name, h_name+";Time [ns];Entries/0.78125 ns",TOTAL_BIN_QUANTITY, -400, 400);

    h_name.Form("tdc_%d_tdc_time_spectrum_selected", tdc_id);
    p_tdc_tdc_time_selected[tdc_id] = new TH1F(h_name, h_name+";Time [ns];Entries/0.78125 ns",TOTAL_BIN_QUANTITY, -400, 400);
    
    h_name.Form("tdc_%d_adc_time_spectrum", tdc_id);
    p_tdc_adc_time[tdc_id] = new TH1F(h_name, h_name+";Wilkinson ADC [ns];Entries/0.78125 ns", TOTAL_BIN_QUANTITY / 2, 0, 400);

    h_name.Form("tdc_%d_adc_time_spectrum_selected", tdc_id);
    p_tdc_adc_time_selected[tdc_id] = new TH1F(h_name, h_name+";Wilkinson ADC [ns];Entries/1.5625 ns", TOTAL_BIN_QUANTITY / 2, 0, 400);
    
    for (Int_t channel_id = 0; channel_id != Geometry::MAX_TDC_CHANNEL; channel_id++) {
      h_name.Form("tdc_%d_channel_%d_tdc_time_spectrum_raw", tdc_id, channel_id);
      p_tdc_time_raw[tdc_id][channel_id] = new TH1F(h_name,h_name+";Time [ns];Entries/0.78125 ns", TOTAL_BIN_QUANTITY,-400, 400);

      h_name.Form("tdc_%d_channel_%d_tdc_time_spectrum_corrected", tdc_id, channel_id);
      p_tdc_time_corrected[tdc_id][channel_id] = new TH1F(h_name,h_name+";Time [ns];Entries/0.78125 ns", TOTAL_BIN_QUANTITY,-400, 400);

      h_name.Form("tdc_%d_channel_%d_tdc_time_spectrum_selected", tdc_id, channel_id);
      p_tdc_time_selected[tdc_id][channel_id] = new TH1F(h_name,h_name+";Time [ns];Entries/0.78125 ns", TOTAL_BIN_QUANTITY,-400, 400);
      
      h_name.Form("tdc_%d_channel_%d_adc_time_spectrum", tdc_id, channel_id);
      p_adc_time[tdc_id][channel_id] = new TH1F(h_name, h_name+";Wilkinson ADC [ns];Entries/0.78125 ns",TOTAL_BIN_QUANTITY / 2, 0, 400);

      h_name.Form("tdc_%d_channel_%d_adc_time_spectrum_selected", tdc_id, channel_id);
      p_adc_time_selected[tdc_id][channel_id] = new TH1F(h_name, h_name + ";Wilkinson ADC [ns];Entries/0.78125 ns",TOTAL_BIN_QUANTITY / 2, 0, 400);
      
    }
  } // end for: all TDC
  p_output_rootfile->cd();

  TH1I* recoStatus = new TH1I("recoStatus", "Status flag from reconstruction",
			      10,0,9);
  
  TH2D* hitByLC = new TH2D("hitByLC", "All hits on tubes", 
			   Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
			   Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);
  hitByLC->SetStats(0);
  TH2D* hitByLC_nonoise = new TH2D("hitByLC_nonoise", "All hits on tubes wth ADC > 105",
				   Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
				   Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);
  hitByLC_nonoise->SetStats(0);

  TH2D* badHitByLC = new TH2D("badHitByLC", "Hits on tubes with ADC < 105",
			      Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
			      Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);
  badHitByLC->SetStats(0);
  TH2D* goodHitByLC = new TH2D("goodHitByLC", "Hits on tubes that passed clustering", 
			       Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
			       Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);
  goodHitByLC->SetStats(0);

  int hitL, hitC;
  int status = 0;
  Bool_t first_signal_flag[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
  bool pass_event_check;
  unsigned int word;
  unsigned int header_type;
  unsigned int bunch_id = 0;
  EventID currEventID;
  EventID prevEventID = EventID(0x00000000);
  vector<Signal> trigVec = vector<Signal>();
  vector<Signal>  sigVec = vector<Signal>();
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
  unsigned long plotted_pass = 0;
  unsigned long plotted_fail = 0;
  int nhits = 0;

  unsigned long maxEventCount = cp.items("General").getInt("MaxEventCount", 1e100, 0);
  unsigned long maxPassEvents = cp.items("General").getInt("MaxPassEvents", 1e100, 0);

  int nloop = 0;
  while (data_in_flow.read((char *) &word, sizeof(word))   && nloop < maxEventCount && 
	 total_events_pass < maxPassEvents) {
    nloop++;
    header = (word & 0x7fffffff) >> 28; // get the four bits header of this word
    header_type = static_cast<unsigned int>((header.to_ulong()));
    
    if (header_type == Signal::GROUP_HEADER || header_type == Signal::TDC_HEADER || header_type == Signal::TDC_TRAILER) {
      currEventID = EventID((word & 0xfffff000) + bunch_id);     
      if ((word & 0x00fff000) == 0x00fff000) bunch_id++;

      // analyze data if we reached a header for a new event 
      if (currEventID.ID() != prevEventID.ID()) {
	prevEventID = currEventID;
	if (total_events % event_print == 0) {
	  std::cout << "Processing Event " << total_events << std::endl;
	  if (TMath::Floor(TMath::Log10(total_events)) > TMath::Floor(TMath::Log10(event_print))) event_print*=10;
	}

	
	total_events++;
        event = Event(trigVec, sigVec, currEventID);
	nhits = ru.DoHitFinding(&event,   &tc, geo);
	ru.DoHitClustering(&event);
	pass_event_check = kTRUE;
	pass_event_check = ru.CheckEvent(event, &status);
	event.SetPassCheck(pass_event_check);
	event.CheckClusterTime();
	recoStatus->Fill(status);

	for (Hit h : event.WireHits()) {
	  p_tdc_tdc_time_raw      [h.TDC()]->Fill(h.DriftTime());
	  p_tdc_tdc_time_corrected[h.TDC()]->Fill(h.CorrTime());
	  p_tdc_adc_time          [h.TDC()]->Fill(h.ADCTime()); 

	  p_tdc_time_raw      [h.TDC()][h.Channel()]->Fill(h.DriftTime());
	  p_tdc_time_corrected[h.TDC()][h.Channel()]->Fill(h.CorrTime());	  
	  p_adc_time          [h.TDC()][h.Channel()]->Fill(h.ADCTime());

	  geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);

	  // fill heatmap
	  hitByLC->Fill(hitC, hitL);
	  if (h.ADCTime() < 105)
	    badHitByLC->Fill(hitC, hitL);
	  else 
	    hitByLC_nonoise->Fill(hitC, hitL);
	}

	if (pass_event_check) {	
	  eTree->Fill();

	  for (Cluster c : event.Clusters()) {
	    for (Hit h : c.Hits()) {

	      p_tdc_tdc_time_selected[h.TDC()]->Fill(h.CorrTime());
	      p_tdc_adc_time_selected[h.TDC()]->Fill(h.ADCTime()); 

	      p_tdc_time_selected[h.TDC()][h.Channel()]->Fill(h.CorrTime());
	      p_adc_time_selected[h.TDC()][h.Channel()]->Fill(h.ADCTime());

	      geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);

	      goodHitByLC->Fill(hitC, hitL);
	      p_hits_distribution[hitL]->Fill(hitC);
	    }
	  }

	  for (Hit h : event.TriggerHits()) {
	    p_tdc_time_raw      [h.TDC()][h.Channel()]->Fill(h.DriftTime());
	    p_adc_time          [h.TDC()][h.Channel()]->Fill(h.ADCTime());
	  }
	}

	
	for (Int_t i = 0; i != Geometry::MAX_TDC; i++) {
	  for (Int_t j = 0; j != Geometry::MAX_TDC_CHANNEL; j++) {
	    first_signal_flag[i][j] = kFALSE;
	  }
	}

	if (pass_event_check) {
	  total_signals_pass += sigVec.size();
	  total_events_pass++;
	  total_triggers_pass++;
	}
	else {
	  total_events_fail++;
	}

	// plot the event
	// plot the first 100 events meeting and not meeting the pass event check criteria
	if ((pass_event_check && plotted_pass < 100) || (!pass_event_check && plotted_fail < 100 && nhits > 0)) { 

	  if (pass_event_check) {
	    sprintf(track_group_name, "events_passing");
	    plotted_pass++;
	  }
	  else {
	    sprintf(track_group_name, "events_failing");
	    plotted_fail++;
	  }
	  
	  if ((pass_event_check && plotted_pass == 1) || (!pass_event_check&& plotted_fail == 1) )
	    event_track[(int)pass_event_check] = p_output_rootfile->mkdir(track_group_name);

	  
	  event_track[(int)pass_event_check]->cd();
	  ed.SetOutputDir(IOUtility::join(IOUtility::getRunOutputDir(runN), track_group_name, kTRUE));
	  ed.DrawEvent(event, geo, event_track[(int)pass_event_check]);
	  ed.Clear();
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
      else if (sig.TDC() != geo.TRIGGER_MEZZ && geo.IsActiveTDC(sig.TDC())) {
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
  p_leading_time->GetXaxis()->SetTitle("Time [ns]");
  p_leading_time->Draw();

  cout << endl << "Plotting total trailing edge spectrum... " << endl;
  TCanvas *p_trailing_canvas = new TCanvas("trailing", "trailing", 0, 450,
                                           800, 450);
  p_trailing_canvas->cd();
  p_trailing_time->GetXaxis()->SetTitle("Time [ns]");
  p_trailing_time->Draw();

  cout << endl << "Plotting hits distribution... " << endl;
  TCanvas *p_hits_together_canvas = new TCanvas("layer_distribution", "layer_distribution");
  char canvas_name[256];
  for (Int_t layer_id = Geometry::MAX_TUBE_LAYER - 1; layer_id != -1; layer_id--) {
    sprintf(canvas_name, "layer_%d_hits_distribution", layer_id);
    //p_hits_distribution[layer_id]->Draw();
    p_hits_together_canvas->cd();
    p_hits_distribution[layer_id]->SetLineColor(layer_id + 1);
    p_hits_distribution[layer_id]->Draw("same");
  }

  p_hits_together_canvas->BuildLegend(0.75, 0.75, 1, 1);
  p_hits_together_canvas->SaveAs(IOUtility::join(IOUtility::getRunOutputDir(runN), "layer_distributions.png"));



  TDirectory* metaPlots = p_output_rootfile->mkdir("composite");
  ed.SetOutputDir(IOUtility::getRunOutputDir(runN));
  ed.DrawTubeHist(geo, hitByLC,         metaPlots);
  ed.Clear();
  ed.DrawTubeHist(geo, hitByLC_nonoise, metaPlots);
  ed.Clear();
  ed.DrawTubeHist(geo, goodHitByLC,     metaPlots);
  ed.Clear();
  ed.DrawTubeHist(geo, badHitByLC,      metaPlots);
  ed.Clear();

  p_output_rootfile->cd();
  eTree->Write();
  hitByLC->Write();
  badHitByLC->Write();
  goodHitByLC->Write();
  recoStatus->Write();
  p_output_rootfile->Write();

  TH2D* hitByLCLog = (TH2D*)hitByLC->Clone("hitByLCLog");

  for (int bx = 1; bx <= hitByLCLog->GetNbinsX(); ++bx) {
    for (int by = 1; by <= hitByLCLog->GetNbinsY(); ++by) {
      if (hitByLCLog->GetBinContent(bx, by))
	hitByLCLog->SetBinContent(bx, by, TMath::Log(hitByLCLog->GetBinContent(bx, by)));
    }
  }
  ed.DrawTubeHist(geo, hitByLCLog, metaPlots);
  ed.Clear();

  // export data to output directory
  #ifdef SAVE_TRACKS_OUT_OF_ROOT
  TCanvas *p_output_canvas = new TCanvas("", "");
  TString dir_name;
  p_output_canvas->cd();
  for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    if (geo.IsActiveTDC(tdc_id)) {
      tdc_directory[tdc_id]->cd();
      dir_name = TString::Format("TDC_%02d_of_%02d_Time_Spectrum", tdc_id, Geometry::MAX_TDC);
      
      p_tdc_tdc_time_raw[tdc_id]->Draw();
      p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d_tdc_time_spectrum_raw.png", tdc_id)));

      p_tdc_tdc_time_corrected[tdc_id]->Draw();
      p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d_tdc_time_spectrum_corrected.png", tdc_id)));
      
      p_tdc_tdc_time_selected[tdc_id]->Draw();
      p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d_tdc_time_spectrum_selected.png", tdc_id)));

      p_tdc_adc_time[tdc_id]->Draw();
      p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__adc_time_spectrum.png", tdc_id)));

      p_tdc_adc_time_selected[tdc_id]->Draw();
      p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__adc_time_spectrum_selected.png", tdc_id)));

      for (Int_t channel_id = 0; channel_id != Geometry::MAX_TDC_CHANNEL;channel_id++) {
	if (geo.IsActiveTDCChannel(tdc_id, channel_id)) {

	  p_tdc_time_raw[tdc_id][channel_id]->Draw();
	  p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__channel_%d__tdc_time_spectrum_raw.png", tdc_id, channel_id)));

	  p_tdc_time_corrected[tdc_id][channel_id]->Draw();
	  p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__channel_%d__tdc_time_spectrum_corrected.png", tdc_id, channel_id)));
	  
	  p_tdc_time_selected[tdc_id][channel_id]->Draw();
	  p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__channel_%d__tdc_time_spectrum_selected.png", tdc_id, channel_id)));

	  p_adc_time[tdc_id][channel_id]->Draw();
	  p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__channel_%d__adc_time_spectrum.png", tdc_id, channel_id)));

	  p_adc_time_selected[tdc_id][channel_id]->Draw();
	  p_output_canvas->SaveAs(IOUtility::join(IOUtility::join(IOUtility::getRunOutputDir(runN), dir_name, kTRUE), TString::Format("tdc_%d__channel_%d__adc_time_spectrum_selected.png", tdc_id, channel_id)));
	}
      }
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


  
  gROOT->SetBatch(kFALSE);
  return 0;
}
    
