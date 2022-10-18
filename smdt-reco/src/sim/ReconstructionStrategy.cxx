#include "MuonSim/ReconstructionStrategy.h"

namespace MuonSim {

  /*! Constructor.  Instantiates the MuonReco::RecoUtility and an empty event
   *  
   *  \param[in] cp MuonReco::ConfigParser describing the run configurations 
   */
  ReconstructionStrategy::ReconstructionStrategy(MuonReco::ConfigParser cp) {
    ru  = MuonReco::RecoUtility(cp.items("RecoUtility"));
    evt = MuonReco::Event();
  }

  /*! Return a pointer to the underlying event 
   *  Before pointer is returned, call MuonReco::RecoUtility::CheckEvent
   *  on the event.  If the event does not pass the defined cuts, return 0
   *  \return Pointer to MuonReco::Event object if the event passes, otherwise 0
   */
  MuonReco::Event* ReconstructionStrategy::getEvent() {
    int status = 0;
    ru.DoHitClustering(&evt);
    if (ru.CheckEvent(evt, &status))
      return &evt;
    else return 0;
  }

  /*! Clear the underlying event
   */
  void ReconstructionStrategy::clearEvent() {
    evt = MuonReco::Event();
  }

}
