#include "MuonSim/GasMonitorRTStrategy.h"

namespace MuonSim {
  /*! \brief Constructor
   *  Load the r(t) function information from the file Rt_BMG_6_1.dat
   */
  GasMonitorRTStrategy::GasMonitorRTStrategy(MuonReco::ConfigParser cp) : MCTruthRecoStrategy(cp) {

    double radius, time, error;
    std::vector<double> v_radius, v_time, v_error = std::vector<double>();
    std::ifstream infile(MuonReco::IOUtility::getMDTRTFilePath().Data());
    std::string line;
    int lineNum = 0;
    while (getline(infile, line)) {
      std::istringstream iss(line);
      if (lineNum != 0) {
        if (!(iss >> radius >> time >> error)) { break; }
        v_radius.push_back(radius);
        v_time  .push_back(time);
        v_error .push_back(error);
      }
      lineNum++;
    }

    rtgraph = new TGraph(v_radius.size(), &v_radius[0], &v_time[0]);
    rtgraph2 = new TGraph(v_radius.size(), &v_time[0], &v_radius[0]);

  }

  /*! \brief Implementation of the ReconstructionStrategy interface
   * 
   *  The same as the MCTruthRecoStrategy, but uses the r(t) function to find the drift time 
   */
  void GasMonitorRTStrategy::addRecoHit(std::vector<G4DriftTubeHit*> truthHits) {
    if (truthHits.size() == 0) return;

    G4double minDistance = getMinHitDistance(truthHits);
    G4double hitTime     = findHitTime(minDistance);

    double hx, hy;
    geo.GetHitXY(truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), &hx, &hy);    
    MuonReco::Hit h = MuonReco::Hit(hitTime/CLHEP::ns, hitTime/CLHEP::ns,
                                    hitTime/CLHEP::ns, hitTime/CLHEP::ns,
                                    0, 0, truthHits.at(0)->GetLayer(), truthHits.at(0)->GetColumn(), hx, hy);
    h.SetRadius(minDistance/CLHEP::mm);
    evt.AddSignalHit(h);
  }

  /*! Using inverse r(t) function find the true hit time
   */
  G4double GasMonitorRTStrategy::findHitTime(G4double hitR) {
    return rtgraph->Eval(hitR/CLHEP::mm)*CLHEP::ns;
  }
}
