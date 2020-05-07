
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

#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"

#include "src/depreciated/CheckEventZhe.cpp"
#include "src/depreciated/GetHitInfo.h"
#include "src/depreciated/GetHitLayerColumn.h"
#include "src/depreciated/MdtSlewCorFuncHardcoded.cpp"
#include "src/Signal.cpp"
#include "src/EventID.cpp"

//#define DECODER_DEBUG // comment this line when debugging information is not needed
//#define SET_MAXWORDS // comment this line if you want to decode the whole data words
#define SAVE_TRACKS_OUT_OF_ROOT // comment this line if you don't need to save plots out of rootfile 

#define TRIGGER_MEZZ 1 // set mezz's tdc channel for trigger input
#define TRIGGER_CH 23 // set trigger channel in that mezz card
#define TOTAL_BIN_QUANTITY 1024 // set bin quantity of the plot 


const Int_t MAX_TDC_QUANTITY = 18;
const Int_t MAX_TDC_CHANNEL_QUANTITY = 24;
const Int_t MAX_TUBE_LAYER = 8;
const Int_t MAX_TUBE_COLUMN = 54;
const Long_t MAX_WORD_QUANTITY = 1621748;

using namespace std;

double calibrated_p(const double &adc);
double ADC_Correction(double adc, vector<Double_t> v_LUT);

