#include "src/Geometry.cpp"
#include "macros/AtlasStyle/AtlasLabels.C"
#include "macros/AtlasStyle/AtlasStyle.C"
#include "macros/AtlasStyle/AtlasUtils.C"
#include "src/EventDisplay.cpp"

using namespace MuonReco;

void plotActive(TString inputFilename = "run00187985_20190607.dat") {
  SetAtlasStyle();
  Geometry geo = Geometry();

  TString fn = TString(inputFilename);
  int runN = ((TObjString*)(TString(fn(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  geo.SetRunN(runN);

  static EventDisplay ed = EventDisplay();

  TH2D* isActive = new TH2D("isActive",    "T0 vs Tube position", Geometry::MAX_TUBE_COLUMN, -0.5, Geometry::MAX_TUBE_COLUMN-0.5,Geometry::MAX_TUBE_LAYER, -0.5, Geometry::MAX_TUBE_LAYER-0.5);
  int hitL, hitC;
  for (int TDC = 0; TDC < Geometry::MAX_TDC; TDC++) {
    for (int Chan = 0; Chan < Geometry::MAX_TDC_CHANNEL; Chan++) {
      geo.GetHitLayerColumn(TDC, Chan, &hitL, &hitC);
      isActive->SetBinContent(hitC, hitL, geo.IsActiveTDC(TDC));
    }
  }
  ed.DrawTubeHist(geo, isActive);
}
