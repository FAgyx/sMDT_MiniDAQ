#include "MuonSim/RootIO.h"

namespace MuonSim {
  static RootIO* instance = 0;

  RootIO::RootIO(int runN) {

    eFile = new TFile(MuonReco::IOUtility::getMCEventTreeFilePath(runN), "RECREATE");
    eTree = new TTree("eTree", "eTree");
  }

  RootIO::~RootIO() {}

  RootIO* RootIO::GetInstance(int runN) {
    if (instance == 0) {
      instance = new RootIO(runN);
    }
    return instance;
  }

  void RootIO::SetTarget(MuonReco::Event* evt) {
    eTree->Branch("event", "Event", evt);
  }

  void RootIO::Fill() {
    eTree->Fill();
  }

  void RootIO::Write() {
    eFile->Write();
  }

  void RootIO::Close() {
    eFile->Close();
  }

}
