#include "MuonSim/ReconstructionStrategy.h"

namespace MuonSim {

  ReconstructionStrategy::ReconstructionStrategy(MuonReco::ConfigParser cp) {
    ru  = MuonReco::RecoUtility(cp.items("RecoUtility"));
    evt = MuonReco::Event();
  }

  MuonReco::Event* ReconstructionStrategy::getEvent() {
    ru.DoHitClustering(&evt);
    if (ru.CheckEvent(evt))
      return &evt;
    else return 0;
  }

  void ReconstructionStrategy::clearEvent() {
    evt = MuonReco::Event();
  }

}
