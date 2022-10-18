#include "MuonSim/MCTruthRecoStrategy.h"

namespace MuonSim {
  /*! \brief Constructor
   *  Nothing extra needed, simply call superclass constructor
   */
  MCTruthRecoStrategy::MCTruthRecoStrategy(MuonReco::ConfigParser cp) : ReconstructionStrategy(cp) {
    geo = MuonReco::Geometry(cp);
  }


  /*! \brief Implementation of ReconstructionStrategy interface
   * 
   * Simply check all of the G4Step objects produced by charged particles in a tube and take the one with the 
   * lowest impact parameter.  This is the definition of the radius of a hit.
   */
  void MCTruthRecoStrategy::addRecoHit(std::vector<G4DriftTubeHit*> truthHits) {
    if (truthHits.size() == 0) return;

    G4double minDistance = getMinHitDistance(truthHits);

    double hx, hy;
    geo.GetHitXY(truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), &hx, &hy);
    MuonReco::Hit h = MuonReco::Hit(minDistance/CLHEP::mm, minDistance/CLHEP::mm, 
				    minDistance/CLHEP::mm, minDistance/CLHEP::mm, 
				    0, 0, truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), hx, hy);
    h.SetRadius(minDistance/CLHEP::mm);
    evt.AddSignalHit(h);


  }

  /*! \brief Find lowest impact parameter out of all hits
   */
  G4double MCTruthRecoStrategy::getMinHitDistance(std::vector<G4DriftTubeHit*> truthHits) {
    G4double minDistance  = G4TestStandConstruction::pRMin;
    G4double thisDistance;
    for (G4DriftTubeHit* hit : truthHits) {
      thisDistance = getTruthDistance(hit);
      if (thisDistance < minDistance) minDistance = thisDistance;
    }
    return minDistance;
  }

  /*! \brief Calculate impact parameter of this simulated hit 
   */
  G4double MCTruthRecoStrategy::getTruthDistance(G4DriftTubeHit* hit) {    
    G4ThreeVector dir = hit->GetDirection();
    G4ThreeVector pos = hit->GetLocalPos();
    G4ThreeVector end = pos + dir;

    if (abs(dir.x()) < CLHEP::um) return abs(pos.x()); // has neglible error and will prevent numerical errors later

    double slope     = dir.y()/dir.x();
    double intercept = pos.y() - slope*pos.x();
    double y_min     = intercept / (slope*slope + 1);
    
    if ((y_min > pos.y() && y_min > end.y()) || (y_min < pos.y() && y_min < end.y())) {
      // we have extrapolated beyond the extent of this segment so return the true minimum of finite segment
      double r_pos = get2dDistance(pos);
      double r_end = get2dDistance(end);
      return (r_pos < r_end) ? r_pos : r_end;
    }
    return abs(intercept)/sqrt(slope*slope+1);
  }

  /*! Convert a three-vector (in tube coordinate system) to a drift distance (we lose z direction information) 
   */
  G4double MCTruthRecoStrategy::get2dDistance(G4ThreeVector localPos) {
    return sqrt(localPos.x()*localPos.x() + localPos.y()*localPos.y());
  }
}
