#include "TH1.h"
#include "TCanvas.h"
#include "TVirtualFFT.h"
#include "TSpectrum.h"
#include "TLegend.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/ResolutionResult.h"
#include "MuonReco/IOUtility.h"


using namespace MuonReco;

Double_t cavernResolution(Double_t *x, Double_t *par) {
  return 1000*Hit::CavernRadiusError(x[0]);
}

void AddSystematic(double* errSq, double nominal, std::vector<TString> systNames, int runN) {
  std::vector<Observable> observables = std::vector<Observable>();
  for (TString Sname : systNames) observables.push_back(Observable::Load(IOUtility::getResolutionObservable(runN, Sname)));

  double maxDiff = 0;
  int count = 0;
  TString name = "Nominal";
  for (int i = 0; i < observables.size(); i++) {
    Observable o = observables.at(i);
    if (o.val != 0 && TMath::Abs(o.val - nominal) > maxDiff) {
      maxDiff = TMath::Abs(o.val - nominal);
      count = i;
    }
  }
  name = systNames.at(count);
  std::cout << "Error contribution from " << name << " " << maxDiff << std::endl;
  *errSq += maxDiff*maxDiff;
}

Observable resolution(ConfigParser cp, int runN, TString deconvRunName, TString systName, TF1* fitfunc=0) {
  TString subDir;
  if (!deconvRunName.CompareTo("DeconvolutionRun"))
    subDir = "Nominal";
  else if (!deconvRunName.CompareTo("DeconvolutionRunSoftEUp"))
    subDir = "SoftEUp";
  else if (!deconvRunName.CompareTo("DeconvolutionRunSoftEDown"))
    subDir = "SoftEDown";
  else
    subDir = "NoSoftE";

  // load Fit Residuals
  ResolutionResult* rr_f = new ResolutionResult(cp);
  rr_f->Load(IOUtility::getFitResidualPath(runN, systName));
  rr_f->deconvolutionSubDir = subDir;
  rr_f->systName = systName;

  std::vector<int> dConvRunNs = cp.items("Resolution").getIntVector(deconvRunName);
  std::vector<int> rConvRunNs = cp.items("Resolution").getIntVector("ReconvolutionRun");
  ResolutionResult* rrmc = new ResolutionResult(cp);

  // perform convolutions with biased residuals
  TH1* fitResVsRadius, *nocorr_fitResVsRadius;
  for (int rConvRunN : rConvRunNs) {
    rrmc->Load(IOUtility::getFitResidualPath(rConvRunN));
    fitResVsRadius = rr_f->Deconvolute(*rrmc, kFALSE, kTRUE);
  }
  for (int dConvRunN : dConvRunNs) {
    rrmc->Load(IOUtility::getFitResidualPath(dConvRunN));
    fitResVsRadius = rr_f->Deconvolute(*rrmc);
  }

  double w_fit, e_fit;
  ResolutionResult::GetWidthAndError(&w_fit, &e_fit, rr_f->residuals);
  std::cout << "width: " << w_fit << std::endl;

  // load hit residuals
  ResolutionResult* rr_h = new ResolutionResult(cp);
  rr_h->Load(IOUtility::getHitResidualPath(runN, systName));
  rr_h->deconvolutionSubDir = subDir;
  rr_h->systName = systName;

  // perform convolutions with unbiased residuals
  TH1* hitResVsRadius;
  for (int rConvRunN : rConvRunNs) {
    rrmc->Load(IOUtility::getFitResidualPath(rConvRunN));    
    hitResVsRadius = rr_h->Deconvolute(*rrmc, kTRUE, kTRUE);
  }
  for (int dConvRunN : dConvRunNs) {
    rrmc->Load(IOUtility::getFitResidualPath(dConvRunN));
    hitResVsRadius = rr_h->Deconvolute(*rrmc, kTRUE);
  }



  TCanvas* cResVsRad = new TCanvas("cResVsRad", "", 800, 800);
  cResVsRad->SetLeftMargin(0.15);

  std::cout << "Fit and Hit residuals vs. radius" << std::endl;
  for (int b = 1; b <= fitResVsRadius->GetNbinsX(); b++) {
    std::cout << b << ": " << fitResVsRadius->GetBinContent(b) << " : " 
	      << hitResVsRadius->GetBinContent(b) << std::endl;
  }
  fitResVsRadius->Fit("pol2");
  hitResVsRadius->Fit("pol2");
  ResolutionResult::GetWidthAndError(&w_fit, &e_fit, rr_f->residuals);
  std::cout << "Real fit sigma: " << w_fit << " +/- " << e_fit << std::endl;
  ResolutionResult::GetWidthAndError(&w_fit, &e_fit, rr_h->residuals);
  std::cout << "Real hit sigma: " << w_fit << " +/- " << e_fit << std::endl;
  ResolutionResult::calculateResVsRadius(fitResVsRadius, hitResVsRadius);
  std::cout << "Resolution vs. radius" << std::endl;
  for (int b = 1; b <= fitResVsRadius->GetNbinsX(); b++) {
    std::cout << b << ": " << fitResVsRadius->GetBinContent(b) << std::endl;
  }  
  cResVsRad->cd();
  fitResVsRadius->SetStats(0);
  fitResVsRadius->Fit("pol2");
  fitResVsRadius->GetFunction("pol2")->SetLineColor(kRed);
  
  nocorr_fitResVsRadius = rr_f->GetUncorrectedResolutionCurve(*rr_h, IOUtility::join(IOUtility::join(IOUtility::getResolutionOutputDir(runN), systName), "NoCorrection"));
  nocorr_fitResVsRadius->SetStats(0);
  nocorr_fitResVsRadius->Fit("pol2");
  nocorr_fitResVsRadius->GetFunction("pol2")->SetLineColor(kRed);
  nocorr_fitResVsRadius->GetFunction("pol2")->SetLineStyle(2);
  cResVsRad->cd();
  fitResVsRadius->Draw();
  nocorr_fitResVsRadius->Draw("same");

  // save resolution vs radius histogram
  TFile* f_histOut = new TFile(IOUtility::join(IOUtility::getResolutionOutputDir(runN), "resolutionHist.root"), "RECREATE");
  fitResVsRadius->Write("resolutionVsRadius");
  f_histOut->Write();
  f_histOut->Close();

  TF1* cavernRes = new TF1("cavernRes", cavernResolution, 0, 7.1,0);
  cavernRes->SetLineColor(kBlue);
  cavernRes->Draw("same");
  TF1* withoutGeant = new TF1("withoutGeant", "[0] + [1]*x + [2] *x*x", 0, 7.1);
  withoutGeant->SetParameter(0, 212.95);
  withoutGeant->SetParameter(1, -36.559);
  withoutGeant->SetParameter(2, 2.257);
  withoutGeant->SetLineColor(kOrange+8);
  withoutGeant->Draw("same");
  fitResVsRadius->SetMaximum(250);
  fitResVsRadius->SetMinimum(50);
  auto leg = new TLegend(0.6,1,0.6,1);
  leg->AddEntry(fitResVsRadius->GetFunction("pol2"), "#splitline{sMDT resolution,}{this result}", "l");
  leg->AddEntry(cavernRes, "#splitline{MDT resolution}{measured in ATLAS cavern}", "l");
  leg->AddEntry(withoutGeant, "#splitline{sMDT resolution,}{Prototype BMG chamber}", "l");
  leg->Draw();
  cResVsRad->Print(IOUtility::join(IOUtility::getResolutionOutputDir(runN), "resolutionVsRadius.png"));

  ResolutionResult::GetWidthAndError(&w_fit, &e_fit, rr_f->residuals);
  double w_hit, e_hit;
  ResolutionResult::GetWidthAndError(&w_hit, &e_hit, rr_h->residuals);
  std::cout << "width: " << w_hit << std::endl;
  Observable sigma_h = Observable(w_hit, e_hit);
  Observable sigma_f = Observable(w_fit, e_fit);
  sigma_h *= sigma_f;
  Observable sigma = sigma_h.power(0.5);
  std::cout << "Biased   (fit) residual weighted sigma: " << w_fit << " +/- " << e_fit << std::endl;
  std::cout << "Unbiased (hit) residual weighted sigma: " << w_hit << " +/- " << e_hit << std::endl;
  std::cout << deconvRunName << std::endl;

  
  if (fitfunc) {
    fitfunc->SetParameter(0, fitResVsRadius->GetFunction("pol2")->GetParameter(0));
    fitfunc->SetParameter(1, fitResVsRadius->GetFunction("pol2")->GetParameter(1));
    fitfunc->SetParameter(2, fitResVsRadius->GetFunction("pol2")->GetParameter(2));
  }

  for (int i = 0; i <10; i++) std::cout << std::endl;

  delete cResVsRad;
  delete rr_f;
  delete rr_h;
  delete rrmc;
  //delete cavernRes;
  delete leg;
  return sigma;
}


