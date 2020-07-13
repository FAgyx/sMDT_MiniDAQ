#include "MuonSim/G4DriftTubeSD.h"

namespace MuonSim {
  
  G4String G4DriftTubeSD::myCollectionName = "driftTubeColl";

  G4DriftTubeSD::G4DriftTubeSD(G4String name) : G4VSensitiveDetector(name), fHitsCollection(0), fHCID(-1) {
    collectionName.insert(myCollectionName);
  }

  G4DriftTubeSD::~G4DriftTubeSD() {}

  void G4DriftTubeSD::Initialize(G4HCofThisEvent* hce) {
    fHitsCollection = new G4DriftTubeHitsCollection(SensitiveDetectorName,collectionName[0]);
    if (fHCID<0) {
      fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    }
    hce->AddHitsCollection(fHCID,fHitsCollection);
  }

  G4bool G4DriftTubeSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    G4double charge = step->GetTrack()->GetDefinition()->GetPDGCharge();
    if (charge==0.) return true; // we can only track charged particles!

    G4StepPoint* preStepPoint = step->GetPreStepPoint();

    G4TouchableHistory* touchable = (G4TouchableHistory*)(step->GetPreStepPoint()->GetTouchable());
    G4int layer  = touchable->GetVolume(2)->GetCopyNo();
    G4int column = touchable->GetReplicaNumber(1);

    G4VPhysicalVolume* motherPhysical = touchable->GetVolume(1); // mother
    G4int copyNo = motherPhysical->GetCopyNo();
    
    G4ThreeVector worldPos = preStepPoint->GetPosition();
    G4ThreeVector localPos = touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);

    G4double ionE = step->GetTotalEnergyDeposit() - step->GetNonIonizingEnergyDeposit();

    G4DriftTubeHit* hit = new G4DriftTubeHit(layer, column);
    hit->SetWorldPos(worldPos);
    hit->SetLocalPos(localPos);
    hit->SetTime(preStepPoint->GetGlobalTime());
    hit->SetIonizationEnergy(ionE);
    hit->SetStepLength(step->GetStepLength());
    
    fHitsCollection->insert(hit);


    return true;
  }
}
