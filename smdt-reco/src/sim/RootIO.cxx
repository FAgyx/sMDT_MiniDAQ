#include "MuonSim/RootIO.h"

namespace MuonSim {
  static RootIO* instance = 0;

  /*! \brief Constructor
   *  Indexed by run number in order to get the output path correctly using IOUtility
   */
  RootIO::RootIO(int runN) {

    eFile = new TFile(MuonReco::IOUtility::getDecodedOutputFilePath(runN), "RECREATE");
    eTree = new TTree("eTree", "eTree");
  }

  /*! Destructor */
  RootIO::~RootIO() {}

  /*! \brief Return the global pointer, if not defined then instantiate it */
  RootIO* RootIO::GetInstance(int runN) {
    if (instance == 0) {
      instance = new RootIO(runN);
    }
    return instance;
  }

  /*! Point the underlying TTree to an Event object (managed elsewhere) 
   */
  void RootIO::SetTarget(MuonReco::Event* evt) {
    eTree->Branch("event", "Event", evt);
  }

  /*! Fill the underlying tree */
  void RootIO::Fill() {
    eTree->Fill();
  }

  /*! Write out the underlying file */
  void RootIO::Write() {
    eFile->Write();
  }

  /*! Close the underlying file (frees memory) */
  void RootIO::Close() {
    eFile->Close();
  }

}
