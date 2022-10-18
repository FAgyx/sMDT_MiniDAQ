#ifndef MuonSim_GasMonitorRTStrategy
#define MuonSim_GasMonitorRTStrategy

#include "MuonReco/ConfigParser.h"
#include "MuonReco/IOUtility.h"
#include "MuonReco/Geometry.h"

#include "MuonSim/G4DriftTubeHit.h"
#include "MuonSim/MCTruthRecoStrategy.h"

#include "TGraph.h"

namespace MuonSim {
  /*! class GasMonitorRTStrategy GasMonitorRTStrategy.h "MuonSim/GasMonitorRTStrategy.h"
   * 
   * Implementation of ReconstructionStrategy interface to use the gas monitor
   * RT function as a MC truth RT function.  (see main page for download instructions for Rt_BMG_6_1.dat)
   */
  class GasMonitorRTStrategy : public MCTruthRecoStrategy {
  public:
    GasMonitorRTStrategy(MuonReco::ConfigParser cp);
    virtual ~GasMonitorRTStrategy() {if (rtgraph) delete rtgraph; if (rtgraph2) delete rtgraph2;}
    
    virtual void addRecoHit(std::vector<G4DriftTubeHit*> truthHits) override;

    TGraph* rtgraph  = 0;
    TGraph* rtgraph2 = 0;

  protected:
    G4double findHitTime(G4double hitR);
  };
}

#endif
