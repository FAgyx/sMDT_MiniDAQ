#ifndef MuonSim_ReconstructionStrategy
#define MuonSim_ReconstructionStrategy

#include "MuonSim/G4DriftTubeHit.h"

#include "MuonReco/Event.h"
#include "MuonReco/RecoUtility.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/Geometry.h"

namespace MuonSim {
  class ReconstructionStrategy {
  public:
    ReconstructionStrategy(MuonReco::ConfigParser cp);
    virtual ~ReconstructionStrategy() {}
        
    MuonReco::Event* getEvent();
    void             clearEvent();

    virtual void addRecoHit(std::vector<G4DriftTubeHit*> truthHits) = 0;

    MuonReco::Event       evt;
    MuonReco::RecoUtility ru;
    
    friend class G4TCEventAction;
  };
}
#endif
