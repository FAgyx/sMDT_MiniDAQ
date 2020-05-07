/*******************************************************************************
  file name: GetTrackFast.h
  author: Zhe Yang
  created: 02/09/2019
  last modified: 03/02/2019

  description:
  -Construct track fast to to distinguish noise from signal 

  reference:
  -Developed for using in DecodeRawData.cxx

*******************************************************************************/

#ifndef GETTRACKFAST_H_
#define GETTRACKFAST_H_

#include "src/PlotTrack.h"

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

using namespace std;

Double_t GetPointLineDistance(Double_t x, Double_t y, Double_t k, Double_t b);

Int_t GetTrackFast(unsigned int event_signal[128][5],
                   unsigned int event_signal_length, 
                   bool signal_flag[128]) {
  //linear fitting
  Double_t hit_position_x[128];
  Double_t hit_position_y[128];
  Double_t x_sum = 0;
  Double_t y_sum = 0;
  Double_t xy_sum = 0;
  Double_t xx_sum = 0;
  Double_t hit_quantity = 0;
  Double_t possible_hit_position_x1 = 0;
  Double_t possible_hit_position_x2 = 0;
  Double_t possible_hit_position_y1 = 0;
  Double_t possible_hit_position_y2 = 0;
  Double_t x0 = 0, y0 = 0;
  Int_t hit_layer = 0, hit_column = 0;
  for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
    GetHitInfo(event_signal[signal_id][1], event_signal[signal_id][2], &x0, 
               &y0);
    hit_position_x[signal_id] = x0;
    hit_position_y[signal_id] = y0;
    GetHitLayerColumn(event_signal[signal_id][1], event_signal[signal_id][2],
                      &hit_layer, &hit_column);
    if (event_signal[signal_id][0] == 4) {
      x_sum += hit_position_x[signal_id];
      y_sum += hit_position_y[signal_id];
      xy_sum += hit_position_x[signal_id] * hit_position_y[signal_id];
      xx_sum += hit_position_x[signal_id] * hit_position_x[signal_id];
      hit_quantity++;
    }
  }

  Double_t x_mean = 0;
  Double_t y_mean = 0;
  Double_t xy_mean = 0;
  Double_t xx_mean = 0;
  if (hit_quantity != 0) {
    x_mean =  x_sum / hit_quantity;
    y_mean =  y_sum / hit_quantity;
    xy_mean =  xy_sum / hit_quantity;
    xx_mean =  xx_sum / hit_quantity;
  } else if (hit_quantity == 0) {
    x_mean =  x_sum;
    y_mean =  y_sum;
    xy_mean =  xy_sum;
    xx_mean =  xx_sum;
  }

  Double_t line_para_k = (xy_mean - x_mean * y_mean) / 
                         (xx_mean - x_mean * x_mean);
  Double_t line_para_b = y_mean - line_para_k * x_mean;
  /*
  cout << "////////////// track line parameters /////////////" << endl
       << "x_mean = " << x_mean << endl
       << "y_mean = " << y_mean << endl
       << "xy_mean = " << xy_mean << endl
       << "xx_mean = " << xx_mean << endl
       << "hit quantity = " << hit_quantity << endl
       << "k = " << line_para_k << endl
       << "b = " << line_para_b << endl
       << "////////////// track line parameters /////////////" << endl
       << endl;
  */
  if (hit_quantity != 0) {
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      if (event_signal[signal_id][0] == 4 && 
          GetPointLineDistance(hit_position_x[signal_id],
                               hit_position_y[signal_id], line_para_k, line_para_b) < 15) {
        signal_flag[signal_id] = true;
      } else {
        signal_flag[signal_id] = false;
      }
    }
  } else if (hit_quantity == 0) {
    for (Int_t signal_id = 0; signal_id < event_signal_length; signal_id++) {
      signal_flag[signal_id] = false;
    }
  }

  return 0;
}

Double_t GetPointLineDistance(Double_t x, Double_t y, Double_t k, Double_t b) {
  return fabs((k * x - y +b) / sqrt(k * k + 1.0));
}

#endif // GETTRACKFAST_H_
