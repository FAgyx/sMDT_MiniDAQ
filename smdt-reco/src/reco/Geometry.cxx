#include "MuonReco/Geometry.h"

namespace MuonReco {
  
  Int_t Geometry::MAX_TDC         = 18;
  Int_t Geometry::MAX_TDC_CHANNEL = 24;
  Int_t Geometry::MAX_TUBE_LAYER  =  8;
  Int_t Geometry::MAX_TUBE_COLUMN = 54;
  Int_t Geometry::MAX_TDC_LAYER   =  4;
  Int_t Geometry::MAX_TDC_COLUMN  =  6;

  double Geometry::ML_distance    = 224.231;
  double Geometry::tube_length    = 1.1;

  Geometry::Geometry() {
    runN         = 0;
    TRIGGER_CH   = 0;
    TRIGGER_MEZZ = 0;

    isActiveTDC.resize(MAX_TDC);
    for (int i = 0; i < MAX_TDC; i++) {
      isActiveTDC[i] = 0;
    }

  }

  Geometry::Geometry(ConfigParser& cp) : Geometry() {
    Configure(cp.items("Geometry"));
  } // end method: Geometry initialization


  Geometry::~Geometry() {
    
  }

  void Geometry::Draw(int eventN) {
    axes.at(0)->SetTitle(";x [mm];z [mm]");
    axes.at(0)->Draw("AP");
    for (auto tube : drawable) tube->Draw();
  } // end method: Geometry :: Draw 

  void Geometry::Draw(int eventN, double xmin, double ymin, double xmax, double ymax, TString additionalText/*=""*/) {

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
    
    axes.at(1)->SetMarkerColor(kWhite);
    axes.at(1)->SetTitle(";x [mm];z [mm]");
    axes.at(1)->Draw("AP");

    for (auto tube : drawable) {
      if (tube->GetX1() > xmin && tube->GetX1() < xmax && tube->GetY1() > ymin && tube->GetY1() < ymax) 
	tube->Draw();
    }
  }

  void Geometry::Draw(const char* title) {
    axes.at(0)->SetTitle(title);
    axes.at(0)->Draw("AP");
    for (auto tube : drawable) tube->Draw();
  }

  void Geometry::DrawTDCLabel() {
    for (auto t    : text) {
      t->Draw();
    }
  }

  void Geometry::ResetText() {
    text.clear();
    TString str;
    double hitX, hitY;
    int hitL, hitC;
    int testChan = 0;
    if (flipTDCs) {
      if (!chamberType.CompareTo("C")) testChan = 23;
      else if (!chamberType.CompareTo("A")) testChan = 22;      
    }
    else  {
      if (!chamberType.CompareTo("C")) testChan = 0;
      else if (!chamberType.CompareTo("A")) testChan = 1;
    }

    for (unsigned int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      if (IsActiveTDC(tdc) && tdc != TRIGGER_MEZZ) {
	GetHitLayerColumn(tdc, testChan, &hitL, &hitC);
	GetHitXY(hitL, hitC, &hitX, &hitY);
	str.Form("TDC %d", tdc);
	text.push_back(new TPaveText(hitX-9*radius, hitY+4*layer_distance, hitX-3*radius, hitY+5*layer_distance));
	text.back()->AddText(str.Data());
      }
    }
  }
  
  void Geometry::GetHitLayerColumn(unsigned int tdc_id, unsigned int channel_id, int *hit_layer, int *hit_column) const {
    if (tdcColByTubeNo) 
      *hit_column = TDC_COL[tdc_id] + hit_column_map[channel_id];
    else
      *hit_column = 6*TDC_COL[tdc_id] + hit_column_map[channel_id];

    *hit_layer  = 4*TDC_ML[tdc_id]  + hit_layer_map[channel_id];
  }

  void Geometry::GetHitXY(int hitL, int hitC, double *hitX, double *hitY) {
    // get hit layer and column
    if (hitL < 0 || hitC < 0) {
      *hitX = -1;
      *hitY = -1;
      return;
    }
    if (!chamberType.CompareTo("C")) {
      *hitX = Geometry::radius + hitC * column_distance + ((hitL + 1) % 2) * column_distance / 2.0;
      *hitY = Geometry::radius + hitL * layer_distance  + (ML_distance-4*layer_distance)*(hitL >= 4);
    }
    else if (!chamberType.CompareTo("A")) {
      *hitX = Geometry::radius + hitC * column_distance + (hitL % 2) * column_distance / 2.0;
      *hitY = Geometry::radius + hitL * layer_distance  + (ML_distance-4*layer_distance)*(hitL >= 4);
    }

    if (MAX_TUBE_LAYER == layerOffset.size() && layerOffset.size() == layerSlope.size()) {
      *hitY = layerOffset.at(hitL) + layerSlope.at(hitL)*hitC;
    }

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
    else {
      int hitL, hitC;
      GetHitLayerColumn(tdc, ch, &hitL, &hitC);
      if (hitC < 0 || hitC >= MAX_TUBE_COLUMN) return 0;
      if (hitL < 0 || hitL >= MAX_TUBE_LAYER) return 0;
      return isActiveTDC[tdc];
    }
  }

  Bool_t Geometry::IsActiveLayerColumn(int layer, int column) const {
    int _tdc_layer, _tdc_column;
    Bool_t found = kFALSE;
    unsigned int iTDC;
    unsigned int iCh;
    for (iTDC = 0; iTDC < Geometry::MAX_TDC; iTDC++) {
      for (iCh = 0; iCh < Geometry::MAX_TDC_CHANNEL; iCh++) {
	GetHitLayerColumn(iTDC, iCh, &_tdc_layer, &_tdc_column);
	if (_tdc_layer == layer && _tdc_column == column) {
	  found = kTRUE;
	  break;
	}
      }
      if (found) break;
    }
    return IsActiveTDCChannel(iTDC, iCh);
  }

