#include "src/RTParam.cpp"
#include "src/IOUtility.cpp"
#include "src/Geometry.cpp"

using namespace Muon;

void RT_ML(TString inputFileName = "run00187985_20190607.dat") {
  Geometry geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  RTParam rtp_1 = RTParam(geo);
  rtp_1.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  
  RTParam rtp_2 = RTParam(geo);
  rtp_2.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));

  TFile autocal(IOUtility::getAutoCalFilePath(inputFileName));
  
  rtp_1.Load(&autocal, "ML0");
  rtp_2.Load(&autocal, "ML1");

  TF1* difference = rtp_1.RTDifference(&rtp_2);

  TCanvas* c_diff = new TCanvas("c_diff", "Difference of RT functions");
  difference->Draw();
  difference->GetXaxis()->SetTitle("Normalized Time [T0, Tmax]");
  difference->GetYaxis()->SetTitle("Difference between R(t) [#mu m]");
  difference->SetTitle("Difference between ML autocalibrated RT functions");
  c_diff->SaveAs("RTdifference.png");
}
