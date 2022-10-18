#ifndef MuonSim_MCTruthRecoStrategy
#define MuonSim_MCTruthRecoStrategy

#include "globals.hh"

#include "MuonSim/ReconstructionStrategy.h"
#include "MuonSim/G4TestStandConstruction.h"

#include "MuonReco/Hit.h"
#include "MuonReco/Geometry.h"

namespace MuonSim {
  /*! class MCTruthRecoStrategy MCTruthRecoStrategy.h "MuonSim/MCTruthRecoStrategy.h"
   * 
   * Implement the ReconstructionStrategy interface to use only MC truth information
   * to reconstruct a MuonReco::Event object
   */
  class MCTruthRecoStrategy : public ReconstructionStrategy {
  public:
    MCTruthRecoStrategy(MuonReco::ConfigParser cp);
    virtual ~MCTruthRecoStrategy() {};

    virtual void addRecoHit(std::vector<G4DriftTubeHit*> truthHits) override;

    MuonReco::Geometry geo;

  protected:
    G4double getMinHitDistance(std::vector<G4DriftTubeHit*> truthHits);
    G4double getTruthDistance (G4DriftTubeHit* hit);
    G4double get2dDistance    (G4ThreeVector localPos);
  };
}

#endif
