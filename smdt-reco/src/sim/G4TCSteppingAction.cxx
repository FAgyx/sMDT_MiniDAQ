#include "MuonSim/G4TCSteppingAction.h"

namespace MuonSim {
  G4TCSteppingAction::G4TCSteppingAction() {}

  G4TCSteppingAction::~G4TCSteppingAction() {}

  void G4TCSteppingAction::UserSteppingAction(const G4Step* astep) {
    G4Track* aTrack = astep->GetTrack();
    const G4DynamicParticle* aDynamicParticle = aTrack->GetDynamicParticle();
    G4ParticleDefinition* aParticle = aTrack->GetDefinition();
    
    // get any muon leaving the world
    if (aTrack->GetNextVolume() == 0 && aParticle->GetPDGEncoding() == 13) {
      G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

      const G4ThreeVector initialDir = aTrack->GetVertexMomentumDirection();
      const G4ThreeVector finalDir   = aDynamicParticle->GetMomentumDirection();

      analysisManager->FillH1(G4TCRunAction::H1DeltaPhi, initialDir.angle(finalDir)/CLHEP::deg);
      analysisManager->FillNtupleDColumn(G4TCRunAction::NScatteringAngle, initialDir.angle(finalDir)/CLHEP::deg);
    }
  }
}
