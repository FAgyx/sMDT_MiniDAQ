#include "src/Geometry.cpp"


using namespace MuonReco;

void GetTubeT0(TString inputFilename = "run00187691_20190301.dat") {
  Geometry geo = Geometry();

  TString fn = TString(inputFilename);
  int runN = ((TObjString*)(TString(fn(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  geo.SetRunN(runN);

  TVectorD *fitParams;
  char fitVecName[256];

  TString outfile = "T0.txt";

  ofstream txtfile;
  txtfile.open(outfile);
  txtfile << "TDC Chan T0\n";

  TString t0path = "output/";
  t0path += inputFilename;
  t0path += ".dir/";

  TFile t0File(t0path + "T0.root");

  for (Int_t tdc_id = 0; tdc_id < geo.MAX_TDC; tdc_id++) {

    for (Int_t ch_id = 0; ch_id != geo.MAX_TDC_CHANNEL; ch_id++) {
      if (tdc_id == geo.TRIGGER_MEZZ || !geo.IsActiveTDCChannel(tdc_id, ch_id)) {continue;}

      sprintf(fitVecName, "FitData_tdc_%d_channel_%d", tdc_id, ch_id);
      fitParams = (TVectorD*)t0File.Get(fitVecName);
      txtfile << tdc_id << " " << ch_id << " " << (*fitParams)[1] << endl;


    }
  }
}
