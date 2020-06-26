#include "src/TrackParam.cpp"
#include "src/Optimizer.cpp"
#include "src/Geometry.cpp"
#include "src/Event.cpp"
#include "src/Parameterization.cpp"
#include "src/EventDisplay.cpp"
#include "src/RTParam.cpp"
#include "src/IOUtility.cpp"

using namespace Muon;

void showTracks(TString inputFileName = "run00187985_20190607.dat") {
  int maxEntries;


  // get data from one event and store in a new tree
  TFile  f("/atlas/data19/kevnels/sMDT/output/run00187985_20190607.dat.dir/run00187985_20190607.dat.out.root");
  TTree* t = (TTree*)f.Get("eTree");
  Event* e = new Event();
  t->SetBranchAddress("event", &e);

  Geometry   geo = Geometry();
  geo.SetRunN(187985);


  RTParam rtp = RTParam(geo);
  rtp.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp.LoadTxt("src/Rt_BMG_6_1.dat");
  
  TrackParam tp  = TrackParam(geo);
  tp.SetRT(&rtp);
  tp.setMaxResidual(10.0);
  tp.setVerbose(kTRUE);

  EventDisplay ed = EventDisplay();
  ed.SetRT(&rtp);
  ed.Divide       (2, 2);
  
  chdir("output");
  chdir("run00187985_20190607.dat.dir");
  system("mkdir tracks_pass_chiSq");
  system("mkdir tracks_fail_chiSq");
  TDirectory* outdirPass = new TDirectory("tracks_pass_chiSq", "tracks_pass_chiSq");
  TDirectory* outdirFail = new TDirectory("tracks_fail_chiSq", "tracks_fail_chiSq");
  int npass = 0;
  int nfail = 0;
  TString name;

  
  //ResolutionResult* rr = new ResolutionResult();
  //tp.setMaxResidual(2*TMath::Sqrt(rr->chiSqCut*.01));

  //maxEntries = t->GetEntries();
  maxEntries = 1;

  for (int i = 0; i < maxEntries; i++) {

    tp.setTarget(t);
    tp.setRangeSingle(i);
    tp.optimize();


    if (npass >= 100 && nfail >= 100) break;

    if (npass < 100 && tp.getChiSqNDF() < 2) {
      t->SetBranchAddress("event", &e);
      t->GetEntry(i);
      Event event = *e;
      event.AddTrack(Track(tp.slope(), tp.y_int()));
      
      ed.DrawEvent    (event, geo);
      ed.DrawTrackZoom(event, geo, 0);
      ed.DrawTrackZoom(event, geo, 1);
      name.Form       ("tracks_pass_chiSq/event_%lu.png", event.ID());
      ed.SaveCanvas   (name, outdirPass);
      ed.Clear        ();
      npass++;
    }
    else if (nfail < 100 && tp.getChiSqNDF() > 2) {
      t->SetBranchAddress("event", &e);
      t->GetEntry(i);
      Event event = *e;
      event.AddTrack(Track(tp.slope(), tp.y_int()));

      ed.DrawEvent    (event, geo);
      ed.DrawTrackZoom(event, geo, 0);
      ed.DrawTrackZoom(event, geo, 1);
      name.Form       ("tracks_fail_chiSq/event_%lu.png", event.ID());
      ed.SaveCanvas   (name, outdirFail);
      ed.Clear        ();
      nfail++;
    }
  }
  getchar();
}
