#include "src/TrackParam.cpp"
#include "src/Optimizer.cpp"
#include "src/Geometry.cpp"
#include "src/Event.cpp"
#include "src/Parameterization.cpp"
#include "src/EventDisplay.cpp"
#include "src/RTParam.cpp"

using namespace MuonReco;

void patternRecog() {
  int maxEntries = 30;


  // get data from one event and store in a new tree
  TFile  f("/atlas/data19/kevnels/sMDT/output/run00187985_20190607.dat.dir/run00187985_20190607.dat.out.root");
  TTree* t = (TTree*)f.Get("eTree");
  Event* e = new Event();
  t->SetBranchAddress("event", &e);

  Geometry   geo = Geometry();
  geo.SetRunN(187985);


  RTParam rtp = RTParam(geo);
  rtp.Initialize("run00187985_20190607.dat");
  
  TrackParam tp  = TrackParam(geo);
  tp.SetRT(&rtp);
  tp.setMaxResidual(3*Geometry::radius);
  tp.setVerbose(kFALSE);

  EventDisplay ed = EventDisplay();
  ed.SetRT(&rtp);
  ed.Divide       (2, 2);
  
  chdir("output");
  chdir("run00187985_20190607.dat.dir");
  system("mkdir pattern_recognition");
  TDirectory* outdir = new TDirectory("pattern_recognition", "pattern_recognition");
  int num = 0;
  TString name;

  
  maxEntries = t->GetEntries();

  for (int i = 0; i < maxEntries; i++) {
    t->SetBranchAddress("event", &e);
    t->GetEntry(i);
    tp.Initialize(e);

    if (num >= 100) break;

    e->AddTrack(Track(tp.slope(), tp.y_int()));
    
    ed.DrawEvent    (*e, geo);
    ed.DrawTrackZoom(*e, geo, 0);
    ed.DrawTrackZoom(*e, geo, 1);
    name.Form       ("pattern_recognition/event_%lu.png", e->ID());
    ed.SaveCanvas   (name, outdir);
    ed.Clear        ();
    num++;
  }
  

}
