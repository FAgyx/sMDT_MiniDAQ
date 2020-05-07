/*******************************************************************************
  file name: GetTrack.h
  author: Zhe Yang
  created: 03/05/2019
  last modified: 03/05/2019

  description:
  -Get track parameters with given event data using Legendre Transform. 

  reference:
  -Theodoros Alexopoulos, Michael Bachits, Manolis Dris, Evangelos N. Gazis, George Tsipolitis; “Implementation of the Legendre Transform for the Muon Track Segment Reconstruction in the ATLAS MDT Chambers”; 2007 IEEE Nuclear Science Symposium Conference Record.

*******************************************************************************/

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
#include "TPrincipal.h"
#include "TTree.h"

#include "src/DecodeRawData.h"
#include "src/GetHitInfo.h"
#include "src/GetHitLayerColumn.h"
#include "src/rtfunction.h"

#define _USE_MATH_DEFINES
//#define GETTRACK_H_DEBUG

using namespace std;

Double_t GetPointLineDistance(Double_t x, Double_t y, Double_t k, Double_t b);

Double_t LegendreUpperCurve(Double_t theta, Double_t x_0, Double_t y_0, 
                            Double_t r_0) {
  return x_0 * cos(theta) + y_0 * sin(theta) + r_0;
}

Double_t LegendreLowerCurve(Double_t theta, Double_t x_0, Double_t y_0, 
                            Double_t r_0) {
  return x_0 * cos(theta) + y_0 * sin(theta) - r_0;
}

Double_t float_t0_value = 5;

RtFunction rt("src/Rt_BMG_6_1.dat");

