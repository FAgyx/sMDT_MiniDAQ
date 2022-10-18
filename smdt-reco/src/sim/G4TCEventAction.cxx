#include "MuonSim/G4TCEventAction.h"

namespace MuonSim {
  G4TCEventAction::G4TCEventAction(MuonReco::ConfigParser cp) : G4UserEventAction(), fHCID(-1) {
    G4RunManager::GetRunManager()->SetPrintProgress(1);

    runNumber = cp.items("General").getInt("RunNumber");
    if (!cp.items("General").getStr("Strategy").CompareTo("MCTruth")) {
      reco = new MCTruthRecoStrategy(cp);
    }
    else if (!cp.items("General").getStr("Strategy").CompareTo("SmearPosition")) {
      reco = new SmearPositionStrategy(cp);
    }
    else if (!cp.items("General").getStr("Strategy").CompareTo("GasMonitorRT")) {
      reco = new GasMonitorRTStrategy(cp);
    }
    else if (!cp.items("General").getStr("Strategy").CompareTo("SignalPropagation")) {
      reco = new SignalPropagationStrategy(cp);
    }
    else {
      G4ExceptionDescription msg;
      msg << "The configuration file does not specify a reconstruction strategy." << G4endl 
	  << "Acceptable options include: MCTruth" << G4endl
	  << "                            SmearPosition" << G4endl
	  << "                            GasMonitorRT" << G4endl;
      G4Exception("", "Code001", FatalException, msg);
    }

    RootIO* rio = RootIO::GetInstance(runNumber);
    rio->SetTarget(&reco->evt);
  }
  
  G4TCEventAction::~G4TCEventAction() {}
  
  void G4TCEventAction::BeginOfEventAction(const G4Event*) {
    if (fHCID==-1) {
      G4SDManager* sdManager = G4SDManager::GetSDMpointer();
      fHCID = sdManager->GetCollectionID("driftTube/driftTubeColl");
    }
  }
  
  void G4TCEventAction::EndOfEventAction(const G4Event* event) {
    if (reco) reco->clearEvent();

    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    if (!hce) {
      G4ExceptionDescription msg;
      msg << "No hits colleciton of this event found." << G4endl;
      G4Exception("G4TCEventAction::EndOfEventAction()", "Code001", JustWarning, msg);
      return;
    }

    // get hit collections
    G4DriftTubeHitsCollection* dHCTubes = static_cast<G4DriftTubeHitsCollection*>(hce->GetHC(fHCID));


    if (!dHCTubes) {
      G4ExceptionDescription msg;
      msg << "Some of the hit collections of this event not found." << G4endl;
      G4Exception("G4TCEventAction::EndOfEventAction()", "Code001", JustWarning, msg);
      return;
    }

    
    // fill histograms and ntuples
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    
    G4int n_hit = dHCTubes->entries();
    analysisManager->FillH1(G4TCRunAction::H1NHits, n_hit);

    G4double totalIonE = 0;
    G4double totaldx   = 0;
    
    //
    // Analyze each hit
    //

    for (G4int i=0; i<n_hit; i++) {
      G4DriftTubeHit* hit = (*dHCTubes)[i];
      G4ThreeVector worldPos = hit->GetWorldPos();
      analysisManager->FillH2(G4TCRunAction::H2XYPos, worldPos.x(), worldPos.z());
      totalIonE += hit->GetIonizationEnergy()/CLHEP::keV;
      totaldx   += hit->GetStepLength()/CLHEP::cm;
    }
    if (totaldx != 0) {
      analysisManager->FillH1(G4TCRunAction::H1IonEnergy, totalIonE);
      analysisManager->FillH1(G4TCRunAction::H1dEdx, totalIonE/totaldx);
    }

    G4int nTubesHit = 0;

    // group hits on a particular wire and find the drift distance
    for (G4int iLayer = 0; iLayer < G4TestStandConstruction::nLayersPerMultiLayer*
	   G4TestStandConstruction::nMultiLayers; iLayer++) {
      for (G4int iColumn = 0; iColumn < G4TestStandConstruction::nTubesPerLayer; iColumn++) {
	std::vector<G4DriftTubeHit*> hitsOnThisWire = std::vector<G4DriftTubeHit*>();
	for (G4int iHit = 0; iHit < n_hit; iHit++) {
	  G4DriftTubeHit* hit = (*dHCTubes)[iHit];
	  if (hit->GetLayer() == iLayer && hit->GetColumn() == iColumn)
	    hitsOnThisWire.push_back(hit);
	}

	if (reco && hitsOnThisWire.size() > 0) reco->addRecoHit(hitsOnThisWire);

	// hitsOnThisWire represents all charged particle motion in this tube
	// We need to turn this into a single reconstructed hit object
	if (hitsOnThisWire.size() != 0) nTubesHit++;

      } // end for: iColumn
    } // end for: iLayer


    MuonReco::Event* evt = 0;
    if (reco) evt = reco->getEvent();

    if (reco && evt) {
      G4cout << "Successfully reconstructed an event with : " 
	     << evt->WireHits().size() << " hits" << G4endl;
      RootIO* rio = RootIO::GetInstance(0);
      evt->SetPassCheck(1);      
      rio->Fill();
      analysisManager->FillNtupleIColumn(G4TCRunAction::NEventPass, 1);
    }
    else analysisManager->FillNtupleIColumn(G4TCRunAction::NEventPass, 0);

    analysisManager->FillH1(G4TCRunAction::H1NRecoHits, nTubesHit);

    analysisManager->AddNtupleRow();
  }
}
