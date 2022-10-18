#include "TFile.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TLine.h"
#include "TGaxis.h"
#include "TPaletteAxis.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "MuonReco/ConfigParser.h"
#include "MuonReco/RTParam.h"
#include "MuonReco/T0Fit.h"
#include "MuonReco/ResolutionResult.h"

#include "MuonSim/GasMonitorRTStrategy.h"

using namespace MuonReco;
using namespace MuonSim;

void scatteringAngle() {
  TFile* f_4GeV   = TFile::Open("run1/Events.root");
  TFile* f_20GeV  = TFile::Open("run2/Events.root");
  TFile* f_0p5GeV = TFile::Open("run3/Events.root");
  
  TH1D* h_4GeV=0;
  TH1D* h_20GeV=0;
  TH1D* h_0p5GeV=0;

  f_4GeV  ->GetObject("DeltaPhi", h_4GeV);
  f_20GeV ->GetObject("DeltaPhi", h_20GeV);
  f_0p5GeV->GetObject("DeltaPhi", h_0p5GeV);

  h_4GeV->SetLineColor(kRed);
  h_4GeV->SetFillColor(kRed);
  h_4GeV->SetFillStyle(3005);
  h_20GeV->SetLineColor(kBlue);
  h_20GeV->SetFillColor(kBlue);
  h_20GeV->SetFillStyle(3004);
  h_20GeV->SetStats(0);
  h_0p5GeV->SetFillColor(kBlack);

  TCanvas* c1 = new TCanvas("c1", "", 800, 600);
  c1->cd();

  h_20GeV->Draw("C");
  h_4GeV->Draw("same C");
  h_0p5GeV->Draw("same C");

  TLegend* leg = new TLegend(0.6,0.7,0.9,0.9);
  leg->AddEntry(h_4GeV,   "4 GeV",   "le");
  leg->AddEntry(h_20GeV,  "20 GeV",  "le");
  leg->AddEntry(h_0p5GeV, "0.5 GeV", "le");
  leg->Draw();
}

void resolutionSmearing() {
  TFile* f_4GeV   = TFile::Open("run1/fitResiduals.root");
  TFile* f_20GeV  = TFile::Open("run2/fitResiduals.root");
  TFile* f_0p5GeV = TFile::Open("run3/fitResiduals.root");
  
  TH1D* h_4GeV=0;
  TH1D* h_20GeV=0;
  TH1D* h_0p5GeV=0;

  f_4GeV  ->GetObject("RR_residuals", h_4GeV);
  f_20GeV ->GetObject("RR_residuals", h_20GeV);
  f_0p5GeV->GetObject("RR_residuals", h_0p5GeV);

  h_4GeV->SetLineColor(kRed);
  h_20GeV->SetLineColor(kBlue);
  h_20GeV->SetStats(0);
  h_4GeV->SetStats(0);
  h_0p5GeV->SetStats(0);

  h_4GeV->GetFunction("doubGaus")->SetBit(TF1::kNotDraw);
  h_20GeV->GetFunction("doubGaus")->SetBit(TF1::kNotDraw);
  h_0p5GeV->GetFunction("doubGaus")->SetBit(TF1::kNotDraw);

  TCanvas* c1 = new TCanvas("c1", "", 800, 600);
  c1->cd();

  h_20GeV->Draw();
  h_4GeV->Draw("same");
  h_0p5GeV->Draw("same");

  TLegend* leg = new TLegend(0.6,0.7,0.9,0.9);
  leg->AddEntry(h_4GeV,   "4 GeV",   "le");
  leg->AddEntry(h_20GeV,  "20 GeV",  "le");
  leg->AddEntry(h_0p5GeV, "0.5 GeV", "le");
  leg->Draw();

}

