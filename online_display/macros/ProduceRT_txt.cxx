#include "src/RTParam.cpp"
#include "src/Geometry.cpp"
#include "src/IOUtility.cpp"

using namespace Muon;

void ProduceRT_txt(TString inputFileName = "run00187691_20190301.dat") {
  gROOT->SetBatch(kTRUE);
  TFile  f(IOUtility::getDecodedOutputFilePath(inputFileName));
  TFile autocal(IOUtility::getAutoCalFilePath(inputFileName));
  Geometry   geo = Geometry();
  geo.SetRunN(IOUtility::getRunN(inputFileName));

  RTParam rtp = RTParam(geo);
  rtp.Initialize(IOUtility::getT0FilePath(inputFileName), IOUtility::getDecodedOutputFilePath(inputFileName));
  rtp.Load(&autocal);
  rtp.WriteOutputTxt(IOUtility::getRTTxtDirPath(inputFileName));
}