int main(int argc, char* argv[]) {
  
  SetAtlasStyle();

  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry"));
  int runN        = cp.items("General").getInt("RunNumber");
  std::cout << "Run Number: " << runN << std::endl;

  Bool_t floatUp = kTRUE;
  if (ap.hasKey("-d")) {
    floatUp = kFALSE;
  }

  Bool_t biasSyst = kFALSE;
  if (ap.hasKey("-b")) biasSyst = kTRUE;

  Bool_t anySystematic = kFALSE;
  TString systName     = "Nominal";
  int partitionIndex = 0;
  if (ap.hasKey("--syst")) {
    systName  = ap.getTString("--syst");
    if (!systName.CompareTo("t0") || !systName.CompareTo("slope") ||
        !systName.CompareTo("intercept") || !systName.CompareTo("slew") ||
	!systName.CompareTo("sigProp")) {

      if (!biasSyst) {
	if (floatUp) systName += "_up";
	else systName += "_down";
      }
      else {
	systName += "_bias";
      }
      anySystematic = kTRUE;
    }
    else {
      partitionIndex = TString(systName(systName.Length()-1, systName.Length())).Atoi();
    }
  }

  TF1* fitfunc = new TF1("fitfunc", "pol2", 0, Geometry::max_drift_dist);

  Observable sigmaSoftEUp   = resolution(cp, runN, "DeconvolutionRunSoftEUp",   systName);
  Observable sigmaSoftEDown = resolution(cp, runN, "DeconvolutionRunSoftEDown", systName);
  Observable sigmaNoSoftE   = resolution(cp, runN, "DeconvolutionRunNoSoftE",   systName);
  Observable sigmaNominal   = resolution(cp, runN, "DeconvolutionRun",          systName, fitfunc);

  double deltaUp     = TMath::Abs(sigmaNominal.val - sigmaSoftEUp.val);
  double deltaDown   = TMath::Abs(sigmaNominal.val - sigmaSoftEDown.val);
  double deltaSyst   = (deltaUp < deltaDown) ? deltaDown : deltaUp;
  double errWithSyst = TMath::Sqrt(sigmaNominal.err*sigmaNominal.err + deltaSyst*deltaSyst);

  std::cout << "============================================================" << std::endl;
  std::cout << "Shape variation:        " << systName << std::endl;
  std::cout << "Nominal resolution:     " << sigmaNominal.val << " +/- " << sigmaNominal.err<< std::endl;
  std::cout << "Soft E Up resolution:   " << sigmaSoftEUp.val << " +/- " << sigmaSoftEUp.err << std::endl;
  std::cout << "Soft E Down resolution: " << sigmaSoftEDown.val << " +/- " << sigmaSoftEDown.err << std::endl;
  std::cout << "No Soft E resolution:   " << sigmaNoSoftE.val << " +/- " << sigmaNoSoftE.err << std::endl;
  sigmaNominal.err = errWithSyst;
  sigmaNominal.Write(IOUtility::getResolutionObservable(runN, systName));

  // incorporate other systematics
  errWithSyst *= errWithSyst;
  std::vector<TString> systNames = std::vector<TString>();
  systNames.push_back("t0_up");
  systNames.push_back("t0_down");
  systNames.push_back("slope_up");
  systNames.push_back("slope_down");
  systNames.push_back("intercept_up");
  systNames.push_back("intercept_down");
  AddSystematic(&errWithSyst, sigmaNominal.val, systNames, runN);

  systNames.clear();
  systNames.push_back("sigProp_bias");
  AddSystematic(&errWithSyst, sigmaNominal.val, systNames, runN);

  systNames.clear();  
  systNames.push_back("slew_up");
  systNames.push_back("slew_down");
  AddSystematic(&errWithSyst, sigmaNominal.val, systNames, runN);

  systNames.clear();
  systNames.push_back("Partition1");
  systNames.push_back("Partition2");
  systNames.push_back("Partition3");
  systNames.push_back("Partition4");
  systNames.push_back("Partition5");
  AddSystematic(&errWithSyst, sigmaNominal.val, systNames, runN);


  std::cout << "Fit function: " << std::endl;
  for (int i = 0; i < fitfunc->GetNpar(); ++i) {
    std::cout << i << ": " << fitfunc->GetParameter(i) << std::endl;
  }
  TFile* fitfile = new TFile(IOUtility::getResolutionCurveFile(runN), "RECREATE");
  fitfile->cd();
  fitfunc->Write("fitfunc");
  fitfile->Write();
  fitfile->Close();

  errWithSyst = TMath::Sqrt(errWithSyst);
  std::cout << std::endl;
  std::cout << "Single hit resolution:  " << sigmaNominal.val << " +/- " << errWithSyst << " microns" << std::endl;
  std::cout << "============================================================" << std::endl;


  return 0;
}
