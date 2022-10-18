#include "MuonReco/T0Reader.h"

namespace MuonReco {
  static T0Reader* instance = 0;
  
  T0Reader::T0Reader(TString t0path) {
    UpdatePath(t0path);
  }

  T0Reader::~T0Reader() { t0file->Close(); }

  void T0Reader::UpdatePath(TString t0path) {
    //if (t0file) t0file->Close();
    t0file = new TFile(t0path);
    fitData = 0;
    t0file->GetObject("FitData", fitData);    
  }

  T0Reader* T0Reader::GetInstance(TString t0path) {
    if (instance == 0) {
      instance = new T0Reader(t0path);
    }
    instance->UpdatePath(t0path);
    return instance;
  }

  void T0Reader::GetEntry(int iEntry) {
    fitData->GetEntry(iEntry);
  }

  int T0Reader::GetEntries() {
    return fitData->GetEntries();
  }

  void T0Reader::SetBranchAddresses(int* tdc, int* ch, int* layer, int* col, TVectorD* fitParams) {
    fitData->SetBranchAddress("tdc", tdc);
    fitData->SetBranchAddress("channel", ch);
    fitData->SetBranchAddress("layer", layer);
    fitData->SetBranchAddress("column", col);
    for (int i = 0; i < NT0FITDATA; i++) {
      fitData->SetBranchAddress(fitDataNames[i], &(*fitParams)[i]);
    }
  }
}