Bool_t GetTrack(Int_t event_trigger_length, Int_t event_trigger[128][6],
                Int_t event_signal_length, Int_t event_signal[128][6],
                Double_t t0_value[MAX_TDC_QUANTITY][MAX_TDC_CHANNEL_QUANTITY],
                Double_t *output_line_parameter_k,
                Double_t *output_line_parameter_b,
                Double_t drift_time[100],
                Double_t drift_distance[100],
                Double_t output_residual[128],
                Int_t output_good_hit_flag[128],
		Bool_t verbose=false) {
  // check whether the input data is enough
  if (event_trigger_length < 1 || event_signal_length < 4) {
    if (verbose) cout << "missing trigger or signal, can't construct track" << endl;
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      output_good_hit_flag[signal_id] = -1; 
    }
    return kFALSE; // data is noise
  } else {
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      output_good_hit_flag[signal_id] = 100; // represent radius range
                                             // set initialization out of range
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
  Bool_t in_range[100];
  for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
    drift_time[signal_id] = signal_time[signal_id] - trigger_time 
                            - (t0_value[event_signal[signal_id][1]][event_signal[signal_id][2]] + float_t0_value); 
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
  //RtFunction rt("Rt_BMG_6_1.dat");
  for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
    //drift_distance[signal_id] = RTFunction(drift_time[signal_id]);
    drift_distance[signal_id] = rt.GetRadius(drift_time[signal_id]);
    if (verbose) cout << "drift distance:" << drift_distance[signal_id] << endl;
  }

  // display event's data
  if (verbose) {
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
  }

  // prepare base of output track display
  const Double_t layer_distance = 13.0769836;
  const Double_t column_distance = 15.1;
  const Double_t radius = 7.5;

  #ifdef GETTRACK_H_DEBUG
  TCanvas *track_base = new TCanvas("track base", "track base", 0, 0, 1200, 480);
  track_base->cd();
  Double_t center_x, center_y;
  Double_t track_corner_x[2] = {0, 800};
  Double_t track_corner_y[2] = {0, 320};
  TGraph * track_baseline = new TGraph(2, track_corner_x, track_corner_y);
  char track_name[256];
  sprintf(track_name, "selected_event");
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
  #endif


  // tracking start ////////////////////////////////////////////////////////////

  Int_t good_hit_count = 0;
  Int_t total_hit_count = 0;
  Double_t residual_sum = 0;
  Double_t residual_mean = 0;
  Bool_t layer_hit_record[8] = {kFALSE};
  for (Int_t track_iteration = 0; track_iteration != 1; track_iteration++) {
    // plot legendre curve, find the intersection
    Double_t theta, r, x_0, y_0; // parameters used in legendre curve
    Double_t fill_error;
    Double_t fill_weight;
    TH2F *plot_map = new TH2F("plot_map", "plot_map", 800, 0, 4, 180, -900, 900);
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      if (event_signal[signal_id][0] == 4 && in_range[signal_id] == true && output_good_hit_flag[signal_id] > 0) {
        GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0, 
                  &y_0);
        for (Int_t theta_id = 0; theta_id != 10000; theta_id++) {
          theta = M_PI * theta_id / 10000;
          fill_error = 250 - 18.75 * drift_distance[signal_id];
          // fill_weight = 1 / (fill_error * fill_error);
          fill_weight = 1;
          r = LegendreUpperCurve(theta, x_0, y_0, drift_distance[signal_id]);
          plot_map->Fill(theta, r, fill_weight);
          r = LegendreLowerCurve(theta, x_0, y_0, drift_distance[signal_id]);
          plot_map->Fill(theta, r, fill_weight);
        }
      }
    }
    Int_t max_bin_theta, max_bin_r, max_bin_z;
    plot_map->GetMaximumBin(max_bin_theta, max_bin_r, max_bin_z);
    if (verbose) cout << "max_bin_r: " << max_bin_r << " | max_bin_z: " << max_bin_z << endl;
    Double_t line_para_k, line_para_b; // line parameters for track line
    line_para_k = -1 / tan(max_bin_theta * 4.0 / 800.0);
    line_para_b = (-900 + max_bin_r * 1800 / 180.0) / 
                  sin(max_bin_theta * 4.0 / 800.0);

    
    if (verbose) cout << "line parameter b = " << line_para_b << endl 
		      << "line parameter k = " << line_para_k << endl;

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
      if (event_signal[signal_id][0] == 4 && in_range[signal_id] == true && output_good_hit_flag[signal_id] > 0) {
        for (Int_t theta_id = 0; theta_id != 10000; theta_id++) {
          theta = min_theta_limit + 
                  theta_id * (max_theta_limit - min_theta_limit) / 10000;
          fill_error = 250 - 18.75 * drift_distance[signal_id];
          // fill_weight = 1 / (fill_error * fill_error);
          fill_weight = 1;
          GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0,
                    &y_0);
          r = LegendreUpperCurve(theta, x_0, y_0, drift_distance[signal_id]);
          plot_map_accurate->Fill(theta, r, fill_weight);
          r = LegendreLowerCurve(theta, x_0, y_0, drift_distance[signal_id]);
          plot_map_accurate->Fill(theta, r, fill_weight);
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

    if (verbose) cout << "line_para_k = " << line_para_k << endl;
    if (verbose) cout << "line_para_b = " << line_para_b << endl;

    // use Principal Components Analysis (PCA) to fit final result track
    Double_t hit_position_x[100];
    Double_t hit_position_y[100];
    bool hit_position_acception[100];
    Double_t x_sum = 0;
    Double_t y_sum = 0;
    Double_t xy_sum = 0;
    Double_t xx_sum = 0;
    Double_t yy_sum = 0;
    Double_t quantity = 0;
    
    //// prepare dat for first run
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
      if (verbose) cout << "hit layer: " << hit_layer << " | hit column: " << hit_column << "  || x0: " << x0 << " | y0: " << y0;
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
      if (verbose) cout << " | selected hit x: " << hit_position_x[signal_id] << " | selected hit y: " << hit_position_y[signal_id] << endl;

      Double_t sum_weight;
      Double_t sum_error;
      if (fabs(GetPointLineDistance(hit_position_x[signal_id], hit_position_y[signal_id], line_para_k, line_para_b) - drift_distance[signal_id]) < 7.5 && event_signal[signal_id][0] == 4 && output_good_hit_flag[signal_id] > 0) {
        hit_position_acception[signal_id] = true;
        sum_error = 250 - 18.75 * drift_distance[signal_id];
        sum_weight = 1 / (sum_error * sum_error);
        quantity += 1.0 * sum_weight;
        x_sum += hit_position_x[signal_id] * sum_weight;
        y_sum += hit_position_y[signal_id] * sum_weight;
        xy_sum += hit_position_x[signal_id] * hit_position_y[signal_id] * sum_weight;
        xx_sum += hit_position_x[signal_id] * hit_position_x[signal_id] * sum_weight;
        yy_sum += hit_position_y[signal_id] * hit_position_y[signal_id] * sum_weight;
      } else {
        hit_position_acception[signal_id] = false;
      }
    }

    for (Int_t pca_iteration = 0; pca_iteration != 5; pca_iteration++) {
      Double_t x_mean = 0;
      Double_t y_mean = 0;
      Double_t xy_mean = 0;
      Double_t xx_mean = 0;
      Double_t yy_mean = 0;
      if (quantity != 0) {
        x_mean =  x_sum / quantity;
        y_mean =  y_sum / quantity;
        xy_mean =  xy_sum / quantity;
        xx_mean =  xx_sum / quantity;
        yy_mean = yy_sum / quantity;
      } else if (quantity == 0) {
        x_mean =  x_sum;
        y_mean =  y_sum;
        xy_mean =  xy_sum;
        xx_mean =  xx_sum;
        yy_mean = yy_sum;
      }
      
      Double_t xx_sum_centered = 0;
      Double_t xy_sum_centered = 0;
      Double_t yy_sum_centered = 0;
      for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
        Double_t sum_weight;
        Double_t sum_error;
        if (hit_position_acception[signal_id] == true) {
          sum_error = 250 - 18.75 * drift_distance[signal_id];
          sum_weight = 1 / (sum_error * sum_error) ;
          xx_sum_centered += (hit_position_x[signal_id] - x_mean) * (hit_position_x[signal_id] - x_mean) * sum_weight;
          xy_sum_centered += (hit_position_x[signal_id] - x_mean) * (hit_position_y[signal_id] - y_mean) * sum_weight;
          yy_sum_centered += (hit_position_y[signal_id] - y_mean) * (hit_position_y[signal_id] - y_mean) * sum_weight;
        }
      }
      Double_t xy_mean_centered = 0;
      Double_t xx_mean_centered = 0;
      Double_t yy_mean_centered = 0;
      if (quantity != 0) {
        xy_mean_centered = xy_sum_centered / quantity;
        xx_mean_centered = xx_sum_centered / quantity;
        yy_mean_centered = yy_sum_centered / quantity;
      } else if (quantity == 0) {
        xy_mean_centered = xy_sum_centered;
        xx_mean_centered = xx_sum_centered;
        yy_mean_centered = yy_sum_centered;
      }

      Double_t a, b, c, n1, n2;
      a = xx_mean_centered;
      b = xy_mean_centered;
      c = yy_mean_centered;
      if (verbose) cout << "a = " << a << " b = " << b << " c = " << c << endl; 
      Double_t eigen1, eigen2;
      eigen1 = 0.5 * (a + c - sqrt(a * a + 4 * b * b - 2 * a * c + c * c));
      eigen2 = 0.5 * (a + c + sqrt(a * a + 4 * b * b - 2 * a * c + c * c));
      Double_t eigen_vec1[2], eigen_vec2[2];
      eigen_vec1[0] = - (-a + c + sqrt(a * a + 4 * b * b - 2 * a * c + c * c)) / (2 * b);
      eigen_vec1[1] = 1;
      eigen_vec2[0] = - (-a + c - sqrt(a * a + 4 * b * b - 2 * a * c + c * c)) / (2 * b);
      eigen_vec2[1] = 1;
      if (eigen1 > eigen2) {
        n1 = eigen_vec2[0];
        n2 = eigen_vec2[1];
      } else {
        n1 = eigen_vec1[0];
        n2 = eigen_vec1[1];
      }
      *output_line_parameter_k = - n1 / n2;
      *output_line_parameter_b = y_mean - *output_line_parameter_k * x_mean;

      //// prepare data for next run
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
        if (verbose) cout << "hit layer: " << hit_layer << " | hit column: " << hit_column << "  || x0: " << x0 << " | y0: " << y0 << endl;
        possible_hit_position_x1 = x0 - drift_distance[signal_id] * (*output_line_parameter_k) / sqrt((*output_line_parameter_k) * (*output_line_parameter_k) + 1);
        possible_hit_position_x2 = x0 + drift_distance[signal_id] * (*output_line_parameter_k) / sqrt((*output_line_parameter_k) * (*output_line_parameter_k) + 1);
        possible_hit_position_y1 = y0 + drift_distance[signal_id] / sqrt((*output_line_parameter_k) * (*output_line_parameter_k) + 1);
        possible_hit_position_y2 = y0 - drift_distance[signal_id] / sqrt((*output_line_parameter_k) * (*output_line_parameter_k) + 1);

        if (GetPointLineDistance(possible_hit_position_x1, possible_hit_position_y1, (*output_line_parameter_k), (*output_line_parameter_b)) > GetPointLineDistance(possible_hit_position_x2, possible_hit_position_y2, (*output_line_parameter_k), (*output_line_parameter_b))) {
          hit_position_x[signal_id] = possible_hit_position_x2;
          hit_position_y[signal_id] = possible_hit_position_y2;
        } else if (GetPointLineDistance(possible_hit_position_x1, possible_hit_position_y1, (*output_line_parameter_k), (*output_line_parameter_b)) < GetPointLineDistance(possible_hit_position_x2, possible_hit_position_y2, (*output_line_parameter_k), (*output_line_parameter_b))) {
          hit_position_x[signal_id] = possible_hit_position_x1;
          hit_position_y[signal_id] = possible_hit_position_y1;
        }
        if (verbose) cout << " | selected hit x: " << hit_position_x[signal_id] << " | selected hit y: " << hit_position_y[signal_id] << endl;

        Double_t sum_weight;
        Double_t sum_error;
        if (fabs(GetPointLineDistance(hit_position_x[signal_id], hit_position_y[signal_id], (*output_line_parameter_k), (*output_line_parameter_b)) - drift_distance[signal_id]) < 7.5 && hit_position_acception[signal_id] == true) {
          hit_position_acception[signal_id] = true;
          sum_error = 250 - 18.75 * drift_distance[signal_id];
          sum_weight = 1 / (sum_error * sum_error);
          quantity += 1.0 * sum_weight;
          x_sum += hit_position_x[signal_id] * sum_weight;
          y_sum += hit_position_y[signal_id] * sum_weight;
          xy_sum += hit_position_x[signal_id] * hit_position_y[signal_id] * sum_weight;
          xx_sum += hit_position_x[signal_id] * hit_position_x[signal_id] * sum_weight;
          yy_sum += hit_position_y[signal_id] * hit_position_y[signal_id] * sum_weight;
        } else {
          hit_position_acception[signal_id] = false;
        }
      }
    }
    
    delete plot_map;
    delete plot_map_accurate;
    
    if (verbose) cout << "*output_line_parameter_k = " << *output_line_parameter_k << endl;
    if (verbose) cout << "*output_line_parameter_b = " << *output_line_parameter_b << endl;

    // calculate segment residual
    good_hit_count = 0;
    total_hit_count = 0;
    residual_sum = 0;
    residual_mean = 0;
    for (Int_t i = 0; i != 8; i++) {
      layer_hit_record[i] = kFALSE;
    }
    cout << endl << "// Segment Residual ///////////////////////" << endl;
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      if (event_signal[signal_id][0] == 4) {
        Double_t x_0, y_0, distance, residual;
        Int_t hit_layer, hit_column;
        GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x_0, 
                  &y_0);
        GetHitLayerColumn(event_signal[signal_id][1], event_signal[signal_id][2],
                          &hit_layer, &hit_column);
        distance = GetPointLineDistance(x_0, y_0, *output_line_parameter_k,
                                        *output_line_parameter_b);
        residual = drift_distance[signal_id] - distance;
        layer_hit_record[hit_layer] = kTRUE;

        if (fabs(residual) <= 0.005 * (250 - 18.75 * drift_distance[signal_id])) {
        //if (fabs(residual) <= 1) {
          good_hit_count++;
          output_good_hit_flag[signal_id] = static_cast<int>(drift_distance[signal_id]);
          output_residual[signal_id] = residual;
          residual_sum += residual;
        } else {
          output_good_hit_flag[signal_id] = -1;
        }
        total_hit_count++;

        cout << "hit layer: " << hit_layer << " | " << "hit column: " << hit_column << " | residual: " << residual << endl;
      } else {
        output_good_hit_flag[signal_id] = -1;
      }
    }
    residual_mean = residual_sum / good_hit_count;
    cout << "///////////////////////////////////////////" << endl;

    /*// float signal relative (drift) time
    Double_t float_value = 0;
    float_value = - 0.5 * residual_mean * 200 / 7.5;
    if (float_value > 1) {
      float_value = 1;
    } else if (float_value < -1) {
      float_value = -1;
    }

    for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
      drift_time[signal_id] = signal_time[signal_id] - trigger_time 
                              - t0_value[event_signal[signal_id][1]][event_signal[signal_id][2]] + float_value; 
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

    // update signal drift distance
    for (int signal_id = 0; signal_id < event_signal_length; signal_id++) {
      //drift_distance[signal_id] = RTFunction(drift_time[signal_id]);
      drift_distance[signal_id] = rt.GetRadius(drift_time[signal_id]);
    }*/
  }

  // tracking end //////////////////////////////////////////////////////////////

  #ifdef GETTRACK_H_DEBUG
  TLine *track_line_final = new TLine(0, *output_line_parameter_b, 900,
                                      *output_line_parameter_b + *output_line_parameter_k * 900);
  track_line_final->SetLineColor(kPink);
  track_line_final->Draw("same");

  track_base->cd();
  track_base->Update();
  track_base->Draw();
  #endif

  Bool_t pass_all_layers_flag = kTRUE;
  for (Int_t layer_id = 0; layer_id != 8; layer_id++) {
    if (layer_hit_record[layer_id] == kFALSE) {
      pass_all_layers_flag = kFALSE;
    }
  }
  // if good_hit_count lager than 4, consider this event has good track
  if (good_hit_count > 7 /*&& pass_all_layers_flag && (total_hit_count - good_hit_count) < 2*/) {
    #ifdef GETTRACK_H_DEBUG
    // debugging
    cout << "Paused. Enter 'e' to exit or other key to continue." << endl;
    if (getchar() == 'e') {
      cout << "Program is stopped by user." << endl;
      return 1;
    }
    delete track_base;
    #endif
    return true;
  } else {
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      output_good_hit_flag[signal_id] = -1;
    }
    #ifdef GETTRACK_H_DEBUG
    delete track_base;
    #endif
    return false;
  }

}
// end GetTrack ////////////////////////////////////////////////////////////////

Double_t GetPointLineDistance(Double_t x, Double_t y, Double_t k, Double_t b) {
  return fabs((k * x - y +b) / sqrt(k * k + 1.0));
}
