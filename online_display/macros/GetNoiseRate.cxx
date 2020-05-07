/*******************************************************************************
  file name: GetNoiseRate.cxx
  author: Zhe Yang
  created: 02/19/2019
  last modified: 03/05/2019

  description:
  -Calculate the noise rate for each tube using the formula:
  noise_rate  = hits_in_searchwindow / (searchwindow * trigger_number)

  remark:
  -search window is set to (0, 70ns)

*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2F.h"
#include "TLine.h"
#include "TTree.h"

#include "src/DecodeRawData.h"
#include "src/GetHitInfo.h"
#include "src/GetTrack.h"

#define _USE_MATH_DEFINES
//#define GETNOISERATE_DEBUG

#define TOTAL_BIN_QUANTITY 1024


using namespace std;

Int_t GetHistId_8(Double_t radius);
Int_t GetHistId_16(Double_t radius);


double calibrated_p(const double &w) {
  return std::exp(1.11925e+00 + 2.08708e-02 * w);
}

double ADC_Correction(double w, vector<Double_t> v_LUT) {
  if( w > 400.0 || w < 0 ) return 0;
  return v_LUT[static_cast<int>(w)];
}

int GetNoiseRate(string s_input_filename = "run00187691_20190301.dat") {
  // stop canvas from popping up
  Bool_t batch_mode_flag;
  if (gROOT->IsBatch()) {
    batch_mode_flag = true;
  } else {
    batch_mode_flag = false;
  }
  gROOT->SetBatch(kTRUE);

  // prepare adc correction table
  vector<Double_t> v_LUT;
  for (Int_t i = 0; i != 400; i++) {
    v_LUT.push_back(109./calibrated_p(i));
  }

  // read input rootfile and set branch
  char input_path_name[256];
  char input_directory_name[256];
  sprintf(input_path_name, "output/");
  strcat(input_path_name, s_input_filename.c_str());
  strcat(input_path_name, ".dir/");
  strcpy(input_directory_name, input_path_name);
  strcat(input_path_name, s_input_filename.c_str());
  strcat(input_path_name, ".out.root");
  TFile *p_input_rootfile = new TFile(input_path_name);
  //TFile *p_input_t0_rootfile = new TFile(s_input_t0_path.c_str());
  TTree *p_input_tree = (TTree*)p_input_rootfile->Get("HPTDCData");
  Float_t type, tdc, data0, data1, data2, signal_flag;
  p_input_tree->SetBranchAddress("type", &type);
  p_input_tree->SetBranchAddress("tdc", &tdc);
  p_input_tree->SetBranchAddress("data0", &data0);
  p_input_tree->SetBranchAddress("data1", &data1);
  p_input_tree->SetBranchAddress("data2", &data2);
  p_input_tree->SetBranchAddress("signal_flag", &signal_flag);

  // get drift time distribution histogram and calculate T0
  Long_t total_noise_count = 0;
  Long_t test_noise_count = 0;
  Long_t total_trigger_count = 0;
  Long_t noise_count[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY] = {{0}};
  Double_t t0_value[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  Double_t fit_output_value[2][5];
  TH1F *p_drift_time_hist[MAX_TDC_QUANTITY];
  char directory_name[256];
  char hist_name[256];
  char temp_name[256];
  char t0_path[256];
  char fitVecName[256];
  sprintf(t0_path, "output/");
  strcat(t0_path, s_input_filename.c_str());
  strcat(t0_path, ".dir/T0.root");
  TFile t0File(t0_path);
  for (Int_t tdc_id = 8; tdc_id < 12; tdc_id++) {
    for (Int_t ch_id = 0; ch_id != MAX_TDC_CHANNEL_QUANTITY; ch_id++) {
      sprintf(fitVecName, "FitData_tdc_%d_channel_%d", tdc_id, ch_id);
      TVectorD *fitParams = (TVectorD*)t0File.Get(fitVecName);
      t0_value[tdc_id][ch_id] = (*fitParams)[1];
    }
  }

  //// calculate noise rate accurately
  // prepare output file
  char output_directory_name[256];
  strcpy(output_directory_name, input_directory_name);
  strcat(output_directory_name, "noise_level_output");
  if (mkdir(output_directory_name, 0777) == -1) {
    cerr << strerror(errno) << endl;
  }
  char output_path_name[256];
  strcpy(output_path_name, output_directory_name);
  strcat(output_path_name, s_input_filename.c_str());
  strcat(output_path_name, ".noise.root");
  TFile *p_output_rootfile = new TFile(output_path_name, "RECREATE");
  TNtuple *p_noise_data = new TNtuple("NoiseData", "NoiseData", "event_id:tdc_id:channel_id:coarse:fine");
  TH1F *p_radius_hist = new TH1F("radius_distribution", "radius_distribution", 200, 0, 8);
  p_radius_hist->GetXaxis()->SetTitle("radius/mm");
  p_radius_hist->GetYaxis()->SetTitle("entries");
  TH1F *p_residual_hist = new TH1F("residual_distribution", "residual_distribution", 100, -2000, 2000);
  p_residual_hist->GetXaxis()->SetTitle("residual/mm");
  p_residual_hist->GetYaxis()->SetTitle("entries");
  TH1F *p_residual_hist_2 = new TH1F("residual_distribution_2", "residual_distribution_2", 100, -2000, 2000);
  p_residual_hist_2->GetXaxis()->SetTitle("residual/mm");
  p_residual_hist_2->GetYaxis()->SetTitle("entries");
  TH1F *p_residual_hist_vs_radius[8];
  TH1F *p_residual_hist_vs_radius_2[8];
  for (Int_t i = 0; i != 8; i++) {
    char hist_name[128];
    sprintf(hist_name, "residual_distribution_%dmm", i);
    p_residual_hist_vs_radius[i] = new TH1F(hist_name, hist_name, 100, -1000, 1000);
    p_residual_hist_vs_radius[i]->GetXaxis()->SetTitle("residual/mm");
    p_residual_hist_vs_radius[i]->GetYaxis()->SetTitle("entries");
    sprintf(hist_name, "residual_distribution_%dmm_2", i);
    p_residual_hist_vs_radius_2[i] = new TH1F(hist_name, hist_name, 100, -1000, 1000);
    p_residual_hist_vs_radius_2[i]->GetXaxis()->SetTitle("residual/mm");
    p_residual_hist_vs_radius_2[i]->GetYaxis()->SetTitle("entries");
  }
  TH2F *p_residual_vs_radius = new TH2F("residual vs radius", "residual vs radius", 100, 0, 8, 100, -500, 500);
  p_residual_vs_radius->GetXaxis()->SetTitle("radius/mm");
  p_residual_vs_radius->GetYaxis()->SetTitle("residual/um");


  // find the event and get event's data
  Int_t total_entries = p_input_tree->GetEntries();
  Int_t event_id = 0;
  Int_t current_event_id = -1;
  Int_t event_length = 0;
  Int_t event_trigger_length = 0;
  Int_t event_signal_length = 0;
  Int_t event_trigger[128][6];
  Int_t event_signal[128][6];
  Double_t output_line_parameter_k;
  Double_t output_line_parameter_b;
  Double_t current_time = 0;
  bool event_start_flag = false;
  bool good_track_flag = false;
  Int_t output_good_hit_flag[128];
  Double_t output_residual[128];
  Double_t output_drift_time[128];
  Double_t output_drift_distance[128];
  #ifdef GETNOISERATE_DEBUG
  gROOT->SetBatch(kFALSE);
  #endif
  //for (Int_t entry_id = 0; entry_id < total_entries; entry_id++) {
  for (Int_t entry_id = 0; entry_id < 10000 && entry_id < total_entries; entry_id++) {
    p_input_tree->GetEntry(entry_id);

    if (type == 0 || type == 2 || type == 3) {
      event_id = data1;
    } 
    else if (type == 4 || type == 5) {
      if (event_length < 100) {
        if (tdc == 1) {
          event_trigger[event_trigger_length][0] = type;
          event_trigger[event_trigger_length][1] = tdc;
          event_trigger[event_trigger_length][2] = data0;
          event_trigger[event_trigger_length][3] = data1;
          event_trigger[event_trigger_length][4] = data2;
          event_trigger[event_trigger_length][5] = signal_flag;
          event_trigger_length++;
        } 
	else {
          if (signal_flag) {
            event_signal[event_signal_length][0] = type;
            event_signal[event_signal_length][1] = tdc;
            event_signal[event_signal_length][2] = data0;
            event_signal[event_signal_length][3] = data1;
            event_signal[event_signal_length][4] = data2;
            event_signal[event_signal_length][5] = signal_flag;
            event_signal_length++;
          }
        }
        event_length++;
      }
    }

    if (current_event_id != event_id) {
      cout << "Current event id: " << current_event_id << endl;
      good_track_flag = GetTrack(event_trigger_length, event_trigger,
                                 event_signal_length, event_signal, t0_value,
                                 &output_line_parameter_k,
                                 &output_line_parameter_b,
                                 output_drift_time,
                                 output_drift_distance,
                                 output_residual,
                                 output_good_hit_flag);
      #ifdef GETNOISERATE_DEBUG
      // debugging
      if (good_track_flag) {
        cout << "Paused. Enter 'e' to exit or other key to continue." << endl;
        if (getchar() == 'e') {
          cout << "Program is stopped by user." << endl;
          return 1;
        }
      }
      #endif

      for (Int_t trigger_id = 0; trigger_id < event_trigger_length; trigger_id++) {
        if (event_trigger[trigger_id][0] == 4) {
          total_trigger_count++;
        }
      }

      if (good_track_flag) {
        // fill residual and noise data
        for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
          if (output_good_hit_flag[signal_id] == -1) {
            if (event_signal[signal_id][0] == 4
                && event_signal[signal_id][1] != 1) {
              current_time = (event_signal[signal_id][3] +
                              event_signal[signal_id][4] / 128.0 ) *
		              25.0;
              if (current_time > 0 && current_time < 70) {
                noise_count[event_signal[signal_id][1]][event_signal[signal_id][2]]++;
                total_noise_count++;
              }
	      
              p_noise_data->Fill(current_event_id,
				 event_signal[signal_id][1],
				 event_signal[signal_id][2],
				 event_signal[signal_id][3],
				 event_signal[signal_id][4]);
            }
          } else if (output_good_hit_flag[signal_id] != -1) {
            if (event_signal[signal_id][0] == 4
                && event_signal[signal_id][1] != 1) {
              p_residual_hist->Fill(output_residual[signal_id] * 1000);
              p_residual_vs_radius->Fill(output_drift_distance[signal_id], output_residual[signal_id] * 1000);
              //Int_t hist_id = GetHistId_16(output_drift_distance[signal_id]);
              Int_t hist_id = output_good_hit_flag[signal_id];
              p_residual_hist_vs_radius[hist_id]->Fill(output_residual[signal_id] * 1000);
              p_radius_hist->Fill(output_drift_distance[signal_id]);
            }
          }
        }
	
        // fill residual using another method
        if (fabs(output_line_parameter_k) > 100) {
          Bool_t upper_layers_flag1 = kFALSE;
          Bool_t upper_layers_flag2 = kFALSE;
          Bool_t same_layer_flag = kFALSE;
          Bool_t pair_flag = kFALSE;
          for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
            if (event_signal[signal_id][0] == 4 && 
		event_signal[signal_id][1] != 1 &&
		output_good_hit_flag[signal_id] != -1) {
              Double_t residual1, residual2;
              residual1 = output_residual[signal_id] * 1000;
              Int_t hit_layer, hit_column;
              GetHitLayerColumn(event_signal[signal_id][1], event_signal[signal_id][2], &hit_layer, &hit_column);
              if (hit_layer < 4) {
                upper_layers_flag1 = kFALSE;
              } else {
                upper_layers_flag1 = kTRUE;
              }

              for (Int_t signal_id2 = signal_id + 1; signal_id2 < event_signal_length; signal_id2++) {
                if (event_signal[signal_id2][0] == 4 && 
		    event_signal[signal_id2][1] != 1 && 
		    output_good_hit_flag[signal_id] != -1) {
                  Int_t hit_layer2, hit_column2;
                  GetHitLayerColumn(event_signal[signal_id2][1], event_signal[signal_id2][2], &hit_layer2, &hit_column2);
                  if (hit_layer2 < 4) {
                    upper_layers_flag2 = kFALSE;
                  } else {
                    upper_layers_flag2 = kTRUE;
                  }
                  same_layer_flag = (upper_layers_flag1 == upper_layers_flag2);
                  if (abs(hit_layer - hit_layer2) == 2) {
                    pair_flag = kTRUE;
                  } else {
                    pair_flag = kFALSE;
                  }

                  if (same_layer_flag && pair_flag) {
                    residual2 = output_residual[signal_id2] * 1000;
                    Double_t fill_value = (residual1 - residual2) / sqrt(2);
                    p_residual_hist_2->Fill(fill_value);
                    Double_t radius_mean = (output_drift_distance[signal_id] + output_drift_distance[signal_id2]) / 2;
                    //Int_t hist_id = GetHistId_16(radius_mean);
                    Int_t hist_id =  GetHistId_8(radius_mean);
                    p_residual_hist_vs_radius_2[hist_id]->Fill(fill_value);
                  }
                }
              }
            }
          }
        }
      } // end if: good track flag
      
      event_trigger_length = 0;
      event_signal_length = 0;
      event_length = 0;
      
      current_event_id = event_id;
    } // end if: current_event_id != event_id (trigger is of type 0, 2, 3)
    
  } // end for: entry_id (NTuple loop)

  TCanvas *p_plot_canvas = new TCanvas();
  p_plot_canvas->cd();
  p_residual_vs_radius->Draw("colz");
  p_plot_canvas->SaveAs("p_residual_vs_radius.png");

  // calculate noise rate
  Double_t channel_noise_rate = 0;
  Double_t test_noise_rate;
  for (Int_t tdc_id = 0; tdc_id < MAX_TDC_QUANTITY; tdc_id++) {
    for (Int_t ch_id = 0; ch_id < MAX_TDC_CHANNEL_QUANTITY; ch_id++) {
      channel_noise_rate = noise_count[tdc_id][ch_id] / (0.00000007 * total_trigger_count);
      if (tdc_id >= 8 && tdc_id <= 11) {
        cout << "tdc#" << tdc_id << " ch#" << ch_id << " noise_rate: " << channel_noise_rate << endl;
      }
      test_noise_rate += channel_noise_rate;
    }
  }

  Double_t noise_rate = total_noise_count / (0.00000007 * total_trigger_count);
  cout << "noise counts: " << total_noise_count << endl;
  cout << "trigger counts: " << total_trigger_count << endl;
  cout << "total noise rate is: " << noise_rate << " Hz" << endl;
  cout << "test noise rate is: " << test_noise_rate << " Hz" << endl;

  //p_residual_hist->SetStats(kTRUE);
  //p_residual_hist->Draw();
  //p_output_rootfile->Write();

  // resume batch mode setup
  if (batch_mode_flag == false) {
    gROOT->SetBatch(kFALSE);
  }

  p_output_rootfile->Write();

  return 0;
}

// get hist id if use 8 bins, each bin's range is 1mm
Int_t GetHistId_8(Double_t radius) {
  Int_t integer_bit = static_cast<int>(radius);
  if (integer_bit >= 0 && integer_bit <= 7) {
    return integer_bit;
  } else {
    return -1; // unexpected result
  }
}

// get hist id if use 16 bins, each bin's range is 0.5mm
Int_t GetHistId_16(Double_t radius) {
  Int_t integer_bit = static_cast<int>(radius);
  Double_t decimal_bit = radius - integer_bit;
  if (decimal_bit >= 0.5) {
    return 2 * integer_bit + 1;
  } else if (decimal_bit < 0.5) {
    return 2 * integer_bit;
  } else {
    return -1; // unexpected result
  }
}
