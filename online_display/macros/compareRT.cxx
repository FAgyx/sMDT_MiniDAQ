#include "src/Geometry.cpp"
#include "src/RTParam.cpp"

using namespace Muon;

void compareRT() {

  gROOT->SetBatch(kTRUE);
  Geometry geo = Geometry();
  geo.SetRunN(187985);

  RTParam rtp = RTParam(geo);

  TFile* infile = new TFile("output/run00187985_20190607.dat.dir/autoCalibratedRT.root");
  rtp.Initialize("run00187985_20190607.dat");
  //rtp.Load(infile);
  //rtp.SaveImage("rtfunction.png");

  RTParam mdt = RTParam(geo);
  //mdt.Initialize("run00187985_20190607.dat");
  mdt.LoadTxt("src/Rt_BMG_6_1.dat");
  mdt.SaveImage("rtfunction.png");

  TF1* RTdifference = rtp.RTDifference(&mdt);
  TF1* oldResult = new TF1("oldResult", "240-20*x", 0, 7);
  TF1* newResult = new TF1("newResult", "256-81*x+8.56*x*x", 0, 7);
  TF1* r_difference = new TF1("r_difference", "oldResult-newResult", 0, 7);
  
  cout << r_difference->Eval(3) << endl;
  gROOT->SetBatch(kFALSE);


  int numPts = 100;
  double deltaRes[numPts];
  double deltaRT [numPts];
  double xval = 0;
  for (int i = 0; i < numPts; i++) {
    deltaRes[i] = r_difference->Eval(xval);
    deltaRT [i] = RTdifference->Eval((xval*2/7)-1);
    xval += 0.07;
  }

  TGraph* graph = new TGraph(numPts, deltaRT, deltaRes);
  
  
  TCanvas* compare = new TCanvas("compare", "Compare old and new result");
  compare->cd();
  graph->SetTitle("Correlation between difference in RT and resolution improvement");
  graph->GetXaxis()->SetTitle("#Delta RT function [#mu m]");
  graph->GetYaxis()->SetTitle("#Delta Resolution Result [#mu m]");
  graph->Draw();

  /*
  oldResult->SetLineColor(40);
  oldResult->Draw();
  newResult->SetLineColor(kRed);
  newResult->Draw("same");
  */
}
