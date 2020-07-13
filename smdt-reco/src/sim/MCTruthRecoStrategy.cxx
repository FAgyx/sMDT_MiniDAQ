#include "MuonSim/MCTruthRecoStrategy.h"

namespace MuonSim {
  MCTruthRecoStrategy::MCTruthRecoStrategy(MuonReco::ConfigParser cp) : ReconstructionStrategy(cp) {}

  void MCTruthRecoStrategy::addRecoHit(std::vector<G4DriftTubeHit*> truthHits) {
    if (truthHits.size() == 0) return;

    G4double minDistance  = G4TestStandConstruction::pRMin;
    G4double thisDistance = 0;
    for (G4DriftTubeHit* hit : truthHits) {
      thisDistance = getTruthDistance(hit);
      if (thisDistance < minDistance) minDistance = thisDistance;
    }

    double hx, hy;
    MuonReco::Geometry::GetHitXY(truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), &hx, &hy);
    MuonReco::Hit h = MuonReco::Hit(0, 0, 0, 0, 0, 0, truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), hx, hy);
    //h.SetRadius(getTruthDistance/CLHEP::mm);
    evt.AddSignalHit(h);
  }

  G4double MCTruthRecoStrategy::getTruthDistance(G4DriftTubeHit* hit) {
    // todo
    return 1.0 * CLHEP::mm;
  }

}
