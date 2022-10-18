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

  /*! \class EventDisplay EventDisplay.h "MuonReco/EventDisplay.h"
   *
   * \brief 2D event display for sMDT project
   *
   * Will preferentially draw highest level reconstructed object is in 
   * the Event object (Signals, Hits, Clusters, Tracks) if they conflict
   * (i.e. A Hit will be drawn as a part of a cluster, not an individual hit)
   * 
   * Provides function DrawTubeHist(), which allows a 2D histogram to be plotted
   * over the tube geometry (useful for data visualization, since the rectangular
   * 2D histogram does not properly represent the adjacency of tubes)
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date 18 May 2020
   */
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
    void SetRT        (Callable* rtp);
    void SetOutputDir (TString dir);

  private:
    TCanvas*               eCanv;
    std::vector<TEllipse*> hit_model;
    std::vector<TBox*>     boxes;
    std::vector<TLine*>    track_model;
    Callable*              rtfunction = 0;

    void              DrawTDCBorders();
    
    TString _dir = ".";
  };
}
#endif
