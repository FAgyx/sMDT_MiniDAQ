/*******************************************************************************
  file name: PlotTrack.cxx
  author: Zhe Yang
  created: 02/09/2019
  last modified: 02/22/2019

  description:
  -Plot track for .out file using Legendre Transform.

  reference:
  -Theodoros Alexopoulos, Michael Bachits, Manolis Dris, Evangelos N. Gazis, George Tsipolitis; “Implementation of the Legendre Transform for the Muon Track Segment Reconstruction in the ATLAS MDT Chambers”; 2007 IEEE Nuclear Science Symposium Conference Record.

  remark:
  -The method was used by Shuzhou in MDT cosmic test.

*******************************************************************************/

#include "PlotTrack.h"

#include <stdio.h>
#include <math.h>
#include <iostream>

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLine.h"
#include "TTree.h"

#include "src/DecodeRawData.h"
#include "src/GetHitInfo.h"
#include "src/GetHitLayerColumn.h"
#include "src/rtfunction.h"

#define _USE_MATH_DEFINES

const Double_t FITTING_PARA0 = 25000;
const Double_t FITTING_PARA1 = -80;
const Double_t FITTING_PARA2 = 10;
const Double_t FITTING_PARA3 = -1;

using namespace std;

Double_t TestFunction(Double_t *x, Double_t *par) {
  return par[0] + par[1] * x[0];
}// used for debugging only

int TdcFit(TH1F *input_hist, Double_t fit_output_value[2][5],
           Double_t fit_initial_value_t0[5], Double_t fit_initial_value_tmax[5],Bool_t plot_flag);

Double_t GetPointLineDistance(Double_t x, Double_t y, Double_t k, Double_t b);

int PlotTrack(string s_input_filename = "Output/run00187691_20190301.dat.dir/run00187691_20190301.dat.out.root") {

  Int_t entry_id_low    = 0;

  // read input rootfile and set branch
  TFile *p_input_rootfile = new TFile(s_input_filename.c_str());
  TTree *p_input_tree = (TTree*)p_input_rootfile->Get("HPTDCData");
  Float_t type, tdc, data0, data1, data2;
  p_input_tree->SetBranchAddress("type", &type);
  p_input_tree->SetBranchAddress("tdc", &tdc);
  p_input_tree->SetBranchAddress("data0", &data0);
  p_input_tree->SetBranchAddress("data1", &data1);
  p_input_tree->SetBranchAddress("data2", &data2);

  // find the event and get event's data
  Int_t total_entries = p_input_tree->GetEntries();
  Int_t event_length = 0;
  Int_t event_trigger_length = 0;
  Int_t event_signal_length = 0;
  Int_t event_trigger[100][5];
  Int_t event_signal[100][5];
  bool event_start_flag = false;

  int user_input = 0;

  while (user_input != '.') {
    event_start_flag = false;
    unsigned long selected_event_id;

    // find the first good event
    for (Int_t entry_id = entry_id_low; entry_id < total_entries; entry_id++) {
      p_input_tree->GetEntry(entry_id);

      if ((type == 0 || type == 2 || type || 3) && !event_start_flag) {
	event_start_flag = true;
	selected_event_id = data1;
      } // end if: start of first event

      if (event_start_flag) {
	if ((type == 0 || type == 2 || type == 3) && data1 != selected_event_id) {
	  break;
	} // end if: event has ended
      
	else if (type == 4 || type == 5) {
	  if (event_length < 100) {
	    if (tdc == 1) {
	      event_trigger[event_trigger_length][0] = type;
	      event_trigger[event_trigger_length][1] = tdc;
	      event_trigger[event_trigger_length][2] = data0;
	      event_trigger[event_trigger_length][3] = data1;
	      event_trigger[event_trigger_length][4] = data2;
	      event_trigger_length++;
	    } else {
	      event_signal[event_signal_length][0] = type;
	      event_signal[event_signal_length][1] = tdc;
	      event_signal[event_signal_length][2] = data0;
	      event_signal[event_signal_length][3] = data1;
	      event_signal[event_signal_length][4] = data2;
	      event_signal_length++;
	    }
	    event_length++;
	  } // end if: this event buffer not yet full
	} // end else if: in bulk of event data
      } // end if: event has started
      entry_id_low++;
    } // end for: entry ID
    if (event_trigger_length < 1 || event_signal_length < 2) {
      cout << "missing trigger or signal, can't construct track" << endl;
      continue;
    }
    

    user_input = PlotTrackFromData(selected_event_id, p_input_rootfile, event_trigger_length, event_signal_length, event_trigger, event_signal);

  } // end while: user wants to see next event



  cout << "Exiting" << endl;
  return 0;
}

