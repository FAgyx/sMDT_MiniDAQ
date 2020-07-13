#include "MuonReco/Geometry.h"

namespace MuonReco {
  Geometry::Geometry() {

    runN         = 0;
    TRIGGER_CH   = 0;
    TRIGGER_MEZZ = 0;

    for (int i = 0; i < Geometry::MAX_TDC_CHANNEL; i++) {
      hit_layer_map[i] = i%4;
      hit_column_map[i] = 5 - i/4;
    }
    for (int i = 0; i < Geometry::MAX_TDC; i++) {
      TDC_ML[i]  = 0;
      TDC_COL[i] = 0;
    }

    track_corner_x[0] = 0;
    track_corner_x[1] = 800;
    track_corner_y[0] = 0;
    track_corner_y[1] = 320;
    axes.push_back(new TGraph(2, track_corner_x, track_corner_y));

    // create the background of the event display: the outlines of the tubes
    for (Int_t layer_id = 0; layer_id != Geometry::MAX_TUBE_LAYER/2; layer_id++) {
      for (Int_t column_id = 0; column_id != Geometry::MAX_TUBE_COLUMN; column_id++) {
        center_x = radius + column_id * column_distance + ((layer_id + 1) % 2) * column_distance / 2.0;
        center_y = radius + layer_id * layer_distance;
        drawable.push_back(new TEllipse(center_x, center_y, radius, radius));

        if ((column_id / 6) % 2 == 0) {
          drawable.at(layer_id*Geometry::MAX_TUBE_COLUMN + column_id)->SetFillColor(kGray);
        }
      }
    }
    for (Int_t layer_id = Geometry::MAX_TUBE_LAYER/2; layer_id != Geometry::MAX_TUBE_LAYER; layer_id++) {
      for (Int_t column_id = 0; column_id != Geometry::MAX_TUBE_COLUMN; column_id++) {
        center_x = radius + column_id * column_distance + ((layer_id + 1) % 2) * column_distance / 2.0;
        center_y = radius + (layer_id - Geometry::MAX_TUBE_LAYER/2) * layer_distance + ML_distance;
        drawable.push_back(new TEllipse(center_x, center_y, radius, radius));

        if ((column_id / 6) % 2 == 0) {
          drawable.at(layer_id*Geometry::MAX_TUBE_COLUMN + column_id)->SetFillColor(kGray);
        }
      }
    }

    axes.at(0)->SetTitle("event");
    axes.at(0)->SetMarkerStyle(0);
  } // end method: Geometry initialization


  Geometry::~Geometry() {
    
  }

  void Geometry::Draw(int eventN) {
    TString title;
    title.Form("Event %d  Run %d", eventN, runN);
    axes.at(0)->SetTitle(title);
    axes.at(0)->Draw("AP");
    for (auto tube : drawable) tube->Draw();
  } // end method: Geometry :: Draw 

  void Geometry::Draw(int eventN, double xmin, double ymin, double xmax, double ymax, TString additionalText/*=""*/) {
    TString title;
    title.Form("Event %d  Run %d", eventN, runN);
    title += additionalText;
    double xlim[2];
    double ylim[2];
    
    xlim[0] = xmin;
    xlim[1] = xmax;
    ylim[0] = ymin;
    ylim[1] = ymax;

    if (axes.size() == 1)
      axes.push_back(new TGraph(2,xlim,ylim));
    else
      axes[1] = new TGraph(2,xlim,ylim);
    
    axes.at(1)->SetTitle(title);
    axes.at(1)->Draw("AP");

    for (auto tube : drawable) tube->Draw();
  }

  void Geometry::Draw(const char* title) {
    axes.at(0)->SetTitle(title);
    axes.at(0)->Draw("AP");
    for (auto tube : drawable) tube->Draw();
  }

