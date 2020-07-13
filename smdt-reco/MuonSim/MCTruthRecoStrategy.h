#ifndef MuonSim_MCTruthRecoStrategy
#define MuonSim_MCTruthRecoStrategy

#include "globals.hh"

#include "MuonSim/ReconstructionStrategy.h"
#include "MuonSim/G4TestStandConstruction.h"

#include "MuonReco/Hit.h"

namespace MuonSim {
  class MCTruthRecoStrategy : public ReconstructionStrategy {
  public:
    MCTruthRecoStrategy(MuonReco::ConfigParser cp);
    virtual ~MCTruthRecoStrategy() {};

    virtual void addRecoHit(std::vector<G4DriftTubeHit*> truthHits) override;
  protected:
    G4double getTruthDistance(G4DriftTubeHit* hit);
  };
}

#endif