int PlotTrackFromData(unsigned long selected_event_id, TFile *p_input_rootfile, Int_t event_trigger_length, Int_t event_signal_length, Int_t event_trigger[100][5], Int_t event_signal[100][5]) {

  // use TdcFit() to get T0;
  Double_t t0_value[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  Double_t fit_output_value[2][5];
  TH1F *p_drift_time_hist[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY];
  char directory_name[256];
  char hist_name[256];
  char temp_name[256];
  for (Int_t tdc_id = 0; tdc_id < MAX_TDC_QUANTITY; tdc_id++) {
    for (Int_t ch_id = 0; ch_id != MAX_TDC_CHANNEL_QUANTITY; ch_id++) {
      sprintf(directory_name, "TDC_%02d_of_%02d_Time_Spectrum", tdc_id, MAX_TDC_QUANTITY);
      sprintf(temp_name, "tdc_%d_channel_%d_tdc_time_spectrum_corrected", tdc_id, ch_id);
      strcpy(hist_name, directory_name);
      strcat(hist_name, "/");
      strcat(hist_name, temp_name);
      p_input_rootfile->GetObject(hist_name, p_drift_time_hist[tdc_id][ch_id]);
      p_drift_time_hist[tdc_id][ch_id]->Draw();
      TdcFit(p_drift_time_hist[tdc_id][ch_id], fit_output_value, nullptr, nullptr, false);
      t0_value[tdc_id][ch_id] = fit_output_value[0][1];
    }
  }

  for (Int_t tdc_id = 0; tdc_id < MAX_TDC_QUANTITY; tdc_id++) {
    cout << "// #" << tdc_id << " //////////////" << endl;
    for (Int_t ch_id =0; ch_id < MAX_TDC_CHANNEL_QUANTITY; ch_id++) {
      cout << "t0 = " << t0_value[tdc_id][ch_id] << endl;
    }
  }

  // get signal absolute time
  Double_t temp_signal_time = 0;
  Double_t total_signal_time = 0;
  Double_t mean_signal_time = 0;
  Double_t signal_time[100];
  for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
    if (event_signal[signal_id][0] == 4) {
      temp_signal_time = (event_signal[signal_id][3] + event_signal[signal_id][4]/ 128.0 ) * 25.0; // time unit: ns
      total_signal_time += temp_signal_time;
      signal_time[signal_id] = temp_signal_time;
    } else if (event_signal[signal_id][0] == 5) {
      temp_signal_time = (event_signal[signal_id][3] + event_signal[signal_id][4]/ 128.0 ) * 25.0; // time unit: ns
      signal_time[signal_id] = temp_signal_time;
    }
  }
  mean_signal_time = total_signal_time / event_signal_length;

  // get trigger absolute time
  Double_t temp_trigger_time = 0;
  Double_t trigger_time = 102400;
  for (int trigger_id = 0; trigger_id < event_trigger_length; trigger_id++) {
    if (event_trigger[trigger_id][0] == 4) {
      temp_trigger_time = (event_trigger[trigger_id][3] + event_trigger[trigger_id][4]/ 128.0 ) * 25.0; // time unit: ns
      if (abs(temp_trigger_time - mean_signal_time) <
          abs(trigger_time - mean_signal_time)) {
        trigger_time = temp_trigger_time;
      }
    }
  }

  // get signal relative (drift) time
  Double_t drift_time[100];
  Bool_t in_range[100];
  for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
    // 100 ns shift is added here
    drift_time[signal_id] = signal_time[signal_id] - trigger_time
                            - t0_value[event_signal[signal_id][1]][event_signal[signal_id][2]];
    in_range[signal_id] = true;
    // set below-zero drift time to zero
    if (drift_time[signal_id] < 0) {
      drift_time[signal_id] = 0;
      in_range[signal_id] = false;
    }
    // set 200ns+ drift time to 200ns
    if (drift_time[signal_id] > 200) {
      drift_time[signal_id] = 200;
      in_range[signal_id] = false;
    }
  }

  // get signal drift distance
  Double_t drift_distance[100];
  RtFunction rt("src/Rt_BMG_6_1.dat");
  for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
    //drift_distance[signal_id] = RTFunction(drift_time[signal_id]);
    drift_distance[signal_id] = rt.GetRadius(drift_time[signal_id]);
  }

  // display event's data
  cout << "selected event's trigger: " << endl
       << "header type | tdc id | channel id | trigger time"
       << endl;
  for (int trigger_id = 0; trigger_id < event_trigger_length; trigger_id++) {
    cout << event_trigger[trigger_id][0] << " "
         << event_trigger[trigger_id][1] << " "
         << event_trigger[trigger_id][2] << " "
         << (event_trigger[trigger_id][3] + event_trigger[trigger_id][4]/ 128.0 ) * 25.0; // time unit: ns
    cout << endl;
  }
  cout << "selected event's signal: " << endl
       << "header type | tdc id | channel id | signal time | drift time | drift distance" << endl;
  for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
    cout << event_signal[signal_id][0] << "\t"
         << event_signal[signal_id][1] << "\t"
         << event_signal[signal_id][2] << "\t"
         << signal_time[signal_id] << "\t"
         << drift_time[signal_id] << "\t"
         << drift_distance[signal_id] << endl;
  }

  // prepare base of output track display
  const Double_t layer_distance = 13.0769836;
  const Double_t column_distance = 15.1;
  const Double_t radius = 7.5;

  TCanvas *track_base = new TCanvas("track base", "track base", 0, 0, 1200, 480);
  track_base->cd();
  Double_t center_x, center_y;
  Double_t track_corner_x[2] = {0, 800};
  Double_t track_corner_y[2] = {0, 320};
  TGraph * track_baseline = new TGraph(2, track_corner_x, track_corner_y);
  char track_name[256];
  sprintf(track_name, "selected_event_id_%lu", selected_event_id);
  track_baseline->SetNameTitle(track_name, track_name);
  track_baseline->Draw("AP");

  TEllipse *tube_model[54][8];
  TEllipse *hit_model[512];
  for (Int_t layer_id = 0; layer_id != 4; layer_id++) {
    for (Int_t column_id = 0; column_id != 54; column_id++) {
    center_x = 7.5 + column_id * column_distance + ((layer_id + 1) % 2) *
               column_distance / 2.0;
    center_y = 7.5 + layer_id * layer_distance;
    tube_model[layer_id][column_id] = new TEllipse(center_x, center_y,
					       radius, radius);
    if ((column_id / 6) % 2 == 0) {
      tube_model[layer_id][column_id]->SetFillColor(kGray);
    }
    tube_model[layer_id][column_id]->Draw();
    }
  }
  for (Int_t layer_id = 4; layer_id != 8; layer_id++) {
    for (Int_t column_id = 0; column_id != 54; column_id++) {
    center_x = 7.5 + column_id * column_distance + ((layer_id + 1) % 2) *
               column_distance / 2.0;
    center_y = 7.5 + (layer_id - 4) * layer_distance + 224.231;
    tube_model[layer_id][column_id] = new TEllipse(center_x, center_y,
					       radius, radius);
    if ((column_id / 6) % 2 == 0) {
      tube_model[layer_id][column_id]->SetFillColor(kGray);
    }
    tube_model[layer_id][column_id]->Draw();
    }
  }
  for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
    if (event_signal[signal_id][0] == 4) {
      Double_t x_0, y_0;
      GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0,
                 &y_0);
      hit_model[signal_id] = new TEllipse(x_0, y_0, drift_distance[signal_id],
				      drift_distance[signal_id]);
      hit_model[signal_id]->SetLineColor(kBlue);
      hit_model[signal_id]->SetFillColor(kBlue);
      hit_model[signal_id]->Draw();
    }
  }

  // plot legendre curve, find the intersection
  Double_t theta, r, x_0, y_0; // parameters used in legendre curve
  Double_t fill_weight;
  TH2F *plot_map = new TH2F("plot_map", "plot_map", 800, 0, 4, 180, -900, 900);
  track_base->cd();
  for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
    if (event_signal[signal_id][0] == 4 && in_range[signal_id] == true) {
      GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0,
                 &y_0);
      for (Int_t theta_id = 0; theta_id != 10000; theta_id++) {
        theta = M_PI * theta_id / 10000;
        fill_weight = (drift_distance[signal_id] * drift_distance[signal_id]);
        r = LegendreUpperCurve(theta, x_0, y_0, drift_distance[signal_id]);
        plot_map->Fill(theta, r, fill_weight);
        r = LegendreLowerCurve(theta, x_0, y_0, drift_distance[signal_id]);
        plot_map->Fill(theta, r, fill_weight);
      }
    }
  }
  Int_t max_bin_theta, max_bin_r, max_bin_z;
  plot_map->GetMaximumBin(max_bin_theta, max_bin_r, max_bin_z);
  Double_t line_para_k, line_para_b; // line parameters for track line
  line_para_k = -1 / tan(max_bin_theta * 4.0 / 800.0);
  line_para_b = (-900 + max_bin_r * 1800 / 180.0) /
                sin(max_bin_theta * 4.0 / 800.0);
  //cout << max_bin_theta << " " << max_bin_r << " " << max_bin_z << endl;
  cout << "line parameter b = " << line_para_b << endl
       << "line parameter k = " << line_para_k << endl;
  TLine *track_line = new TLine(0,  line_para_b,
                                900, line_para_b + line_para_k * 900);
  track_line->SetLineColor(kGreen);
  //track_line->Draw("same");

  // find the accurate intersection
  theta = 0;
  r = 0;
  x_0 = 0;
  y_0 = 0;
  Double_t min_theta_limit, max_theta_limit;
  Double_t min_r_limit, max_r_limit;
  min_theta_limit = (max_bin_theta - 10) * 4.0 / 800.0;
  if (min_theta_limit < 0) min_theta_limit = 0;
  max_theta_limit = (max_bin_theta + 10) * 4.0 / 800.0;
  if (max_theta_limit > 4) max_theta_limit = 4;
  min_r_limit = -900 + (max_bin_r - 2) * 1800 / 180.0;
  if (min_r_limit < -800) min_theta_limit = -800;
  max_r_limit = -900 + (max_bin_r + 2) * 1800 / 180.0;
  if (max_r_limit > 800) max_r_limit = 800;
  TH2F *plot_map_accurate = new TH2F("plot_map_accurate", "plot_map_accurate",
                                     40, min_theta_limit, max_theta_limit,
                                     50, min_r_limit, max_r_limit);
  for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
    if (event_signal[signal_id][0] == 4) {
      for (Int_t theta_id = 0; theta_id != 10000; theta_id++) {
        theta = min_theta_limit +
                theta_id * (max_theta_limit - min_theta_limit) / 10000;
        GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0,
                   &y_0);
        r = LegendreUpperCurve(theta, x_0, y_0, drift_distance[signal_id]);
        plot_map_accurate->Fill(theta, r);
        r = LegendreLowerCurve(theta, x_0, y_0, drift_distance[signal_id]);
        plot_map_accurate->Fill(theta, r);
      }
    }
  }
  max_bin_theta = 0;
  max_bin_r = 0;
  max_bin_z = 0;
  plot_map_accurate->GetMaximumBin(max_bin_theta, max_bin_r, max_bin_z);
  line_para_k = -1 / tan(min_theta_limit + max_bin_theta * (max_theta_limit -
                min_theta_limit) / 40);
  line_para_b = (min_r_limit + max_bin_r * (max_r_limit - min_r_limit) / 50) /
                sin(min_theta_limit + max_bin_theta * (max_theta_limit - min_theta_limit) / 40);

  TLine *track_line_accurate = new TLine(0, line_para_b, 900,
                                         line_para_b + line_para_k * 900);
  track_line_accurate->SetLineColor(kOrange);
  //track_line_accurate->Draw("same");

  // find the point near the line found by legendremethod to perform linear fitting
  Double_t hit_position_x[100];
  Double_t hit_position_y[100];
  bool hit_position_acception[100];
  Double_t x_sum = 0;
  Double_t y_sum = 0;
  Double_t xy_sum = 0;
  Double_t xx_sum = 0;
  Double_t quantity = 0;
  for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
    Double_t possible_hit_position_x1;
    Double_t possible_hit_position_x2;
    Double_t possible_hit_position_y1;
    Double_t possible_hit_position_y2;
    Double_t x0, y0;
    Int_t hit_layer, hit_column;
    GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x0,
               &y0);
    GetHitLayerColumn(event_signal[signal_id][1], event_signal[signal_id][2],
                      &hit_layer, &hit_column);
    cout << "hit layer: " << hit_layer << " | hit column: " << hit_column << "  || x0: " << x0 << " | y0: " << y0;
    possible_hit_position_x1 = x0 - drift_distance[signal_id] * line_para_k / sqrt(line_para_k * line_para_k + 1);
    possible_hit_position_x2 = x0 + drift_distance[signal_id] * line_para_k / sqrt(line_para_k * line_para_k + 1);
    possible_hit_position_y1 = y0 + drift_distance[signal_id] / sqrt(line_para_k * line_para_k + 1);
    possible_hit_position_y2 = y0 - drift_distance[signal_id] / sqrt(line_para_k * line_para_k + 1);

    if (GetPointLineDistance(possible_hit_position_x1, possible_hit_position_y1, line_para_k, line_para_b) > GetPointLineDistance(possible_hit_position_x2, possible_hit_position_y2, line_para_k, line_para_b)) {
      hit_position_x[signal_id] = possible_hit_position_x2;
      hit_position_y[signal_id] = possible_hit_position_y2;
    } else if (GetPointLineDistance(possible_hit_position_x1, possible_hit_position_y1, line_para_k, line_para_b) < GetPointLineDistance(possible_hit_position_x2, possible_hit_position_y2, line_para_k, line_para_b)) {
      hit_position_x[signal_id] = possible_hit_position_x1;
      hit_position_y[signal_id] = possible_hit_position_y1;
    }
    cout << " | selected hit x: " << hit_position_x[signal_id] << " | selected hit y: " << hit_position_y[signal_id] << endl;

    Double_t sum_weight;
    if (fabs(GetPointLineDistance(hit_position_x[signal_id], hit_position_y[signal_id], line_para_k, line_para_b) - drift_distance[signal_id]) < 3 && event_signal[signal_id][0] == 4) {
      hit_position_acception[signal_id] = true;
      sum_weight = (drift_distance[signal_id] * drift_distance[signal_id]);
      x_sum += hit_position_x[signal_id] * sum_weight;
      y_sum += hit_position_y[signal_id] * sum_weight;
      xy_sum += hit_position_x[signal_id] * hit_position_y[signal_id] * sum_weight;
      xx_sum += hit_position_x[signal_id] * hit_position_x[signal_id] * sum_weight;
      quantity += sum_weight;
    } else {
      hit_position_acception[signal_id] = false;
    }
  }

  Double_t x_mean = 0;
  Double_t y_mean = 0;
  Double_t xy_mean = 0;
  Double_t xx_mean = 0;
  if (quantity != 0) {
    x_mean =  x_sum / quantity;
    y_mean =  y_sum / quantity;
    xy_mean =  xy_sum / quantity;
    xx_mean =  xx_sum / quantity;
  } else if (quantity == 0) {
    x_mean =  x_sum;
    y_mean =  y_sum;
    xy_mean =  xy_sum;
    xx_mean =  xx_sum;
  }

  Double_t final_k = (xy_mean - x_mean * y_mean) / (xx_mean - x_mean * x_mean);
  Double_t final_b = y_mean - final_k * x_mean;

  cout << "accurate line parameter b = " << final_b << endl
       << "accurate line parameter k = " << final_k << endl;
  TLine *track_line_final = new TLine(0, final_b, 900,
                                         final_b + final_k * 900);
  track_line_final->SetLineColor(kPink);
  track_line_final->Draw("same");

  // calculate segment residual
  cout << endl << "// Segment Residual ///////////////////////" << endl;
  for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
    if (event_signal[signal_id][0] == 4) {
      Double_t x_0, y_0, distance, residual;
      Int_t hit_layer, hit_column;
      GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0,
                 &y_0);
      GetHitLayerColumn(event_signal[signal_id][1], event_signal[signal_id][2],
                        &hit_layer, &hit_column);
      distance = GetPointLineDistance(x_0, y_0, final_k, final_b);
      residual = drift_distance[signal_id] - distance;
      cout << "hit layer: " << hit_layer << " | " << "hit column: " << hit_column << " | residual: " << residual << endl;
    }
  }
  cout << "///////////////////////////////////////////" << endl;

  // draw the plots for debugging
  TCanvas *track_map_canvas = new TCanvas("track", "track", 0, 550, 600, 600);
  track_map_canvas->cd();
  plot_map->SetStats(0);
  plot_map->Draw("COLZ");

    
  TCanvas *track_map_accurate_canvas = new TCanvas("track_accurate", "track_accurate", 600, 550, 600, 600);
  track_map_accurate_canvas->cd();
  plot_map_accurate->SetStats(0);
  plot_map_accurate->Draw("COLZ");
  
  track_base->Update();
  track_map_canvas->Update();
  track_map_accurate_canvas->Update();

  // check if user wants to continue
  cout << "Displaying event number   " << selected_event_id << endl;
  cout << "To continue, enter any key" << endl 
       << "To exit,     enter '.'    " << endl;
  int c;
  c = getchar();
  getchar();
  

  delete track_base;
  delete track_map_canvas;
  delete track_map_accurate_canvas;
  delete track_baseline;
  delete plot_map;
  delete track_line;
  delete plot_map_accurate;
  delete track_line_accurate;
  delete track_line_final;
  
  for (int i = 0; i < MAX_TDC_QUANTITY; i++) {
    for (int j = 0; j < MAX_TDC_CHANNEL_QUANTITY; j++) {
      delete p_drift_time_hist[i][j];
    }
  }

  return c;
}
// end PlotTrack ///////////////////////////////////////////////////////////////

