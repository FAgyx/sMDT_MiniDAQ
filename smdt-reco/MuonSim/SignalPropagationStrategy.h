#ifndef MuonSim_SignalPropagationStrategy
#define MuonSim_SignalPropagationStrategy

#include "MuonSim/GasMonitorRTStrategy.h"

#include "MuonReco/ConfigParser.h"

#include "TF1.h"

namespace MuonSim {
  /*! class SignalPropagationStrategy SignalPropagationStrategy.h "MuonSim/SignalPropagationStrategy.h"
   *
   * Implementation of ReconstructionStrategy interface to smear hits by signal propagation time
   */
  class SignalPropagationStrategy : public GasMonitorRTStrategy {
  public:
    SignalPropagationStrategy(MuonReco::ConfigParser cp);
    virtual ~SignalPropagationStrategy() {}

    virtual void addRecoHit(std::vector<G4DriftTubeHit*> truthHits) override;
  };
}

#endif
