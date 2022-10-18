#ifndef MUON_RECO_OBJECT
#define MUON_RECO_OBJECT

namespace MuonReco {

  /*! \class RecoObject RecoObject.h "MuonReco/RecoObject.h"
   *  \brief Common interface for physics objects, with a drawable interface for event displays
   *
   *  \author Kevin Nelson
   *          kevin.nelson@cern.ch
   *  \date   6 July 2020
   */
  class RecoObject {
  public:
    virtual void Draw() = 0;
  };
}

#endif
