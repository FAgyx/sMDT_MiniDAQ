#ifndef MUON_GEOMETRY
#define MUON_GEOMETRY

#include <bitset>
#include <vector>

#include "TGraph.h"
#include "TPaveText.h"
#include "TEllipse.h"
#include "TMath.h"

#include "MuonReco/Cluster.h"
#include "MuonReco/ParameterSet.h"

namespace MuonReco {
  class Geometry {
  public:
    Geometry();
    ~Geometry();

    void   Draw              (int eventN);
    void   Draw              (int eventN, double xmin, double ymin, double xmax, double ymax, TString additionalText="");
    void   Draw              (const char * title);
    void   DrawTDCLabel      ();
    void   ResetText         ();
    void   GetHitLayerColumn (unsigned int tdc_id, unsigned int channel_id, int *hit_layer, int *hit_column) const;
    void   SetRunN           (int runNum);
    bool   IsActiveTDC       (unsigned int tdc) const;
    bool   IsActiveTDCChannel(unsigned int tdc, unsigned int ch) const;
    int    MultiLayer        (Cluster c) const;

    int    GetRunN           () const;
    void   Configure         (ParameterSet ps);

    static void   GetHitXY   (int hitL, int hitC, double *hitX, double *hitY);
    static bool   AreAdjacent(double x1, double x2, double y1, double y2);
    static bool   AreAdjacent(Cluster c1, Cluster c2);
    static int    MultiLayer (int layer);

    static const Int_t MAX_TDC         = 18;
    static const Int_t MAX_TDC_CHANNEL = 24;
    static const Int_t MAX_TUBE_LAYER  =  8;
    static const Int_t MAX_TUBE_COLUMN = 54;
    static const Int_t MAX_TDC_COLUMN  =  6;
    static const Int_t MAX_TDC_LAYER   =  4;

    static constexpr double layer_distance  = 13.0769836;
    static constexpr double column_distance = 15.1;
    static constexpr double radius          = 7.5;
    static constexpr double ML_distance     = 224.231;
    static constexpr double min_drift_dist  = 0.0;
    static constexpr double max_drift_dist  = 7.1;

    int hit_column_map[Geometry::MAX_TDC_CHANNEL];
    int hit_layer_map [Geometry::MAX_TDC_CHANNEL];
    int runN;            // configurable on the fly
    short TRIGGER_CH;
    short TRIGGER_MEZZ;
    short TDC_ML [Geometry::MAX_TDC];
    short TDC_COL[Geometry::MAX_TDC];
    
  private:
    std::bitset<Geometry::MAX_TDC> isActiveTDC;

    double center_x, center_y;
    double track_corner_x[2];
    double track_corner_y[2];

    std::vector<TEllipse*> drawable;
    std::vector<TGraph*>    axes;
    std::vector<TPaveText*> text;
  };
    
}
#endif