int DecodeRawData_Zhe() {
  gROOT->SetBatch(kTRUE); // set to batch mode to inprove the speed

  /* open data file */
  char filename[256];
  char input_filename[256];
  cout << "Please enter the input file name to analyse:" << endl;
  cin >> filename; // input name of the file to be decoded
  strcpy(input_filename, "raw/");
  strcat(input_filename, filename);
  ifstream data_in_flow;
  data_in_flow.open(input_filename);
  while (!data_in_flow.is_open()) {
    cout << "Unable to open input file!" << endl;
    cout << "Please enter the input file name to analyse, enter 'NULL' to exit:" << endl;
    cin >> filename;
    if (strcmp(filename, "NULL") == 0) {
      cout << "NULL input file, exiting program." << endl;
      return 1;
    }
    strcpy(input_filename, "raw/");
    strcat(input_filename, filename);
    data_in_flow.open(input_filename);
  }
  data_in_flow.seekg(0, data_in_flow.end);
  unsigned int data_in_flow_length = data_in_flow.tellg(); // get file size
  data_in_flow.seekg(0, data_in_flow.beg);

  /* prepare output file */
  system("mkdir Output");
  chdir("Output");
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


  /* prepare output Ntuple */
  char output_root_filename[200];
  strcpy(output_root_filename, output_filename);
  strcat(output_root_filename, ".root");
  TFile *p_output_rootfile = new TFile(output_root_filename, "RECREATE");
  const char *ntuple_varlist = "type:tdc:data0:data1:data2:signal_flag";
    // for header_type = 2, data0 = 0, data1 = EventID, data2 = BunchID
    // for heaser_type = 4/5, data0 = channel, data1 = coarse, data2 = fine
  TNtuple *p_HPTDC_data = new TNtuple("HPTDCData", "HPTDCData", ntuple_varlist);

  /* prepare output histogram */
  TH1F *p_leading_time = new TH1F("leading time spectrum", "leading time spectrum", 100, 0, 1000);
  TH1F *p_trailing_time = new TH1F("trailing time spectrum",
                                   "trailing time spectrum", 100, 0, 1000);
  TH1F *p_hits_distribution[MAX_TUBE_LAYER];
  char histogram_name[256];
  for (Int_t layer_id = 0; layer_id != MAX_TUBE_LAYER; layer_id++) {
    sprintf(histogram_name, "layer_%d_hits_distribution", layer_id);
    p_hits_distribution[layer_id] = new TH1F(histogram_name, histogram_name, 20,
                                             20, 40);
  }
  TH1F *p_tdc_time[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  TH1F *p_tdc_time_original[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  TH1F *p_tdc_time_corrected[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  TH1F *p_tdc_time_selected[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  TH1F *p_adc_time[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  TH1F *p_tdc_tdc_time[MAX_TDC_QUANTITY];
  TH1F *p_tdc_tdc_time_original[MAX_TDC_QUANTITY];
  TH1F *p_tdc_tdc_time_corrected[MAX_TDC_QUANTITY];
  TH1F *p_tdc_tdc_time_selected[MAX_TDC_QUANTITY];
  TH1F *p_tdc_adc_time[MAX_TDC_QUANTITY];
  TH1F *p_tdc_channel[MAX_TDC_QUANTITY];
  TH2F *p_adc_vs_tdc[MAX_TDC_QUANTITY];
  TDirectory *tdc_directory[MAX_TDC_QUANTITY];
  char directory_name[256];
  for (Int_t tdc_id = 0; tdc_id != MAX_TDC_QUANTITY; tdc_id++) {
    /* create output directories */
    sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id,
            MAX_TDC_QUANTITY);
    tdc_directory[tdc_id] = p_output_rootfile->mkdir(directory_name);
    tdc_directory[tdc_id]->cd();
    if (mkdir(directory_name, 0777) == -1) {
      cerr << strerror(errno) << endl;
    }

    /* histogram settings */
    sprintf(histogram_name, "tdc_%d_tdc_time_spectrum", tdc_id);
    p_tdc_tdc_time[tdc_id] = new TH1F(histogram_name, histogram_name, TOTAL_BIN_QUANTITY, -400, 400);
    p_tdc_tdc_time[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_tdc_time[tdc_id]->GetYaxis()->SetTitle("entries");
    sprintf(histogram_name, "tdc_%d_tdc_time_spectrum_original", tdc_id);
    p_tdc_tdc_time_original[tdc_id] = new TH1F(histogram_name, histogram_name,
                                               TOTAL_BIN_QUANTITY, 0, 800);
    p_tdc_tdc_time_original[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_tdc_time_original[tdc_id]->GetYaxis()->SetTitle("entries");
    sprintf(histogram_name, "tdc_%d_tdc_time_spectrum_corrected", tdc_id);
    p_tdc_tdc_time_corrected[tdc_id] = new TH1F(histogram_name, histogram_name,
                                               TOTAL_BIN_QUANTITY, -400, 400);
    p_tdc_tdc_time_corrected[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_tdc_time_corrected[tdc_id]->GetYaxis()->SetTitle("entries");
    sprintf(histogram_name, "tdc_%d_tdc_time_spectrum_selected", tdc_id);
    p_tdc_tdc_time_selected[tdc_id] = new TH1F(histogram_name, histogram_name,
                                               TOTAL_BIN_QUANTITY, -400, 400);
    p_tdc_tdc_time_selected[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_tdc_time_selected[tdc_id]->GetYaxis()->SetTitle("entries");
    sprintf(histogram_name, "tdc_%d_adc_time_spectrum", tdc_id);
    p_tdc_adc_time[tdc_id] = new TH1F(histogram_name, histogram_name, TOTAL_BIN_QUANTITY / 2, 0, 400);
    p_tdc_adc_time[tdc_id]->GetXaxis()->SetTitle("time/ns");
    p_tdc_adc_time[tdc_id]->GetYaxis()->SetTitle("entries");
    sprintf(histogram_name, "tdc_%d_channel_distribution", tdc_id);
    p_tdc_channel[tdc_id] = new TH1F(histogram_name, histogram_name, 24, 0, 24);
    p_tdc_channel[tdc_id]->GetXaxis()->SetTitle("channel");
    p_tdc_channel[tdc_id]->GetYaxis()->SetTitle("entries");
    sprintf(histogram_name, "tdc_%d_adc_vs_tdc", tdc_id);
    p_adc_vs_tdc[tdc_id] = new TH2F(histogram_name, histogram_name, TOTAL_BIN_QUANTITY / 8, -200, 200, TOTAL_BIN_QUANTITY / 8, 0, 400);
    p_adc_vs_tdc[tdc_id]->GetXaxis()->SetTitle("tdc/ns");
    p_adc_vs_tdc[tdc_id]->GetYaxis()->SetTitle("adc/ns");

    for (Int_t channel_id = 0; channel_id != MAX_TDC_CHANNEL_QUANTITY;
         channel_id++) {
      sprintf(histogram_name, "tdc_%d_channel_%d_tdc_time_spectrum", tdc_id,
              channel_id);
      p_tdc_time[tdc_id][channel_id] = new TH1F(histogram_name, histogram_name,
                                                TOTAL_BIN_QUANTITY, -400, 400);
      sprintf(histogram_name, "tdc_%d_channel_%d_tdc_time_spectrum_original",
              tdc_id, channel_id);
      p_tdc_time_original[tdc_id][channel_id] = new TH1F(histogram_name,
                                                         histogram_name, TOTAL_BIN_QUANTITY,
                                                         0, 800);
      sprintf(histogram_name, "tdc_%d_channel_%d_tdc_time_spectrum_corrected",
              tdc_id, channel_id);
      p_tdc_time_corrected[tdc_id][channel_id] = new TH1F(histogram_name,
                                                         histogram_name, TOTAL_BIN_QUANTITY,
                                                        -400, 400);
      sprintf(histogram_name, "tdc_%d_channel_%d_tdc_time_spectrum_selected",
              tdc_id, channel_id);
      p_tdc_time_selected[tdc_id][channel_id] = new TH1F(histogram_name,
                                                         histogram_name, TOTAL_BIN_QUANTITY,
                                                        -400, 400);
      sprintf(histogram_name, "tdc_%d_channel_%d_adc_time_spectrum", tdc_id,
              channel_id);
      p_adc_time[tdc_id][channel_id] = new TH1F(histogram_name, histogram_name,
                                                TOTAL_BIN_QUANTITY / 2, 0, 400);
    }
  }


  /* prepare base of output track display */
  const Double_t layer_distance = 13.0769836;
  const Double_t column_distance = 15.1;
  const Double_t radius = 7.5;
  
  TDirectory *event_track[10];
  event_track[0] = p_output_rootfile->mkdir("event_tracks_record_example0");
  if (mkdir("event_tracks_record_example0", 0777) == -1) {
    cerr << strerror(errno) << endl;
  }
  char track_group_name[128];
  strcpy(track_group_name, "event_tracks_record_example0");
  TCanvas *track_base = new TCanvas("track base", "track base", 1200, 480);
  track_base->cd();
  Double_t center_x, center_y;
  Double_t track_corner_x[2] = {0, 800};
  Double_t track_corner_y[2] = {0, 320};
  TGraph * track_baseline = new TGraph(2, track_corner_x, track_corner_y);
  track_baseline->SetTitle("event");
  track_baseline->Draw("AP");
  
  TEllipse *tube_model[54][8];
  TEllipse *hit_model[512];
  for (Int_t layer_id = 0; layer_id != 4; layer_id++) {
    for (Int_t column_id = 0; column_id != 54; column_id++) {
      center_x = 7.5 + column_id * column_distance + ((layer_id + 1) % 2) *
	column_distance / 2.0;
      center_y = 7.5 + layer_id * layer_distance;
      tube_model[layer_id][column_id] = new TEllipse(center_x, center_y, radius, radius);

      if ((column_id / 6) % 2 == 0) {
	tube_model[layer_id][column_id]->SetFillColor(kGray);
      }
      tube_model[layer_id][column_id]->Draw();
    }
  }
  for (Int_t layer_id = 4; layer_id != 8; layer_id++) {
    for (Int_t column_id = 0; column_id != 54; column_id++) {
      center_x = 7.5 + column_id * column_distance + ((layer_id + 1) % 2) * column_distance / 2.0;
      center_y = 7.5 + (layer_id - 4) * layer_distance + 224.231;
      tube_model[layer_id][column_id] = new TEllipse(center_x, center_y, radius, radius);

      if ((column_id / 6) % 2 == 0) {
	tube_model[layer_id][column_id]->SetFillColor(kGray);
      }
      tube_model[layer_id][column_id]->Draw();
    }
  }


  // kevin: what is this adc correction table????
  
  /* prepare adc correction table using MDT correction function*/
  vector<Double_t> v_LUT;
  for (Int_t i = 0; i != 400; i++) {
    v_LUT.push_back(109./calibrated_p(i));
  }
  
  /* execute selected operation and save data to output file */
  unsigned int word, id, header_type;
  unsigned int trigger_cache_length = 0;
  unsigned int trigger_cache[256][5];
  unsigned int signal_cache_length = 0;
  unsigned int signal_cache[256][5];
  unsigned int new_data_line[5];
  bool signal_flag[256];
  bool got_new_event = kFALSE;
  unsigned long total_loop = 0;
  unsigned long total_tracks = 0;
  int current_track_group = 0;
  TCanvas *hit_display_cache;
  unsigned int event_count = 0;
  unsigned long current_event_id = 0;
  unsigned long event_id_group = 0;
  bool new_trigger_flag = kFALSE;
  bool new_signal_flag = kFALSE;
  bool plot_flag = kTRUE;
  cout << "Processing..." << endl;
  
  unsigned long total_triggers = 0;
  unsigned long total_events   = 0;
  unsigned long total_triggers_pass = 0;
  unsigned long total_events_pass = 0;
  unsigned long total_signals = 0;
  unsigned long total_signals_pass = 0;

  while (data_in_flow.read((char *) &word, sizeof(word))) {
    bitset<32> data_cache;
    bitset<4> header; // for header type
    data_cache = word;
    header = word >> 28; // get the four bits header of this word
    header_type = static_cast<unsigned int>((header.to_ulong()));
    
    unsigned int _type, _event_id, _bunch_id,  _tdc, _channel, _width, _coarse, _fine ,_leading_time;
    Double_t time_in_ns;
    bitset<4> type;
    bitset<4> tdc;
    bitset<12> event_id;
    bitset<12> bunch_id;
    bitset<5> channel;
    bitset<12> coarse;
    bitset<7> fine;

    unsigned int leading_mean_coarse = 0;
    unsigned int trailing_mean_coarse = 0;
    bool pass_event_check = kFALSE;
    unsigned int trigger_data_line[5];
    bool got_trigger = kFALSE;
    if (header_type == 2 || header_type == 3 || header_type == 0) {
      /*
       * Word is a group header, TDC header, TDC trailer
       * See HPTDC Documentation, pages 21-22
       */
      /* collect new data */
      type = word >> 28;
      tdc = word >> 24;
      event_id = word >> 12;
      bunch_id = word;
      _type = static_cast<unsigned int>((type.to_ulong()));
      _tdc = static_cast<unsigned int>((tdc.to_ulong()));
      _event_id = static_cast<unsigned int>((event_id.to_ulong()));
      _bunch_id = static_cast<unsigned int>((bunch_id.to_ulong()));

      /* analysis data if one event is over */
      got_new_event = (current_event_id != _event_id);
      if (got_new_event) {
	total_events++;
        event_count++;
        if (current_event_id == _event_id + 4096 * event_id_group + 4095) {
          event_id_group++;
        }
        current_event_id = _event_id + 4096 * event_id_group;
        leading_mean_coarse = 0;
        trailing_mean_coarse = 0;
        pass_event_check = kFALSE;
        got_trigger = kFALSE;
        Bool_t is_first_signal = kFALSE;
        Bool_t first_signal_flag[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
        for (Int_t i = 0; i != MAX_TDC_QUANTITY; i++) {
          for (Int_t j = 0; j != MAX_TDC_CHANNEL_QUANTITY; j++) {
            first_signal_flag[i][j] = kFALSE;
          }
        }


        /* get trigger id, calculate tdc time & time difference between leading & trailing edge */
        int selected_trigger_id = 256;
        long current_coarse_difference = 2048;
        if (trigger_cache_length != 0) {
          long temp_coarse = 0;
          long total_coarse = 0;
          long temp_mean_coarse = 0;
          int trigger_quantity = 0;
	  pass_event_check = CheckEvent(signal_cache_length, signal_cache, &leading_mean_coarse, &trailing_mean_coarse);

	  // initialize variables on existence of triggers
          for (int trigger_id = 0; trigger_id < trigger_cache_length; trigger_id++) {
            if (trigger_cache[trigger_id][0] == 4) {
              total_coarse += trigger_cache[trigger_id][3];
              trigger_quantity++;
            }
          } // end for: trigger_id
          if (trigger_quantity == 0) {
            temp_mean_coarse = 0;
            got_trigger = kFALSE;
          } 
	  else {
            temp_mean_coarse = total_coarse / trigger_quantity;
            got_trigger = kTRUE;
          }

          for (int trigger_id = 0; trigger_id < trigger_cache_length; trigger_id++) {
            if (trigger_cache[trigger_id][0] == 4) {
              temp_coarse = trigger_cache[trigger_id][3];
              if (abs(temp_coarse - temp_mean_coarse) < current_coarse_difference) {
                current_coarse_difference = abs(temp_coarse - temp_mean_coarse);
                selected_trigger_id = trigger_id;
                // selected_trigger_id = 0;
              }
              double tdc_time = (trigger_cache[trigger_id][3] + trigger_cache[trigger_id][4] / 128.0) * 25.0;

              double tdc_time2;
              Double_t adc_time;
              for (int trigger_id2 = 0; trigger_id2 < trigger_cache_length; trigger_id2++) {
                if (trigger_cache[trigger_id2][0] == 5) {
                  if (trigger_cache[trigger_id][1] == trigger_cache[trigger_id2][1] && trigger_cache[trigger_id][2] == trigger_cache[trigger_id2][2]) {
                    tdc_time2 = (trigger_cache[trigger_id2][3] + trigger_cache[trigger_id2][4] / 128.0) * 25.0;
                    adc_time = tdc_time2 - tdc_time;

                    break;
                  }
                }
              }
	      if (pass_event_check) {
		total_triggers_pass++;
		p_tdc_time[trigger_cache[trigger_id][1]][trigger_cache[trigger_id][2]]->Fill(tdc_time);
		p_tdc_tdc_time[trigger_cache[trigger_id][1]]->Fill(tdc_time);
		p_tdc_channel[trigger_cache[trigger_id][1]]->Fill(trigger_cache[trigger_id][2]);
		p_adc_time[trigger_cache[trigger_id][1]][trigger_cache[trigger_id][2]]->Fill(adc_time);
		p_tdc_adc_time[trigger_cache[trigger_id][1]]->Fill(adc_time);
	      }
            }
          } // end for: trigger_id

          trigger_data_line[0] = trigger_cache[selected_trigger_id][0];
          trigger_data_line[1] = trigger_cache[selected_trigger_id][1];
          trigger_data_line[2] = trigger_cache[selected_trigger_id][2];
          trigger_data_line[3] = trigger_cache[selected_trigger_id][3];
          trigger_data_line[4] = trigger_cache[selected_trigger_id][4];

          /* calculate tdc time using leading edge */
          int hit_layer, hit_column;
          Bool_t pass_all_layer[8];
          for (Int_t i = 0; i != 8; i++) {
            pass_all_layer[i] = kFALSE;
          }
          if (got_trigger) {
            for (int signal_id = 0; signal_id < signal_cache_length;signal_id++) {
	      if (signal_cache[signal_id][0] == 4) {
		double current_signal_time = (signal_cache[signal_id][3] + signal_cache[signal_id][4]/ 128.0 ) * 25.0;
		double current_trigger_time = (trigger_data_line[3] + trigger_data_line[4]/ 128.0 ) * 25.0;
		double current_tdc_time = current_signal_time - current_trigger_time;
		
		if (first_signal_flag[signal_cache[signal_id][1]][signal_cache[signal_id][2]] == kTRUE) {
		  is_first_signal = kFALSE;
		} 
		else {
		  first_signal_flag[signal_cache[signal_id][1]][signal_cache[signal_id][2]] = kTRUE;
		  is_first_signal = kTRUE;
		}
		
		// calculate time difference between leading & trailing edge
		double adc_time = 0;
		bool got_adc = kFALSE;
		for (int signal_id2 = 0; signal_id2 < signal_cache_length;
		     signal_id2++) {
		  if ((signal_cache[signal_id2][0] == 5) &&
		      (signal_cache[signal_id2][1] == signal_cache[signal_id][1]) && (signal_cache[signal_id2][2] == signal_cache[signal_id][2])) {
		    adc_time = (signal_cache[signal_id2][3] + signal_cache[signal_id2][4] / 128.0 ) * 25.0 - (signal_cache[signal_id][3] + signal_cache[signal_id][4] / 128.0 ) * 25.0;
		    
		    p_adc_time[signal_cache[signal_id][1]][signal_cache[signal_id][2]]->Fill(adc_time);
		    p_tdc_adc_time[signal_cache[signal_id][1]]->Fill(adc_time);
		    
		    got_adc = kTRUE;
		  }
		}
		
		if (is_first_signal && got_adc && pass_event_check/*&& adc_time > 40*/) { // can make adc cut here
		  total_signals_pass++;
		  signal_flag[signal_id] = kTRUE;
		  is_first_signal = kFALSE;
		  
		  p_tdc_time[signal_cache[signal_id][1]][signal_cache[signal_id][2]]->Fill(current_tdc_time);
		  p_tdc_time_original[signal_cache[signal_id][1]][signal_cache[signal_id][2]]->Fill(current_signal_time);
		  p_tdc_tdc_time[signal_cache[signal_id][1]]->Fill(current_tdc_time);
		  if (hit_column >= 27 && hit_column <= 32) {
		    p_tdc_tdc_time_selected[signal_cache[signal_id][1]]->Fill(current_tdc_time);
		    p_tdc_time_selected[signal_cache[signal_id][1]][signal_cache[signal_id][2]]->Fill(current_tdc_time);
		  }
		  p_tdc_tdc_time_original[signal_cache[signal_id][1]]->Fill(current_signal_time);
		  p_tdc_channel[signal_cache[signal_id][1]]->Fill(signal_cache[signal_id][2]);
		  
		  double tdc_time_corrected = current_tdc_time - ADC_Correction(adc_time, v_LUT);
		  p_tdc_time_corrected[signal_cache[signal_id][1]][signal_cache[signal_id][2]]->Fill(tdc_time_corrected);
		  p_tdc_tdc_time_corrected[signal_cache[signal_id][1]]->Fill(tdc_time_corrected);
		  
		  p_adc_vs_tdc[signal_cache[signal_id][1]]->Fill(current_tdc_time, adc_time);
		  
		  GetHitLayerColumn(signal_cache[signal_id][1], signal_cache[signal_id][2], &hit_layer, &hit_column);
		  p_hits_distribution[hit_layer]->Fill(hit_column);
                }
                else {
                  signal_flag[signal_id] = kFALSE;
                }
              } // end if: signal is a leading edge measurement
            } // end for signal_id in cache length
          } // end if: got trigger




          /* make plot for event display */
          pass_event_check = CheckEvent(signal_cache_length, signal_cache, &leading_mean_coarse, &trailing_mean_coarse); // select event to display in CheckEvent() function
          if (pass_event_check) {
	    total_events_pass++;
            int temp_track_group;
            unsigned int tdc_id, channel_id;
            double hit_x, hit_y;
            temp_track_group = total_tracks / 100;
            if (current_track_group < temp_track_group) {
              if (temp_track_group < 10) {
                sprintf(track_group_name, "event_tracks_record_example%d", temp_track_group);
                event_track[temp_track_group] = p_output_rootfile->mkdir(track_group_name);

                #ifdef SAVE_TRACKS_OUT_OF_ROOT
                  if (mkdir(track_group_name, 0777) == -1) {
                    cerr << strerror(errno) << endl;
                  }
                #endif
              }
              else {
                plot_flag = kFALSE;
              }
              current_track_group = temp_track_group;
            }
            if (plot_flag) {
              event_track[temp_track_group]->cd();
              track_base->cd();
              for (int signal_id = 0; signal_id < signal_cache_length;
                   signal_id++) {
                if (signal_cache[signal_id][0] == 4) {
                  tdc_id = signal_cache[signal_id][1];
                  channel_id = signal_cache[signal_id][2];
                  GetHitInfo(tdc_id, channel_id, &hit_x, &hit_y);
                  hit_model[signal_id] = new TEllipse(hit_x, hit_y, radius, radius);
                  hit_model[signal_id]->SetFillColor(kRed);
                  hit_model[signal_id]->Draw();
                }
              }

              char canvas_name[256];
              char canvas_output_name[256];
              cout << "entry" << p_HPTDC_data->GetEntries() << endl;
              cout << "curret_event_id:" << current_event_id << " _event_id:" << _event_id << " event_id_group:" << event_id_group << endl;

              sprintf(canvas_name, "selected_event_id_%lu", current_event_id - 1); // already move to next event's header
              strcpy(canvas_output_name, canvas_name);
              strcat(canvas_output_name, ".png");
              hit_display_cache = new TCanvas(canvas_name,canvas_name, 1200, 480);
              hit_display_cache->Divide(1, 1);
              hit_display_cache->cd(1);
              track_baseline->SetNameTitle(canvas_name, canvas_name);
              track_base->DrawClonePad();

	      event_track[temp_track_group]->WriteTObject(hit_display_cache);
              #ifdef SAVE_TRACKS_OUT_OF_ROOT
	      chdir(track_group_name);
	      hit_display_cache->SaveAs(canvas_output_name);
	      chdir("..");
              #endif
	      total_tracks++;

              delete hit_display_cache;
              for (int signal_id = 0; signal_id < signal_cache_length;
                   signal_id++) {
                if (signal_cache[signal_id][0] == 4) {
                  delete hit_model[signal_id];
                }
              }
            }
          } // end if: pass event check


        } // end if: trigger cache length nonzero

	
	// clean cache
        for (int trigger_id = 0; trigger_id < trigger_cache_length; trigger_id++) {
          p_HPTDC_data->Fill(trigger_cache[trigger_id][0],
                             trigger_cache[trigger_id][1],
                             trigger_cache[trigger_id][2],
                             trigger_cache[trigger_id][3],
                             trigger_cache[trigger_id][4],
                             kFALSE);
          for (int i = 0; i < 5; i++) {
            trigger_cache[trigger_id][i] = 0;
          }
        }
        for (int signal_id = 0; signal_id < signal_cache_length; signal_id++) {
          if (first_signal_flag[signal_cache[signal_id][1]][signal_cache[signal_id][2]] == kTRUE) {
            p_HPTDC_data->Fill(signal_cache[signal_id][0],
                              signal_cache[signal_id][1],
                              signal_cache[signal_id][2],
                              signal_cache[signal_id][3],
                              signal_cache[signal_id][4],
                              signal_flag[signal_id]);
          }

          for (int i = 0; i < 5; i++) {
            signal_cache[signal_id][i] = 0;
          }
        }

        trigger_cache_length = 0;
        signal_cache_length = 0;
      } // end if: got new event

      /* fill ntuple */
      p_HPTDC_data->Fill(_type, _tdc, 0, current_event_id, _bunch_id, kFALSE);

    } // end if: data of type 0, 2, 3 
    else if (header_type == 4 || header_type == 5) {
      /*
       * Header is a leading or trailing measurement
       * (See HPTDC Documentation, page 22)
       */


      /*collect new dataword*/
      type = word >> 28;
      tdc = word >> 24;
      channel = word >> 19;
      coarse = word >> 7;
      fine = word;
      _type = static_cast<unsigned int>((type.to_ulong()));
      _tdc = static_cast<unsigned int>((tdc.to_ulong()));
      _channel = static_cast<unsigned int>((channel.to_ulong()));
      _coarse = static_cast<unsigned int>((coarse.to_ulong()));
      _fine = static_cast<unsigned int>((fine.to_ulong()));

      /* collect data */
      new_data_line[0] = _type;
      new_data_line[1] = _tdc;
      new_data_line[2] = _channel;
      new_data_line[3] = _coarse;
      new_data_line[4] = _fine;
      if (trigger_cache_length > 256 - 1) {
        cout << "Warning: too much triggers in one event! cache length is set to max" << endl;
        trigger_cache_length = 256;
      } 
      else if (signal_cache_length > 256 - 1) {
        cout << "Warning: too much signals in one event! cache length is set to max" << endl;
        signal_cache_length = 256;
      } 
      else {
        if (new_data_line[1] == TRIGGER_MEZZ && new_data_line[2] == TRIGGER_CH) {
          trigger_cache[trigger_cache_length][0] = new_data_line[0];
          trigger_cache[trigger_cache_length][1] = new_data_line[1];
          trigger_cache[trigger_cache_length][2] = new_data_line[2];
          trigger_cache[trigger_cache_length][3] = new_data_line[3];
          trigger_cache[trigger_cache_length][4] = new_data_line[4];
          trigger_cache_length++;
	  if (header_type == 4) total_triggers++;
        }
        if (new_data_line[1] != TRIGGER_MEZZ) {
          signal_cache[signal_cache_length][0] = new_data_line[0];
          signal_cache[signal_cache_length][1] = new_data_line[1];
          signal_cache[signal_cache_length][2] = new_data_line[2];
          signal_cache[signal_cache_length][3] = new_data_line[3];
          signal_cache[signal_cache_length][4] = new_data_line[4];
          signal_cache_length++;
	  if (header_type == 4) total_signals++;
        }
      }

      /* fill histogram */
      time_in_ns = (_coarse + _fine / 128.0 ) * 25.0;
      if (header_type == 4) {
        p_leading_time->Fill(time_in_ns);
      }
      if (header_type == 5) {
        p_trailing_time->Fill(time_in_ns);
      }
    } // end else if: header of type 4, 5 
    total_loop++;
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
  TCanvas *p_hits_canvas[MAX_TUBE_LAYER];
  TCanvas *p_hits_together_canvas = new TCanvas("layer_distribution", "layer_distribution");
  char canvas_name[256];
  for (Int_t layer_id = MAX_TUBE_LAYER - 1; layer_id != -1; layer_id--) {
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

  p_output_rootfile->Write();

  /* export data to output directory */
  #ifdef SAVE_TRACKS_OUT_OF_ROOT
    TCanvas *p_output_canvas = new TCanvas("", "");
    p_output_canvas->cd();
    for (Int_t tdc_id = 0; tdc_id != MAX_TDC_QUANTITY; tdc_id++) {
      sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id,
              MAX_TDC_QUANTITY);
      chdir(directory_name);

      p_tdc_tdc_time[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d_tdc_time_spectrum.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);

      p_tdc_tdc_time_original[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d_tdc_time_spectrum_original.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);

      p_tdc_tdc_time_corrected[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d_tdc_time_spectrum_corrected.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);

      p_tdc_adc_time[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d__adc_time_spectrum.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);

      p_tdc_channel[tdc_id]->Draw();
      sprintf(output_filename, "tdc_%d__channel_distribution.png", tdc_id);
      p_output_canvas->SaveAs(output_filename);

      for (Int_t channel_id = 0; channel_id != MAX_TDC_CHANNEL_QUANTITY;
           channel_id++) {
        p_tdc_time[tdc_id][channel_id]->Draw();
        sprintf(output_filename, "tdc_%d__channel_%d__tdc_time_spectrum.png",
                tdc_id, channel_id);
        p_output_canvas->SaveAs(output_filename);

        p_tdc_time_original[tdc_id][channel_id]->Draw();
        sprintf(output_filename,
                "tdc_%d__channel_%d__tdc_time_spectrum_original.png",
                tdc_id, channel_id);
        p_output_canvas->SaveAs(output_filename);

        p_tdc_time_corrected[tdc_id][channel_id]->Draw();
        sprintf(output_filename,
                "tdc_%d__channel_%d__tdc_time_spectrum_corrected.png",
                tdc_id, channel_id);
        p_output_canvas->SaveAs(output_filename);

        p_adc_time[tdc_id][channel_id]->Draw();
        sprintf(output_filename, "tdc_%d__channel_%d__adc_time_spectrum.png",
                tdc_id, channel_id);
        p_output_canvas->SaveAs(output_filename);
      }
      chdir("..");
    }
  #endif

  delete p_output_rootfile;
  delete track_base;
  delete p_leading_canvas;
  delete p_trailing_canvas;

  strcpy(output_filename, filename);
  cout << endl;
  cout << "NTuple data saved to: " << output_root_filename << endl;
  cout << endl << "Work done." << endl;
  
  cout << "Total Triggers: " << total_triggers << endl;
  cout << "Total Events:   " << total_events   << endl;
  cout << "Pass  Triggers: " << total_triggers_pass << endl;
  cout << "Pass  Events:   " << total_events_pass << endl;
  cout << "Total Signals:  " << total_signals  << endl;
  cout << "Pass  Signals:  " << total_signals_pass << endl;
  gROOT->SetBatch(kFALSE); // resume setup
  return 0;
}

double calibrated_p(const double &w) {
  return std::exp(1.11925e+00 + 2.08708e-02 * w);
}

double ADC_Correction(double w, vector<Double_t> v_LUT) {
  if( w > 400.0 || w < 0 ) return 0;
  return v_LUT[static_cast<int>(w)];
}
