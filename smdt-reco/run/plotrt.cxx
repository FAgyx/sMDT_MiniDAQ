#include "src/Geometry.cpp"
#include "macros/AtlasStyle/AtlasLabels.C"
#include "macros/AtlasStyle/AtlasStyle.C"
#include "macros/AtlasStyle/AtlasUtils.C"
#include "src/RTParam.cpp"
#include "src/IOUtility.cpp"
#include "src/Observable.cpp"
#include "src/ResolutionResult.cpp"

using namespace MuonReco;

void plotrt(TString inputFileName = "run00187985_20190607.dat") {
  //SetAtlasStyle();

  // draw the autocalibration output
  gROOT->SetBatch(kTRUE);
  
  Geometry geo = Geometry();
  geo.SetRunN(187985);
  
  
  TCanvas* c_resolution = new TCanvas("c_resolution", "Final Resolution calculation");
  TH1D* h_resolution;
  
  ResolutionResult* rrh = new ResolutionResult(geo);
  rrh->Load(IOUtility::getHitResidualPath(inputFileName));
  
  rrh->Draw();
  h_resolution = (TH1D*) rrh->resolutionRadius->Clone();
  h_resolution->SetDirectory(0);
  

  ResolutionResult* rrf = new ResolutionResult(geo);
  rrf->Load(IOUtility::getFitResidualPath(inputFileName));
  rrf->Draw();
  //rrf->SaveImages();
  
  for (int bin = 1; bin <= h_resolution->GetNbinsX(); bin++) {
    Observable fitRes     = Observable(rrf->resolutionRadius->GetBinContent(bin), rrf->resolutionRadius->GetBinError(bin));
    Observable hitRes     = Observable(h_resolution->GetBinContent(bin),          h_resolution->GetBinError(bin));
    Observable resolution = (fitRes*hitRes).power(0.5);
    
    h_resolution->SetBinContent(bin, resolution.val);
    h_resolution->SetBinError  (bin, resolution.err);
  }

  c_resolution->cd();
  h_resolution->GetYaxis()->SetTitle("Resolution [#mu m]");
  h_resolution->SetStats(0);
  //h_resolution->Fit("pol2");
  h_resolution->GetListOfFunctions()->Delete();//FindObject("pol2");
  h_resolution->Draw();

  // draw nominal resolution

  TString fname  = IOUtility::getMDTRTFilePath();
  double radius, time, error;
  vector<double> v_radius, v_time, v_error = vector<double>();
  std::ifstream infile(fname);
  string line;
  int lineNum = 0;
  while (getline(infile, line)) {
    std::istringstream iss(line);
    if (lineNum != 0) {
      if (!(iss >> radius >> time >> error)) { break; }
      v_radius.push_back(radius);
      v_error .push_back(error);
    }
    lineNum++;
  }


  TGraph* nom_err = new TGraph(v_error.size());
  for (int i = 0; i < v_error.size(); i++) {
    nom_err->SetPoint(i, v_radius.at(i), v_error.at(i)*1000);
  }
  nom_err->Draw("same");

  nom_err->SetLineColor(kRed);
  nom_err->SetLineWidth(2);
  nom_err->SetMarkerColorAlpha(kRed, 1);
  nom_err->SetMarkerSize(2);
  nom_err->SetMarkerStyle(22);
  
  TLegend* res_leg = new TLegend(0.5,0.7,0.9,0.9);
  res_leg->AddEntry(h_resolution, "My resolution", "le");  
  res_leg->AddEntry(nom_err, "Gas monitor resolution", "l");
  res_leg->Draw();
  c_resolution->SaveAs("resolution.png");



  // single hit resolution calculation

  double singleHitRes = 0.0;
  for (int bin = 1; bin <= h_resolution->GetNbinsX(); bin++) {
    singleHitRes += h_resolution->GetBinContent(bin);
  }
  singleHitRes /= 7.0;


  cout << "Bin by bin calculation: " << endl;
  cout << "Single Hit Resolution: " << singleHitRes << endl;


  double fit_nw, hit_nw, fit_err, hit_err;
  rrf->GetWidthAndError(&fit_nw, &fit_err, rrf->residuals);
  rrh->GetWidthAndError(&hit_nw, &hit_err, rrh->residuals);


  Observable fitObs           = Observable(fit_nw, fit_err);
  Observable hitObs           = Observable(hit_nw, hit_err);
  Observable singleHitRes_all = (fitObs*hitObs).power(0.5);

  cout << fit_nw << endl;
  cout << hit_nw << endl;
  cout << "All together calculation: " << endl;
  cout << "Single Hit Resolution: "    << singleHitRes_all.val << " +/- " << singleHitRes_all.err << endl;


}
