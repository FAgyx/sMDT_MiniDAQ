#include "src/TrackParam.cpp"
#include "src/Optimizer.cpp"
#include "src/Geometry.cpp"
#include "src/Event.cpp"
#include "src/Parameterization.cpp"
#include "src/EventDisplay.cpp"
#include "src/RTParam.cpp"

using namespace Muon;

void DrawRT(RTParam rtp) {
  double x[40];
  double y[40];
  for(double time = -200; time < 200; time+=10) {
    int index = (time + 200)/10;
    x[index]  = time;
    y[index]  = rtp.Eval(Hit(0,0,0,time,0,4));
  }

  TGraph* tg = new TGraph(40, x, y);
  tg->Draw();
  gPad->Modified();
  gPad->Update();
  getchar();
}

void testOpt() {
  int maxEntries = 20;


  // get data from one event and store in a new tree
  TFile  f("/atlas/data19/kevnels/sMDT/output/run00187982_20190603.dat.dir/run00187982_20190603.dat.out.root");
  TTree* t = (TTree*)f.Get("eTree");
  Event* e = new Event();
  t->SetBranchAddress("event", &e);

  Geometry   geo = Geometry();
  geo.SetRunN(187939);

  TString name;

  RTParam rtp = RTParam(geo);

  rtp.Print();
  rtp.Initialize("run00187982_20190603.dat");
  rtp.Print();

  TrackParam tp  = TrackParam(geo);
  tp.SetRT(&rtp);
  tp.setMaxResidual(Geometry::radius);

  EventDisplay ed = EventDisplay();
  ed.SetRT(&rtp);

  rtp.addDependency(&tp);
  rtp.setTarget(t);
  rtp.setRange(0,1000);
  rtp.setMaxResidual(Geometry::radius);
  rtp.Print();
  rtp.optimize();
  
  DrawRT(rtp);

}