void truthRT() {
  ConfigParser cp = ConfigParser("conf/mc_run009.conf");
  int runN = cp.items("General").getInt("RunNumber");
  int minEvent    = cp.items("AutoCalibration").getInt("MinEvent", 0, 0);
  int nEvents     = cp.items("AutoCalibration").getInt("NEvents",  100000, 0);

  // get the truth rt function
  GasMonitorRTStrategy rts = GasMonitorRTStrategy(cp);

  // get the auto calibrated rt function
  TFile autocal(IOUtility::getAutoCalFilePath(runN, minEvent, nEvents));
  RTParam rtp = RTParam(cp);
  rtp.Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
  //rtp.Load(&autocal);
  rtp.LoadTxt("raw/Rt_BMG_6_1.dat");

  // make graph showing delta
  double _r, _t, _rAC;
  std::vector<double> rAC;
  std::vector<double> time;
  for (int i = 0; i < rts.rtgraph->GetN(); i++) {
    rts.rtgraph->GetPoint(i, _r, _t);
    _rAC = rtp.Eval(_t/188.1688*2. -1.);
    rAC.push_back(_r - _rAC);
    time.push_back(_t);
    std::cout << "time: " << _t << std::endl;
    std::cout << "radi: " << _rAC << std::endl;
    std::cout << "delt: " << _r - _rAC << std::endl;
    std::cout << std::endl;
  }

  TGraph* gr = new TGraph(rAC.size(), &time[0], &rAC[0]);

  // make plots

  TCanvas* c1 = new TCanvas("c1", "c1", 800, 600);
  c1->cd();  
  gr->Draw();
  
  //rtp.Draw();
  //rts.rtgraph->Draw("same");

  c1->Print("output.png");
}

void T0plots() {
  struct bins {
    int n;
    double low;
    double high;
    bins() : n(100), low(0), high(100) {}
    bins(int _n, double _low, double _high) : n(_n), low(_low), high(_high) {}
    bins(const bins& other) {
      this->n = other.n;
      this->low = other.low;
      this->high = other.high;
    }
  } fitDataBins[NT0FITDATA];
  fitDataBins[0]  = bins();
  fitDataBins[1]  = bins(60, -200, 0);
  fitDataBins[3]  = bins(60, 0, 15);
  fitDataBins[7]  = bins(60, 0, 5);
  fitDataBins[8]  = bins(60, 120, 300);
  fitDataBins[10] = bins(60, 0, 15);
  fitDataBins[12] = bins(60, 0, 5);
  fitDataBins[13] = bins(60, 80, 200);
  fitDataBins[15] = bins(60, 0, 200);
  fitDataBins[17] = bins(60, 0, 5);
  fitDataBins[19] = bins(60, 0, 5);

  for (int iParam = 0; iParam < NT0FITDATA; iParam++) {
    std::cout << "Plotting " << fitDataNames[iParam] << std::endl;

    std::vector<int> v_runN = std::vector<int>();
    //v_runN.push_back(188487);
    //v_runN.push_back(188512);
    //v_runN.push_back(188566); // good data from prototype chamber
    //v_runN.push_back(190880); // ASD 2 from prototype chamber
    //v_runN.push_back(191129);
    //v_runN.push_back(191375);
    v_runN.push_back(191436);
    v_runN.push_back(191437);
    
    std::vector<int> v_colors = std::vector<int>();
    v_colors.push_back(1);
    v_colors.push_back(2);
    v_colors.push_back(4);
    
    std::vector<TH1D*> v_fitparam = std::vector<TH1D*>();
    
    
    for (int iRunN = 0; iRunN < v_runN.size(); iRunN++) {
      int runN = v_runN.at(iRunN);
      v_fitparam.push_back(new TH1D(TString::Format("h_" + fitDataNames[iParam] + "_%d", runN), fitDataNames[iParam], fitDataBins[iParam].n, fitDataBins[iParam].low, fitDataBins[iParam].high));
      
      std::cout << "reading run " << runN << std::endl;
      T0Reader* t0reader = T0Reader::GetInstance(IOUtility::getT0FilePath(runN));
      int tdc, ch, layer, col;
      TVectorD* fitParams = new TVectorD(NT0FITDATA);
      t0reader->SetBranchAddresses(&tdc, &ch, &layer, &col, fitParams);
      for (int iEntry = 0; iEntry < t0reader->GetEntries(); iEntry++) {
	t0reader->GetEntry(iEntry);
	if (ch<0) continue;
	v_fitparam.at(v_fitparam.size()-1)->Fill(fitParams[0][iParam]);
      }
    }
    
    TCanvas* c_fitparam = new TCanvas("c_" + fitDataNames[iParam], "c_" + fitDataNames[iParam], 800, 800);
    TLegend* l_fitparam = new TLegend(0.6,0.7,0.9,0.9);
    c_fitparam->cd();
    for (int iHist = 0; iHist < v_fitparam.size(); iHist++) {
      TH1D* hist = v_fitparam.at(iHist);
      hist->SetStats(0);
      hist->SetLineColor(v_colors.at(iHist));
      hist->SetLineWidth(3);
      std::cout << "Plotting " << hist->GetName() << std::endl;
      
      if (iHist) 
	hist->Draw("same");
      else {
	hist->GetXaxis()->SetTitle(fitDataNames[iParam] + fitDataUnits[iParam]);
	hist->GetYaxis()->SetTitle("Number of tubes");
	hist->Draw();
      }
      l_fitparam->AddEntry(hist, TString::Format("Run %d", v_runN.at(iHist)), "l");
    }
    l_fitparam->Draw();
    c_fitparam->Print("output/runCompare/" + fitDataNames[iParam] + ".png");

    delete c_fitparam;
    delete l_fitparam;
    for (auto p : v_fitparam) delete p;
    v_fitparam.clear();
  }
}

