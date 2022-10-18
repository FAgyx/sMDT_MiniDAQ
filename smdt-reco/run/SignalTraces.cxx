#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TString.h"
#include "TGaxis.h"
#include "TAxis.h"

#include "MuonReco/IOUtility.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/Event.h"

#include "AtlasStyle/AtlasStyle.h"
#include "AtlasStyle/AtlasStyle.C"

using namespace MuonReco;

/** Runner file to plot signal count rate over time
 * 
 */
int main(int argc, char* argv[]) {

  SetAtlasStyle();
  
  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");

  TGaxis::SetMaxDigits(2);

  //
  // set up the vectors to hold traces
  //

  ConfigParser _cp = ConfigParser(IOUtility::join("conf", "signals_over_time.conf"));
  unsigned int MaxEntry = _cp.items("General").getInt("MaxEntry");
  unsigned int DEntry   = _cp.items("General").getInt("DEntry");
  std::vector<int> tdc_trace = _cp.items("Traces").getIntVector("TDC");
  std::vector<int> chan_trace = _cp.items("Traces").getIntVector("Chan");

  std::vector<unsigned int> hit_count = std::vector<unsigned int>();
  for (size_t iTrace = 0; iTrace < tdc_trace.size(); ++iTrace) hit_count.push_back(0);

  unsigned int tot_count = 0;

  std::vector<std::vector<double>> v_v_y = std::vector<std::vector<double>>();
  for (size_t iTrace = 0; iTrace < tdc_trace.size(); ++iTrace) v_v_y.push_back(std::vector<double>());
  std::vector<double> v_x = std::vector<double>();


  //
  // get the TTree and set branches appropriately
  //

  Event* e = new Event();
  TFile* eFile = TFile::Open(IOUtility::getDecodedOutputFilePath(runN));
  TTree* eTree = (TTree*)eFile->Get("eTree");
  eTree->SetBranchAddress("event", &e);

  MaxEntry = (MaxEntry < eTree->GetEntries()) ? MaxEntry : eTree->GetEntries();
  
  //
  // Perform the loop
  //

  unsigned int tdc, chan;
  
  for(unsigned int iEntry = 0; iEntry < MaxEntry; ++iEntry) {
    eTree->GetEntry(iEntry);

    for (size_t iTrace = 0; iTrace < tdc_trace.size(); ++iTrace) {
      for (auto sig : e->WireSignals()) {
	tdc = sig.TDC();
	chan = sig.Channel();
	if (tdc == (unsigned int) tdc_trace.at(iTrace) && chan == (unsigned int) chan_trace.at(iTrace)) {
	  hit_count.at(iTrace) = hit_count.at(iTrace) + 1;
	}
	++tot_count;
      } // end for: each signal in the event

      if (iEntry % DEntry == 0) {
	// fill TGraph vectors only every DEntry entries
	std::cout << "  TDC " << tdc_trace.at(iTrace) << " Channel " << chan_trace.at(iTrace) << ": " <<  hit_count.at(iTrace) << std::endl;
	v_v_y.at(iTrace).push_back((double)(hit_count.at(iTrace)));
      }
    } // end for: traces to check against event
    if (iEntry % DEntry== 0) {
      std::cout << "Total entries: " << tot_count << std::endl;
      v_x.push_back((double)(tot_count));
    }
  } // enf for: each event up to max

  //
  // make plots
  //

  TCanvas* canvas = new TCanvas("canvas", "", 800, 800);
  canvas->SetRightMargin(0.125);
  TLegend* legend = new TLegend(0.2, 0.78, 0.4, 0.92);
  std::vector<TGraph*> grs = std::vector<TGraph*>();
  for (size_t iTrace = 0; iTrace < tdc_trace.size(); ++iTrace) {
    grs.push_back(new TGraph(v_x.size(), &v_x[0], &(v_v_y.at(iTrace)[0]) ));
    grs.at(iTrace)->SetTitle(";Total Hits on Chamber;Hits on Select Channel");
    grs.at(iTrace)->SetLineColor(iTrace+2);
    grs.at(iTrace)->SetLineWidth(2);
    grs.at(iTrace)->SetLineStyle(1);
    grs.at(iTrace)->SetMarkerStyle(0);
    grs.at(iTrace)->GetXaxis()->SetNdivisions(-505);
    if (iTrace)
      grs.at(iTrace)->Draw("same");
    else
      grs.at(iTrace)->Draw();
    legend->AddEntry(grs.at(iTrace), TString::Format("TDC %d Chan %d", tdc_trace.at(iTrace), 
						     chan_trace.at(iTrace)), "l");
    grs.at(iTrace)->Print();
  }

  legend->Draw();
  canvas->Print("output/test.png");

  eFile->Close();

} // end main
