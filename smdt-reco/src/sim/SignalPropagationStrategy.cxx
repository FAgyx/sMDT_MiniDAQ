#include "MuonSim/SignalPropagationStrategy.h"

namespace MuonSim {
  SignalPropagationStrategy::SignalPropagationStrategy(MuonReco::ConfigParser cp) : GasMonitorRTStrategy(cp) {

  }
  
  void SignalPropagationStrategy::addRecoHit(std::vector<G4DriftTubeHit*> truthHits) {
    if (truthHits.size() == 0) return;

    G4double minDistance = getMinHitDistance(truthHits);
    G4double hitTime     = findHitTime(minDistance);
    
    // signal propagation of this wire in meters
    G4double propDist = truthHits.at(0)->GetLocalPos().y()/CLHEP::m;
    G4double propTime = propDist/CLHEP::c_light/CLHEP::ns;
    hitTime = hitTime/CLHEP::ns - propTime; // now in ns
    minDistance = rtgraph2->Eval(hitTime);

    double hx, hy;
    geo.GetHitXY(truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), &hx, &hy);
    MuonReco::Hit h = MuonReco::Hit(hitTime, hitTime,
                                    hitTime, hitTime,
                                    0, 0, truthHits.at(0)->GetLayer(), 
				    truthHits.at(0)->GetColumn(), hx, hy);


    h.SetRadius(minDistance/CLHEP::mm);
    evt.AddSignalHit(h);

  }
}