void compareResiduals() {
  ConfigParser cp = ConfigParser("conf/run188566.conf");
  int runN = 9;
  int runNnoE= 12;

  ResolutionResult* rr = new ResolutionResult(cp);
  rr->Load(IOUtility::getFitResidualPath(runN));
  rr->residuals->GetFunction("doubGaus")->SetBit(TF1::kNotDraw);

  ResolutionResult* rrnoE = new ResolutionResult(cp);
  rrnoE->Load(IOUtility::getFitResidualPath(runNnoE));
  rrnoE->residuals->GetFunction("doubGaus")->SetBit(TF1::kNotDraw);
  rrnoE->residuals->SetLineColor(kRed);
  rrnoE->residuals->Scale(rr->residuals->Integral()/rrnoE->residuals->Integral());
  
  TLegend* leg = new TLegend(0.1,0.7,0.48,0.9);
  leg->AddEntry(rr->residuals, "Full cosmic ray simulation", "l");
  leg->AddEntry(rrnoE->residuals, "Only cosmic muons", "l");

  TCanvas* c = new TCanvas("c", "", 800, 600);
  c->cd();
  rr->residuals->Draw();
  rrnoE->residuals->Draw("same");
  leg->Draw();

  c->Print("test.png");

  
}

void compareRT() {
  int runN = 188566;
  int minEvent = 0;
  int nEvents  = 100000;
  TFile autocal(IOUtility::getAutoCalFilePath(runN, minEvent, nEvents));
  Optimizer* rtp;
  rtp = new RTParam();
  ((RTParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
  ((RTParam*)rtp)->Load(&autocal);

  RTParam gasMonitorRT = RTParam();
  gasMonitorRT.LoadTxt("raw/Rt_BMG_6_1.dat");
  //gasMonitorRT.Initialize(IOUtility::getT0FilePath(191129), IOUtility::getDecodedOutputFilePath(191129));
  //gasMonitorRT.Load(new TFile(IOUtility::getAutoCalFilePath(191129, minEvent, nEvents)));

  TH1* hist = new TH1D("rthist", "", 200, -1, 1);
  for (size_t b = 1; b <= hist->GetNbinsX(); b++) hist->SetBinContent(b, gasMonitorRT.Eval(hist->GetBinCenter(b)));
  std::cout << "gas monitor at -1: " << gasMonitorRT.Eval(0.5) << std::endl;
  std::cout << "rtp at -1:         " << ((RTParam*)rtp)->Eval(0.5) << std::endl;

  for (int i = 0; i < gasMonitorRT.size(); i++) {
    std::cout << "GasM: " << gasMonitorRT[i] << " rtp: " << rtp->operator[](i) << std::endl;
    gasMonitorRT.setParam(i, (gasMonitorRT[i] - rtp->operator[](i)));
  }
  gasMonitorRT.SaveImage("output", ";Drift Time [ns];#Delta r(t) [mm]", kFALSE);

  std::cout << "gas monitor at -1: " << gasMonitorRT.Eval(-1.0) << std::endl;
  std::cout << "rtp at -1:         " << ((RTParam*)rtp)->Eval(-1.0) << std::endl;
 
  std::vector<double> x = std::vector<double>();
  std::vector<double> y = std::vector<double>();
  std::vector<double> ex = std::vector<double>();
  std::vector<double> ey = std::vector<double>();

  double delta = 0.01;
  for (double _x = -1; _x <= 1; _x += delta) {
    x.push_back((_x+1.)/2.*175.);
    y.push_back(100*((RTParam*)rtp)->Eval(_x)/hist->GetBinContent(x.size()));
    ex.push_back(delta/2.0);
    ey.push_back(100*Hit::RadiusError((_x+1.)*Geometry::max_drift_dist/2.0)/hist->GetBinContent(x.size()));
  }

  auto ge = new TGraphErrors(x.size(), &x[0], &y[0], &ex[0], &ey[0]);
  ge->SetLineColor(kBlue);
  ge->SetFillColor(6);
  ge->SetFillStyle(3005);
  ge->SetLineWidth(1);
  ge->SetMarkerStyle(0);
  ge->Print();
  ge->SetTitle(";Drift Time [ns];% r(t) difference");
  ge->SetMinimum(-100);
  ge->SetMaximum(100);
  TCanvas* c1 = new TCanvas();
  c1->cd();
  ge->Draw("a4 L");  
  c1->Print("output/rt_percent_diff.png");
}

void manyAutoCalCompare() {
  int runN = 188566;
  int minEvent = 0;
  int nEvents  = 100000;
  TFile autocal(IOUtility::getAutoCalFilePath(runN, minEvent, nEvents));
  Optimizer* rtp;
  rtp = new RTParam();
  ((RTParam*)rtp)->Initialize(IOUtility::getT0FilePath(runN), IOUtility::getDecodedOutputFilePath(runN));
  ((RTParam*)rtp)->Load(&autocal);
  std::vector<double> r_nom = std::vector<double>();
  double delta = 0.01;
  for (double _x = -1; _x <= 1; _x += delta) {
    r_nom.push_back(((RTParam*)rtp)->Eval(_x));
  }
  // load other graphs and push onto vector of tgraph
  std::vector<TGraph*> grs = std::vector<TGraph*>();
  std::vector<double>  x   = std::vector<double>();
  std::vector<double>  y   = std::vector<double>();

  TFile* autocal2;

  for (minEvent = 0; minEvent <= 500000; minEvent+=100000) {
    std::cout << "MIN EVENT: " << minEvent << std::endl;
    autocal2 = TFile::Open(IOUtility::getAutoCalFilePath(runN, minEvent, nEvents));
    ((RTParam*)rtp)->Load(autocal2);
    x.clear();
    y.clear();
    for (double _x = -1; _x <= 1; _x += delta) {
      x.push_back((_x+1.)/2.*175.);
      y.push_back(1000*(((RTParam*)rtp)->Eval(_x) - r_nom.at((int)((_x+1.)/delta))));
    }
    grs.push_back(new TGraph(x.size(), &x[0], &y[0]));
    delete autocal2;
  }
  
  // draw the graphs
  std::vector<int> colors = std::vector<int>();
  colors.push_back(1);
  colors.push_back(857);
  colors.push_back(880);
  colors.push_back(419);
  colors.push_back(808);
  colors.push_back(633);
  int j = 0;
  
  TCanvas* c1 = new TCanvas("c1", "", 1000, 800);
  c1->cd();
  grs.at(0)->SetMinimum(-15);
  grs.at(0)->SetMaximum(15);
  grs.at(0)->GetXaxis()->SetTitle("Drift time [ns]");
  grs.at(0)->GetYaxis()->SetTitle("#Delta r(t) [#mu m]");
  grs.at(0)->SetTitle("");
  grs.at(0)->SetMarkerStyle(0);
  grs.at(0)->Draw();
  for (TGraph* g : grs) {
    g->SetLineWidth(2);
    g->SetLineColor(colors.at(j));
    g->Draw("same");
    j++;
  }
  c1->Print("output/run188566/autocalibration/manyRT.png");
  c1->Clear();

  std::vector<double> maxdiff = std::vector<double>();
  double min=0, max=0;
  double thisX, thisY;
  for (size_t i = 0; i < x.size(); i++) {
    min=0;
    max=0;
    for (TGraph* g : grs) {
      g->GetPoint(i, thisX, thisY);
      if (thisY > max) max = thisY;
      if (thisY < min) min = thisY;
    }
    maxdiff.push_back(max-min);
  }
  TGraph* maxrtDelta = new TGraph(x.size(), &x[0], &maxdiff[0]);
  maxrtDelta->GetXaxis()->SetTitle("Drift time [ns]");
  maxrtDelta->GetYaxis()->SetTitle("Maximum #Delta r(t) [#mu m]");
  maxrtDelta->SetTitle("");
  maxrtDelta->SetLineColor(kRed);
  maxrtDelta->SetLineWidth(2);
  maxrtDelta->SetMinimum(0);
  maxrtDelta->SetMaximum(20);
  maxrtDelta->SetMarkerStyle(0);
  maxrtDelta->Draw();
  c1->Print("output/run188566/autocalibration/maxRTDiff.png");
}

TGraphErrors* drawObservable(Observable obs, double x, Color_t c = kBlack) {
  double ex = 0;
  TGraphErrors* gr = new TGraphErrors(1, &x, &obs.val, &ex, &obs.err);
  gr->SetMarkerStyle(20);
  gr->SetMarkerColor(c);
  gr->SetMarkerSize(1.3);
  gr->SetLineColor(c);
  gr->SetLineWidth(2);
  gr->Draw("same P");
  return gr;
}

void SummaryPlots() {
  int nChambers = 10;
  TCanvas* canvas = new TCanvas("canvas", "", 800, 600);
  
  canvas->SetRightMargin(0.15);
  
  TH1D* resolution_asd1 = new TH1D("resolution_asd1", ";;Resolution [#mu m]", nChambers, -0.5, nChambers-0.5);
  resolution_asd1->SetMinimum(60);
  resolution_asd1->SetMaximum(130);
  resolution_asd1->SetStats(0);
  resolution_asd1->GetXaxis()->SetBinLabel(1, "BMG Prototype");
  resolution_asd1->GetXaxis()->SetBinLabel(2, "BIS Module 0");
  resolution_asd1->GetXaxis()->SetBinLabel(3, "BIS Module 1");
  resolution_asd1->GetXaxis()->SetBinLabel(4, "BIS Module 2");
  resolution_asd1->GetXaxis()->SetBinLabel(5, "BIS Module 3");
  resolution_asd1->GetXaxis()->SetBinLabel(6, "BIS Module 4");
  resolution_asd1->GetXaxis()->SetBinLabel(7, "BIS Module 5");
  resolution_asd1->GetXaxis()->SetBinLabel(8, "BIS Module 6");
  resolution_asd1->GetXaxis()->SetBinLabel(9, "BIS Module 7");
  resolution_asd1->GetXaxis()->SetBinLabel(10, "BIS Module 8");
  resolution_asd1->GetXaxis()->SetLabelSize(0.06);
  resolution_asd1->Draw();
  TLine* res_exp = new TLine(-0.5, 106, nChambers-0.5, 106);
  res_exp->SetLineColor(kRed);
  res_exp->SetLineWidth(2);
  res_exp->Draw();
  auto res_obs = drawObservable(Observable(103.7, 8.1), 0);
  auto res_obs2 = drawObservable(Observable(101.8, 7.8), 0.94);
  auto res_obs3 = drawObservable(Observable(84.1, 7.8), 1.06, kBlue);
  auto res_obsM1 = drawObservable(Observable(80.4,7.8), 2, kBlue);
  auto res_obs4 = drawObservable(Observable(79.79, 7.8), 3, kBlue);
  auto res_obs5 = drawObservable(Observable(96.9,  7.8), 4, kBlue);
  auto res_obs6 = drawObservable(Observable(88.69, 7.8), 5, kBlue);
  auto res_obs7 = drawObservable(Observable(93.71, 7.8), 6, kBlue);
  auto res_obs8 = drawObservable(Observable(92.73, 7.8), 7, kBlue);
  auto res_obs9 = drawObservable(Observable(94.13, 7.8), 8, kBlue);
  auto res_obs10 = drawObservable(Observable(0,0), 9, kBlue);
 
  TLegend* res_leg = new TLegend(0.6,0.75,0.8,0.9);
  res_leg->AddEntry(res_exp,  "Expected", "l");
  res_leg->AddEntry(res_obs,  "Observed, ASD 1", "ep");
  res_leg->AddEntry(res_obs3, "Observed, ASD 2", "ep");
  res_leg->SetBorderSize(0);
  res_leg->Draw();
  
  canvas->Print("output/runCompare/ResSummary.png");

  TH1D* efficiency = new TH1D("efficiency", ";;Efficiency", nChambers, -0.5, nChambers-0.5);
  efficiency->SetMinimum(0.925);
  efficiency->SetMaximum(0.955);
  efficiency->SetStats(0);
  efficiency->GetXaxis()->SetBinLabel(1, "BMG Prototype");
  efficiency->GetXaxis()->SetBinLabel(2, "BIS Module 0");
  efficiency->GetXaxis()->SetLabelSize(0.06);
  efficiency->Draw();
  TLine* eff_exp = new TLine(-0.5, 0.94, nChambers-0.5, 0.94);
  eff_exp->SetLineColor(kRed);
  eff_exp->SetLineWidth(2);  
  eff_exp->Draw();
  auto eff_obs = drawObservable(Observable(0.942, 0.002), 0);
  auto eff_obs2 = drawObservable(Observable(0.944,0.002), 1);
  TLegend* eff_leg = new TLegend(0.7,0.8,0.9,0.9);
  eff_leg->AddEntry(eff_exp, "Expected", "l");
  eff_leg->AddEntry(eff_obs, "Observed", "ep");
  eff_leg->SetBorderSize(0);
  eff_leg->Draw();
  
  canvas->Print("output/runCompare/EffSummary.png");
}

void TimeSlewPlot(TString fname, TString outname) {

  std::cout << std::endl;
  std::cout << "Fname: " << fname << std::endl;
  std::cout << "outname: " << outname << std::endl;

  TGaxis::SetMaxDigits(3);

  TFile* fnoslew = TFile::Open(fname);
  TH2*   hnoslew = 0;
  fnoslew->GetObject("RR_residualVsADC", hnoslew);

  // overlay mean residual vs adc                                                                          
  double maxADC = 0.012;
  double minADC = 0.005;
  double deltaADC = 0.001;
  std::vector<double> _x = std::vector<double>();
  std::vector<double> _y = std::vector<double>();
  std::vector<double> _ex = std::vector<double>();
  std::vector<double> _ey = std::vector<double>();
  double upper;
  int binL, binU;
  TH1* py;
  for (double lower = minADC; lower < maxADC; lower += deltaADC) {
    upper = lower + deltaADC;
    binL  = hnoslew->GetXaxis()->FindBin(lower);
    binU  = hnoslew->GetXaxis()->FindBin(upper);
    std::cout << "binL: " << binL << " binU: " << binU << std::endl;
    py    = hnoslew->ProjectionY("_py", binL, binU);

    _x .push_back((upper+lower)/2);
    _ex.push_back((upper-lower)/2);
    _y .push_back(py->GetMean());
    _ey.push_back(py->GetMeanError());
  }

  TF1* slewFunction = new TF1("slewFunction", "[0]*x+[1]", minADC, maxADC);
  slewFunction->SetParameter(0,0);
  slewFunction->SetParameter(1,0);

  TGraphErrors* tge = new TGraphErrors(_x.size(), &_x[0], &_y[0], &_ex[0], &_ey[0]);
  tge->SetLineColor(kRed);
  tge->SetMarkerColor(kRed);
  tge->SetMarkerStyle(20);
  tge->SetMarkerSize (1);
  tge->Fit("slewFunction");
  tge->GetFunction("slewFunction")->SetLineColor(kBlack);
  auto fitlegend = new TLegend(0.4,0.2,0.75,0.3);
  fitlegend->AddEntry(tge->GetFunction("slewFunction"),
		      TString::Format("t=%.2f/ADC + %.2f",
				      tge->GetFunction("slewFunction")->GetParameter(0),
				      tge->GetFunction("slewFunction")->GetParameter(1)), "l");
  tge->Print();


  TCanvas* c = new TCanvas("c", "", 800, 600);
  c->SetLeftMargin(0.15);
  c->SetRightMargin(0.2);
  c->cd();
  hnoslew->GetZaxis()->SetTitle("Entries");
  hnoslew->Draw("colz");
  c->Update();
  TPaletteAxis *palette = 
    (TPaletteAxis*)hnoslew->GetListOfFunctions()->FindObject("palette");
  palette->SetY1NDC(0.25);
  fitlegend->Draw();
  tge->Draw("P Z E same");
  c->Modified();
  c->Print(outname);
  delete c;
  fnoslew->Close();
}

void TimeSlewPlots() {
  TimeSlewPlot("/atlas/data19/kevnels/sMDT/output/run188566/fitResiduals/Nominal_noslew/fitResiduals.root",
	       "output/runCompare/NoTimeSlew.png");
  TimeSlewPlot("/atlas/data19/kevnels/sMDT/output/run188566/fitResiduals/Nominal/fitResiduals.root", 
	       "output/runCompare/WithTimeSlew.png");
  TimeSlewPlot("/atlas/data19/kevnels/sMDT/output/run188566/fitResiduals/slew_down/fitResiduals.root", 
	       "output/runCompare/WithTimeSlew_down.png");
  TimeSlewPlot("/atlas/data19/kevnels/sMDT/output/run188566/fitResiduals/slew_up/fitResiduals.root",
	       "output/runCompare/WithTimeSlew_up.png");
}

Double_t cavernResolution(Double_t *x, Double_t *par) {
  return 1000*Hit::CavernRadiusError(x[0]);
}

void compareResCurve() {
  std::vector<int> v_runN = std::vector<int>();
  //v_runN.push_back(191222);
  //v_runN.push_back(191241);
  //v_runN.push_back(1912412);
  //v_runN.push_back(191176);
  //v_runN.push_back(100001);
  v_runN.push_back(188566);
  v_runN.push_back(191328);
  //v_runN.push_back(191375);
  //v_runN.push_back(191436);
  //v_runN.push_back(191440);
  v_runN.push_back(191609);

  //v_runN.push_back(191533);
  //v_runN.push_back(191506);
  

  std::vector<int> v_colors = std::vector<int>();
  v_colors.push_back(kRed);
  v_colors.push_back(kBlack);
  v_colors.push_back(kOrange + 7);
  v_colors.push_back(kAzure - 4);
  v_colors.push_back(kPink + 5);
  v_colors.push_back(kYellow + 2);
  v_colors.push_back(kCyan + 2);
  
  std::vector<TString> v_names = std::vector<TString>();
  //v_names.push_back("Module 0, 39 mV, nominal wire position");
  //v_names.push_back("Module 0, 39 mV, nominal wire position");
  //v_names.push_back("Module 0, 39 mV, linear wire pitch correction");
  //v_names.push_back("Module 0, 51 mV, nominal wire position");
  //v_names.push_back("Module 0, 39 mV, full tdc t0 fit");
  v_names.push_back("BMG Prototype Chamber, ASD-1");
  v_names.push_back("BIS Module 0, ASD-2");
  //v_names.push_back("Module 0, ASD 2, June 2021");
  //v_names.push_back("Module 1, ASD 2, June 2021");
  //v_names.push_back("Module 2, ASD 2, July 2021");
  //v_names.push_back("Module 2, new RO HH cards, old FE layout");
  //v_names.push_back("Module 2, ASD 2, July 2021");

  //v_names.push_back("Module 3, ASD 2, August 2021");
  v_names.push_back("Module 4, ASD 2, August 2021");

  TCanvas* c1 = new TCanvas("c1", "Mutliple resolution curves", 800,800);
  c1->cd();
  TLegend* legend = new TLegend(0.4,0.65,0.9,0.9);
  TF1* cavernRes = new TF1("cavernRes", cavernResolution, 0, 7.1,0);
  cavernRes->SetLineColor(kBlue);
  cavernRes->SetTitle(";Drift Radius [mm];Resolution [#mum]");
  cavernRes->SetMinimum(40);
  cavernRes->Draw();
  legend->AddEntry(cavernRes, "#splitline{MDT resolution}{measured in ATLAS data}", "l");
  

  for (size_t i = 0; i < v_runN.size(); ++i) {
    TFile* f = TFile::Open(IOUtility::getResolutionCurveFile(v_runN.at(i)));
    TF1* func = (TF1*)(f->Get("fitfunc"));
    //v_funcs.push_back(func);
    f->Close();
    c1->cd();
    func->SetLineColor(v_colors.at(i));
    func->Draw("same");
    
    TFile* f2 = TFile::Open(IOUtility::join(IOUtility::getResolutionOutputDir(v_runN.at(i)), "resolutionHist.root"));
    TH1* hist = (TH1*)(f2->Get("resolutionVsRadius"));
    hist->SetLineColor(v_colors.at(i));
    hist->SetMarkerColor(v_colors.at(i));
    hist->GetFunction("pol2")->SetLineColor(v_colors.at(i));
    hist->Draw("same");
    legend->AddEntry(hist, v_names.at(i), "lep");
  }

  legend->SetBorderSize(0);
  legend->Draw();
  c1->Print("output/runCompare/compareResCurve.png");

}

int main() {

  SetAtlasStyle();

  //T0plots();
  //compareResiduals();
  //compareRT();
  //manyAutoCalCompare();
  SummaryPlots();
  //TimeSlewPlots();
  //compareResCurve();
  return 0;
}
