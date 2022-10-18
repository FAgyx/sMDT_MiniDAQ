#ifndef MuonSim_SmearPositionStrategy
#define MuonSim_SmearPositionStrategy

#include "Randomize.hh"

#include "MuonSim/MCTruthRecoStrategy.h"

namespace MuonSim {
  /*! class SmearPositionStrategy SmearPositionStrategy.h "MuonSim/SmearPositionStrategy.h"
   * 
   * Inherit from MCTruthRecoStrategy, but this time add a small amount of smearing to truth position 
   */
  class SmearPositionStrategy : public MCTruthRecoStrategy {
  public:
    SmearPositionStrategy(MuonReco::ConfigParser cp);
    virtual ~SmearPositionStrategy() {};
    
    virtual void addRecoHit(std::vector<G4DriftTubeHit*> truthHits) override;
  };
}

#endif
