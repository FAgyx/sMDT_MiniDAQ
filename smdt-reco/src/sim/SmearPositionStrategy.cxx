#include "MuonSim/SmearPositionStrategy.h"

namespace MuonSim {
  /*! \brief Constructor
   * Call MCTruthRecoStrategy constructor    
   */
  SmearPositionStrategy::SmearPositionStrategy(MuonReco::ConfigParser cp) : MCTruthRecoStrategy(cp) {}

  /*! \brief Implementation of ReconstructionStrategy interface
   * 
   * Add gaussian noise to truth position.  For truth position calculation, see MCTruthRecoStrategy::addRecoHit
   */
  void SmearPositionStrategy::addRecoHit(std::vector<G4DriftTubeHit*> truthHits) {
    if (truthHits.size() == 0) return;

    G4double minDistance = getMinHitDistance(truthHits);
    
    double hx, hy;
    geo.GetHitXY(truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), &hx, &hy);
    MuonReco::Hit h = MuonReco::Hit(minDistance/CLHEP::mm, minDistance/CLHEP::mm,
                                    minDistance/CLHEP::mm, minDistance/CLHEP::mm,
				    0, 0, truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), hx, hy);
    // add radius dependent gaussian noise to the hit radius
    h.SetRadius(minDistance/CLHEP::mm + MuonReco::Hit::RadiusError(minDistance/CLHEP::mm)*G4RandGauss::shoot(0,1));
    evt.AddSignalHit(h);
  }

}
