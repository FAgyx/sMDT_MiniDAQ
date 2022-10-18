#ifndef MuonSim_ReconstructionStrategy
#define MuonSim_ReconstructionStrategy

#include "MuonSim/G4DriftTubeHit.h"

#include "MuonReco/Event.h"
#include "MuonReco/RecoUtility.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/Geometry.h"

namespace MuonSim {
  /*! \class ReconstructionStrategy ReconstructionStrategy.h "MuonSim/ReconstructionStrategy.h"
   *
   *  Abstract class to represent a method of adding hits to a reconstructed event in the Geant4
   *  simulation.
   *  The pure virtual method addRecoHit must be overridden by a derived class                                     
   *  This class manages the configuration of the MuonReco::RecoUtility, which clusters the hits
   *  and cuts events based on a series of criteria derived from the configuration file                            
   */
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
