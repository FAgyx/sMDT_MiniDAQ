#ifndef MUON_EVENT
#define MUON_EVENT

#include <vector>

#include "MuonReco/RecoObject.h"
#include "MuonReco/Signal.h"
#include "MuonReco/EventID.h"
#include "MuonReco/Hit.h"
#include "MuonReco/Cluster.h"
#include "MuonReco/Track.h"


namespace MuonReco {
  
  /*! \class Event Event.h "MuonReco/Event.h"
   *
   * \brief Event is a container for all reconstructed objects in the reco chain for sMDT chambers
   *
   * A CINT dictionary is generated for Event, as for all objects inheriting from RecoObject, which allows the user to write Event objects to a TTree for easy storage
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   May 21 2019
   */
  class Event : public RecoObject {
  public:
    Event();
    Event(std::vector<Signal> triggers, std::vector<Signal> signals, EventID eID);
    Event(std::vector<Signal> triggers, std::vector<Signal> signals, std::vector<Hit> wHits,
          std::vector<Hit>    tHits,    std::vector<Cluster> clusts, std::vector<Track> trks,
          unsigned long ID);
    Event(const Event &e);

    std::vector<Signal>  TrigSignals() const;
    std::vector<Signal>  WireSignals() const;
    std::vector<Hit>     WireHits   () const;
    std::vector<Hit>     TriggerHits() const;
    std::vector<Cluster> Clusters   () const;
    std::vector<Track>   Tracks     () const;
    unsigned long        ID         () const;
    bool               Pass       () const;

    std::vector<Cluster> mClusters();

    void   AddSignalHit (Hit h);
    void   AddTriggerHit(Hit h);
    void   update       ();
    void   SetPassCheck (bool b);
    void   AddCluster   (Cluster c);
    void   AddTrack     (Track t);
    bool AnyWireHit   (unsigned int TDC, unsigned int Channel);

    // temporary
    void CheckClusterTime();

    void Draw() override;

  private:
    unsigned long        id;
    std::vector<Signal>  trigs;
    std::vector<Signal>  sigs;
    std::vector<Hit>     trigHits;
    std::vector<Hit>     sigHits;
    std::vector<Cluster> clusters;
    std::vector<Track>   tracks;
    int                  nTrigs        = 0;
    int                  nSigs         = 0;
    bool               pass          = kFALSE;
    bool               hasBadHitTime = kFALSE;
  };
}

#endif