Int_t TdcFit(TH1F *input_hist,
             Double_t fit_output_value[2][5],
             Double_t fit_initial_value_t0[5] = nullptr,
             Double_t fit_initial_value_tmax[5] = nullptr,
             Bool_t plot_flag = false) {
  // t0 fitting parameters
  Double_t t0_amplitude, t0, t0_slope, t0_top_slope, t0_noise, t0_fit_range_min, t0_fit_range_max;

  // histogram features
  Double_t max_bin_value;

  // Protect for null histograms and low statistics time spectra
  if (input_hist == nullptr || input_hist->GetEntries() < 3000.0) return 1;

  // default text box locations
  gStyle->SetOptStat(0);
  TPaveText *entry_pave_text = new TPaveText(0.65, 0.8, 0.9, 0.9, "NDC");
  TPaveText *t0_pave_text = new TPaveText(0.65, 0.7, 0.9, 0.8, "NDC");
  TPaveText *tmax_pave_text = new TPaveText(0.65, 0.6, 0.9, 0.7, "NDC");
  TPaveText *t_drift_time_pave_text = new TPaveText(0.65, 0.5, 0.9, 0.6, "NDC");

  // Perform T0 fit
  // initialize parameters for t0 fittind, use default if not specified
  if (fit_initial_value_t0 != nullptr) {
    t0_amplitude = fit_initial_value_t0[0];
    t0 = fit_initial_value_t0[1];
    t0_slope = fit_initial_value_t0[2];
    t0_top_slope = fit_initial_value_t0[3];
    t0_noise = fit_initial_value_t0[4];
  } else {
    max_bin_value = input_hist->GetMaximum();
    t0_amplitude = max_bin_value / 1.1;
    t0 = input_hist->GetBinCenter(input_hist->FindFirstBinAbove(0.45 * max_bin_value));
    t0_slope = 2.5;
    t0_top_slope = 0;
    t0_noise = t0_amplitude / 100.0;
  }
  t0_fit_range_min = t0 - 300.0;
  t0_fit_range_max = input_hist->GetBinCenter(input_hist->GetMaximumBin());
  // Define TF1 function for T0 fit and perform fitting
  TF1 *t0_fit_function =  new TF1("t0_fit_function", FermiDiracFunction,
                                t0_fit_range_min, t0_fit_range_max, 5);
  t0_fit_function->SetParameters(t0_amplitude, t0, t0_slope, t0_top_slope);
  t0_fit_function->SetParLimits(5, 0, t0_amplitude / 5.0); //Do not allow negative t0_noise
  input_hist->Fit("t0_fit_function", "R");

  if (fit_output_value != nullptr) {
    fit_output_value[0][0] = t0_fit_function->GetParameter(0);
    fit_output_value[0][1] = t0_fit_function->GetParameter(1);
    fit_output_value[0][2] = t0_fit_function->GetParameter(2);
    fit_output_value[0][3] = t0_fit_function->GetParameter(3);
    fit_output_value[0][4] = t0_fit_function->GetParameter(4);
  }

  t0_pave_text->AddText(Form("T0 = %.2lf #pm %.2lf ns",
                      t0_fit_function->GetParameter(1),
                      t0_fit_function->GetParError(1)));
  t0_pave_text->AddText(Form("Slope = %.2lf #pm %.2lf ns",
                      t0_fit_function->GetParameter(2),
                      t0_fit_function->GetParError(2)));
  t0_pave_text->SetTextColor(kRed);

  // Perform Tmax fit //
  // tmax fitting parameters
  Double_t tmax_amplitude, tmax, tmax_slope, tmax_top_slope, tmax_noise, tmax_fit_range_min, tmax_fit_range_max;
  // initialize parameters for tmax fittind, use default if not specified
  if (fit_initial_value_t0 != nullptr) {
    tmax_amplitude = fit_initial_value_tmax[0];
    tmax = fit_initial_value_tmax[1];
    tmax_slope = fit_initial_value_tmax[2];
    tmax_top_slope = fit_initial_value_tmax[3];
    tmax_noise = fit_initial_value_tmax[4];
  } else {
    tmax_amplitude = max_bin_value / 2.0;
    tmax = input_hist->GetBinCenter(input_hist->FindLastBinAbove(0.1 * max_bin_value));
    tmax_slope = -8.0;
    tmax_top_slope = 0;
    tmax_noise = t0_fit_function->GetParameter(2);
  }
  tmax_fit_range_min = tmax - 80.;
  tmax_fit_range_max = tmax + 200.;
  // Define TF1 function for Tmax fit
  TF1 *tmax_fit_function = new TF1("tmax_fit_function", FermiDiracFunction,
                                    tmax_fit_range_min, tmax_fit_range_max, 5);
  tmax_fit_function->SetParameters(tmax_amplitude, tmax, tmax_slope,
                                   tmax_top_slope);
  tmax_fit_function->SetParLimits(1, t0_fit_function->GetParameter(1),
                                  t0_fit_function->GetParameter(1) + 300);
  tmax_fit_function->SetLineColor(kGreen);
  input_hist->Fit("tmax_fit_function", "r+"); //the "+" is to add the function to the list
  tmax_fit_function->SetLineColor(kGreen);

  if (fit_output_value != nullptr) {
    fit_output_value[1][0] = tmax_fit_function->GetParameter(0);
    fit_output_value[1][1] = tmax_fit_function->GetParameter(1);
    fit_output_value[1][2] = tmax_fit_function->GetParameter(2);
    fit_output_value[1][3] = tmax_fit_function->GetParameter(3);
    fit_output_value[1][4] = tmax_fit_function->GetParameter(4);
  }

  tmax_pave_text->AddText(Form("Tmax %.1lf #pm %.1lf ns",
                      tmax_fit_function->GetParameter(0),
                      tmax_fit_function->GetParError(0)));
  tmax_pave_text->AddText(Form("Slope %.1lf #pm %.1lf /ns",
                      tmax_fit_function->GetParameter(1),tmax_fit_function->GetParError(1)));
  tmax_pave_text->SetTextColor(kGreen);

  // Compute max drift time of time spectra: Tmax-T0
  Double_t drift_time_max = tmax_fit_function->GetParameter(1) -
                            t0_fit_function->GetParameter(1);
  Double_t drift_time_max_err = sqrt(tmax_fit_function->GetParError(0) *
                                     tmax_fit_function->GetParError(0) +
                                     t0_fit_function->GetParError(0) *
                                     t0_fit_function->GetParError(0));
  t_drift_time_pave_text->AddText(Form("Max drift time"));
  t_drift_time_pave_text->AddText(Form("%.1lf #pm %.1lf ns", drift_time_max,
                                        drift_time_max_err));
  t_drift_time_pave_text->SetTextColor(kViolet);
  t_drift_time_pave_text->Draw();

  // draw TPaveText in order to avoid overlap between each TPaveText
  entry_pave_text->AddText(Form("Total entries: %.0f",
                                 input_hist->GetEntries()));
  entry_pave_text->SetTextColor(kBlack);
  entry_pave_text->Draw();
  t0_pave_text->Draw();
  tmax_pave_text->Draw();
  t_drift_time_pave_text->Draw();

  return 0;
}
// end TdcFit //////////////////////////////////////////////////////////////////

Double_t GetPointLineDistance(Double_t x, Double_t y, Double_t k, Double_t b) {
  return fabs((k * x - y +b) / sqrt(k * k + 1.0));
}
