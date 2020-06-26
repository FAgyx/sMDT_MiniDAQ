/*******************************************************************************
  file name: PlotTrack.h
  author: Zhe Yang
  created: 02/09/2019
  last modified: 02/09/2019

  description:
  -Header for PlotTrack.cxx
*******************************************************************************/

#ifndef PLOTTRACK_H_
#define PLOTTRACK_H_

#include <string>
#include <math.h>

int PlotTrack(string s_input_filename = "Output/run00187691_20190301.dat.dir/run00187691_20190301.dat.out.root");

int PlotTrackFromData(unsigned long selected_event_id, TFile *p_input_rootfile, Int_t event_trigger_length, Int_t event_signal_length, Int_t event_trigger[100][5], Int_t event_signal[100][5]);

Double_t RTFunction(Double_t drift_time) {
  return drift_time * 0.0375; // linear approximation
}

Double_t LegendreUpperCurve(Double_t theta, Double_t x_0, Double_t y_0, 
                            Double_t r_0) {
  return x_0 * cos(theta) + y_0 * sin(theta) + r_0;
}

Double_t LegendreLowerCurve(Double_t theta, Double_t x_0, Double_t y_0, 
                            Double_t r_0) {
  return x_0 * cos(theta) + y_0 * sin(theta) - r_0;
}

Double_t FermiDiracFunction(Double_t *x, Double_t *par) {
  return (par[0] + par[3] * x[0]) / (1 + exp((par[1] - x[0]) / par[2])) + par[5];
}


#endif // PLOTTRACK_H_