  int  Geometry::GetRunN() const {
    return runN;
  }

  void Geometry::SetRunN(int runNum) {
    runN = runNum;
  }

  void Geometry::Configure(ParameterSet ps) {
    Geometry::MAX_TDC         = ps.getInt("MAX_TDC",         18, 0);
    Geometry::MAX_TDC_CHANNEL = ps.getInt("MAX_TDC_CHANNEL", 24, 0);
    Geometry::MAX_TUBE_COLUMN = ps.getInt("MAX_TUBE_COLUMN", 54, 0);
    Geometry::MAX_TUBE_LAYER  = ps.getInt("MAX_TUBE_LAYER",  8,  0);
    Geometry::MAX_TDC_COLUMN  = ps.getInt("MAX_TDC_COLUMN",  6,  0);
    Geometry::MAX_TDC_LAYER   = ps.getInt("MAX_TDC_LAYER",   4,  0);

    Geometry::ML_distance     = ps.getDouble("ML_distance", 224.231, 0);
    Geometry::tube_length     = ps.getDouble("TubeLength",  1.5,     0);


    // get run number and trigger information
    runN           = ps.getInt("RunNumber");
    TRIGGER_MEZZ   = ps.getInt("TriggerMezz");
    TRIGGER_CH     = ps.getInt("TriggerChannel");
    chamberType    = ps.getStr("ChamberType", "C");
    tdcColByTubeNo = ps.getInt("TdcColByTubeNo", 0, 0);
    flipTDCs       = ps.getInt("FlipTDCs", 0, 0);
    /*
    TString flip   = ps.getStr("FlipTDCs", "0");
    TDC_FLIP.resize(Geometry::MAX_TDC);
    if (flip.Contains(":")) {
      int TDC;
      std::vector<int> activeTDCs = ps.getIntVector("ActiveTDCs");
      std::vector<int> tdc_flip = ps.getIntVector("FlipTDCs");
      for (size_t i = 0; i<tdc_flip.size(); ++i) {
	TDC         = activeTDCs.at(i);
	TDC_FLIP[TDC] = tdc_flip.at(i);
      }
      flipTDCs = TDC_FLIP[0];
    }
    else {
      flipTDCs       = ps.getInt("FlipTDCs", 0, 0);
      for (size_t i = 0; i<TDC_FLIP.size(); ++i) {
	TDC_FLIP[i] = flipTDCs;
      }
    }
    */
    
    isActiveTDC.resize(MAX_TDC);
    isActiveTDC[TRIGGER_MEZZ] = 1;

    TDC_ML .resize(Geometry::MAX_TDC);
    TDC_COL.resize(Geometry::MAX_TDC);
    hit_layer_map.resize(Geometry::MAX_TDC_CHANNEL);
    hit_column_map.resize(Geometry::MAX_TDC_CHANNEL);

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


    if (ps.hasKey("LayerOffset")) {
      layerOffset = ps.getDoubleVector("LayerOffset");
    }
    if (ps.hasKey("LayerSlope")) {
      layerSlope = ps.getDoubleVector("LayerSlope");
    }


    ResetTubeLayout();
  }

  void Geometry::ResetTubeLayout() {
    for (int i = 0; i < Geometry::MAX_TDC_CHANNEL; i++) {
      if (!chamberType.CompareTo("C")) {
	if (flipTDCs) {
	  hit_layer_map[i] = 3-(i%4);
          hit_column_map[i] = i/4;
	}
	else {
	  hit_layer_map[i] = i%4;
	  hit_column_map[i] = 5 - i/4;
	}
      }
      else if (!chamberType.CompareTo("A")) {
	if (flipTDCs) {
	  hit_layer_map[i] = (i+2)%4;
          hit_column_map[i] = i/4;
	}
	else {
	  hit_column_map[i] = 5 - i/4;
	  hit_layer_map[i] = (i%2) ? (i%4)-1 : (i%4)+1;
	}
      }
    }

    double hitX, hitY;
    GetHitXY(MAX_TUBE_LAYER-1, MAX_TUBE_COLUMN-1, &hitX, &hitY);
    track_corner_x[0] = 0;
    track_corner_x[1] = hitX+radius*1.1;
    track_corner_y[0] = 0;
    track_corner_y[1] = hitY+radius*1.1;
    axes.clear();
    axes.push_back(new TGraph(2, track_corner_x, track_corner_y));

    drawable.clear();
    // create the background of the event display: the outlines of the tubes
    for (Int_t layer_id = 0; layer_id != Geometry::MAX_TUBE_LAYER; layer_id++) {
      for (Int_t column_id = 0; column_id != Geometry::MAX_TUBE_COLUMN; column_id++) {
	GetHitXY(layer_id, column_id, &center_x, &center_y);
        drawable.push_back(new TEllipse(center_x, center_y, radius, radius));

        if ((column_id / 6) % 2 == 0) {
          drawable.at(layer_id*Geometry::MAX_TUBE_COLUMN + column_id)->SetFillColor(kGray);
        }
      }
    }

    axes.at(0)->SetTitle("event");
    axes.at(0)->SetMarkerStyle(0);
    ResetText();
  }

  double Geometry::getMeanYPosition() {
    return (ML_distance + 2.0*radius + layer_distance*(MAX_TDC_LAYER-1))/2.0;
  }

}

