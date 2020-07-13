#ifndef MUON_EVENT_DISPLAY
#define MUON_EVENT_DISPLAY

#include <bitset>
#include <vector>
#include <iostream>

#include "TCanvas.h"
#include "TEllipse.h"
#include "TBox.h"
#include "TLine.h"
#include "TDirectory.h"
#include "TH1D.h"
#include "TH2D.h"

#include "MuonReco/Event.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/RTParam.h"

namespace MuonReco {

  class EventDisplay {

  public:
    EventDisplay();
    ~EventDisplay();

    void Divide       (int nX, int nY);
    void DrawEvent    (Event &e, Geometry &geo, TDirectory* outdir=NULL);
    void DrawTubeHist (Geometry &geo, TH2D* hist, TDirectory* outdir=NULL, bool noROOT=kFALSE);
    void DrawTrackZoom(Event &e, Geometry &geo, int ML, TDirectory* outdir=NULL);
    void SaveCanvas   (TString name, TDirectory* outdir);
    void Clear        ();
    void SetRT        (RTParam* rtp);

  private:
    TCanvas*               eCanv;
    std::vector<TEllipse*> hit_model;
    std::vector<TBox*>     boxes;
    std::vector<TLine*>    track_model;
    RTParam*               rtfunction = 0;

    void              DrawTDCBorders();
  };
}
#endif