  void Geometry::DrawTDCLabel() {
    TString str;
    double hitX, hitY;
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      if (IsActiveTDC(tdc) && tdc != TRIGGER_MEZZ) {
	GetHitXY(tdc, 0, &hitX, &hitY);
	str.Form("TDC %d", tdc);
	text.push_back(new TPaveText(hitX-9*radius, hitY+9*radius, hitX-3*radius, hitY+11*radius));
	text.back()->AddText(str.Data());
	text.back()->Draw();
      }
    }
  }

  void Geometry::ResetText() {
    
    return;
  }
  
  void Geometry::GetHitLayerColumn(unsigned int tdc_id, unsigned int channel_id, int *hit_layer, int *hit_column) const {
    *hit_column = 6*TDC_COL[tdc_id] + hit_column_map[channel_id];
    *hit_layer  = 4*TDC_ML[tdc_id]  + hit_layer_map[channel_id];
  }

  void Geometry::GetHitXY(int hitL, int hitC, double *hitX, double *hitY) {
    // get hit layer and column
    if (hitL < 0 || hitC < 0) {
      *hitX = -1;
      *hitY = -1;
    }
    *hitX = Geometry::radius + hitC * column_distance + ((hitL + 1) % 2) * column_distance / 2.0;
    *hitY = Geometry::radius + hitL * layer_distance  + (224.255-4*layer_distance)*(hitL >= 4);
  }    

  int Geometry::MultiLayer(Cluster c) const {
    return c.Hits().at(0).Layer()/MAX_TDC_LAYER;
  }

  int Geometry::MultiLayer(int layer) {
    return layer/MAX_TDC_LAYER;
  }
  

  Bool_t Geometry::AreAdjacent(double x1, double y1, double x2, double y2) {
    return TMath::Sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 1.5 * Geometry::layer_distance;
  }
  
  Bool_t Geometry::AreAdjacent(Cluster c1, Cluster c2) {
    for (Hit h1 : c1.Hits()) {
      for (Hit h2 : c2.Hits()) {
	if (AreAdjacent(h1.X(), h1.Y(), h2.X(), h2.Y()))
	  return kTRUE;	 	
      }
    }
    return kFALSE;
  }

  Bool_t Geometry::IsActiveTDC(unsigned int tdc) const {
    return isActiveTDC[tdc];
  }

  Bool_t Geometry::IsActiveTDCChannel(unsigned int tdc, unsigned int ch) const {
    if (tdc == TRIGGER_MEZZ)
      return 0;//ch == TRIGGER_CH;
    else
      return isActiveTDC[tdc];
  }

  int  Geometry::GetRunN() const {
    return runN;
  }

  void Geometry::SetRunN(int runNum) {
    runN = runNum;
    if (runN == 187691 || 
	runN == 187846) {
      TRIGGER_CH   = 23;
      TRIGGER_MEZZ =  1;
      
      TDC_ML[8]  = 0;
      TDC_ML[9]  = 1;
      TDC_ML[10] = 1;
      TDC_ML[11] = 0;

      TDC_COL[8]  = 5;
      TDC_COL[9]  = 5;
      TDC_COL[10] = 4;
      TDC_COL[11] = 4;
      
      isActiveTDC.reset();
      isActiveTDC[1]  = 1;
      isActiveTDC[8]  = 1;
      isActiveTDC[9]  = 1;
      isActiveTDC[10] = 1;
      isActiveTDC[11] = 1;
    }
    else if (runN == 187903) {
      TRIGGER_CH   = 23;
      TRIGGER_MEZZ =  5;
      
      TDC_ML[2]  = 0;
      TDC_ML[3]  = 0;
      TDC_ML[8]  = 1;
      TDC_ML[9]  = 1;
      TDC_ML[10] = 1;
      
      TDC_COL[2]  = 0;
      TDC_COL[3]  = 1;
      TDC_COL[8]  = 0;
      TDC_COL[9]  = 1;
      TDC_COL[10] = 2;
      
      isActiveTDC.reset();
      isActiveTDC[5]  = 1;
      isActiveTDC[2]  = 1;
      isActiveTDC[3]  = 1;
      isActiveTDC[9]  = 1;
      isActiveTDC[10] = 1;
    }
    else if (runN == 187939 || runN == 187982 || runN == 187984 ||
	     runN == 187985 || runN == 188734) {
      TRIGGER_CH   = 23;
      TRIGGER_MEZZ =  5;

      TDC_ML[0]  = 0;
      TDC_ML[2]  = 0;
      TDC_ML[8]  = 1;
      TDC_ML[9]  = 1;
      TDC_ML[10] = 1;

      TDC_COL[0]  = 1;
      TDC_COL[2]  = 2;
      TDC_COL[8]  = 0;
      TDC_COL[9]  = 1;
      TDC_COL[10] = 2;

      isActiveTDC.reset();
      isActiveTDC[5]  = 1;
      isActiveTDC[0]  = 1;
      isActiveTDC[2]  = 1;
      isActiveTDC[8]  = 1;
      isActiveTDC[9]  = 1;
      isActiveTDC[10] = 1;
    }
    else if (runN == 188192 || runN == 188193 || runN == 188194) {

      TDC_ML[0] = 0;
      TDC_ML[1] = 0;
      TDC_ML[2] = 0;
      TDC_ML[3] = 1;
      TDC_ML[4] = 1;
      TDC_ML[7] = 1;
      
      TDC_COL[0]  = 1;
      TDC_COL[1]  = 2;
      TDC_COL[2]  = 3;
      TDC_COL[3]  = 1;
      TDC_COL[4]  = 2;
      TDC_COL[7]  = 3;

      isActiveTDC[0] = 1;
      isActiveTDC[1] = 1;
      isActiveTDC[2] = 1;
      isActiveTDC[3] = 1;
      isActiveTDC[4] = 1;
      isActiveTDC[7] = 1;
    }
  } // end function: SetRunN

  void Geometry::Configure(ParameterSet ps) {
    isActiveTDC.reset();

    // get run number and trigger information
    runN         = ps.getInt("RunNumber");
    TRIGGER_MEZZ = ps.getInt("TriggerMezz");
    TRIGGER_CH   = ps.getInt("TriggerChannel");

    isActiveTDC[TRIGGER_MEZZ] = 1;


    std::vector<int> activeTDCs    = ps.getIntVector("ActiveTDCs");
    std::vector<int> TDCMultiLayer = ps.getIntVector("TDCMultilayer");
    std::vector<int> TDCColumn     = ps.getIntVector("TDCColumn");

    int nTDC = activeTDCs.size();
    nTDC = (nTDC < TDCMultiLayer.size()) ? nTDC : TDCMultiLayer.size();
    nTDC = (nTDC < TDCColumn.size())     ? nTDC : TDCColumn.size();

    int TDC;
    for (int i = 0; i < nTDC; i++) {
      TDC              = activeTDCs.at(i);
      TDC_ML     [TDC] = TDCMultiLayer.at(i);
      TDC_COL    [TDC] = TDCColumn.at(i);
      isActiveTDC[TDC] = 1;
    }
  }

}

