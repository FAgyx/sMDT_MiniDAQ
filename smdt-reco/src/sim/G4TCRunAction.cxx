#include "MuonSim/G4TCRunAction.h"

namespace MuonSim {

  G4int G4TCRunAction::H1NHits     = 0;
  G4int G4TCRunAction::H1DeltaPhi  = 1;
  G4int G4TCRunAction::H1IonEnergy = 2;
  G4int G4TCRunAction::H1dEdx      = 3;
  G4int G4TCRunAction::H1NRecoHits = 4;
  G4int G4TCRunAction::H2XYPos     = 0;


  G4TCRunAction::G4TCRunAction() : G4UserRunAction() {
    InitHistos();
  }

  G4TCRunAction::G4TCRunAction(MuonReco::ConfigParser &cp) : G4UserRunAction() {
    // do something with cp
    outpath = MuonReco::IOUtility::getMCOutputFilePath(cp.items("General").getInt("Run"));
    InitHistos();
  }

  void G4TCRunAction::InitHistos() {
    // book histograms, create ntuples, create analysis manager
    // todo
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    G4cout << "Using " << analysisManager->GetType() << G4endl;

    analysisManager->SetVerboseLevel(1);
    analysisManager->SetFileName(outpath);

    analysisManager->CreateH1("N hits", "Number of hits in event", 300, 0, 300);
    analysisManager->SetH1XAxisTitle(H1NHits, "Number of hits");
    analysisManager->SetH1YAxisTitle(H1NHits, "Number of events");

    analysisManager->CreateH1("DeltaPhi", "MC Truth scattering angle", 100, 0, 5);
    analysisManager->SetH1XAxisTitle(H1DeltaPhi, "Scattering angle (degrees)");
    analysisManager->SetH1YAxisTitle(H1DeltaPhi, "Number of events / 0.05 degrees");

    analysisManager->CreateH1("IonizationEnergy", "MC Truth Ionization energy of the event", 100, 0, 100);
    analysisManager->SetH1XAxisTitle(H1IonEnergy, "Ionization energy (keV)");
    analysisManager->SetH1YAxisTitle(H1IonEnergy, "Number of Events / 1 keV");

    analysisManager->CreateH1("dEdx", "MC Truth average dE/dx in tube volume", 100, 0, 20);
    analysisManager->SetH1XAxisTitle(H1dEdx, "dE/dx (keV/cm");
    analysisManager->SetH1YAxisTitle(H1dEdx, "number of events / (0.2 keV / cm)");

    analysisManager->CreateH1("nRecoHits", "Reconstructed number of hits", 10, 0, 10);
    analysisManager->SetH1XAxisTitle(H1NRecoHits, "Number of hits");
    analysisManager->SetH1YAxisTitle(H1NRecoHits, "Number of events");

    analysisManager->CreateH2("Hit XZ", "All Tube hits XZ", 50, 0., G4TestStandConstruction::columnSpacing
			      *G4TestStandConstruction::nTubesPerLayer,
			      50, 0., 2*G4TestStandConstruction::multiLayerSpacing);
  }

  G4TCRunAction::~G4TCRunAction() {
    delete G4AnalysisManager::Instance();
  }

  void G4TCRunAction::BeginOfRunAction(const G4Run* /*run*/) {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile();
  }

  void G4TCRunAction::EndOfRunAction(const G4Run* /*run*/) {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
    
    RootIO* rio = RootIO::GetInstance(0);
    rio->Write();
    rio->Close();
  }
}
