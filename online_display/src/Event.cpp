#include "src/RecoObject.cpp"
#include "src/Signal.cpp"
#include "src/EventID.cpp"
#include "src/Hit.cpp"
#include "src/Cluster.cpp"
#include "src/Track.cpp"

#ifndef MUON_EVENT
#define MUON_EVENT

namespace Muon {

  /***********************************************
   * Event is a container for all reconstructed  *
   * objects in the reco chain for sMDT chambers *
   *                                             *
   * Author: Kevin Nelson                        *
   * Date:   May    2019                         *
   * Update: May 21 2019                         *
   ***********************************************
   */
  class Event : public RecoObject {
  public:
    Event();
    Event(vector<Signal> triggers, vector<Signal> signals, EventID eID);
    Event(vector<Signal> triggers, vector<Signal> signals, vector<Hit> wHits,
	  vector<Hit>    tHits,    vector<Cluster> clusts, vector<Track> trks,
	  unsigned long ID);
    Event(const Event &e);

    vector<Signal>  TrigSignals() const;
    vector<Signal>  WireSignals() const;
    vector<Hit>     WireHits   () const;
    vector<Hit>     TriggerHits() const;
    vector<Cluster> Clusters   () const;
    vector<Track>   Tracks     () const;
    unsigned long   ID         () const;
    Bool_t          Pass       () const;

    vector<Cluster> mClusters();

    void   AddSignalHit (Hit h);
    void   AddTriggerHit(Hit h);
    void   update       ();
    void   SetPassCheck (Bool_t b);
    void   AddCluster   (Cluster c);
    void   AddTrack     (Track t);
    Bool_t AnyWireHit   (unsigned int TDC, unsigned int Channel);

    // temporary
    void CheckClusterTime();


    void Draw() override;

  private:
    unsigned long   id;
    vector<Signal>  trigs;
    vector<Signal>  sigs;
    vector<Hit>     trigHits;
    vector<Hit>     sigHits;
    vector<Cluster> clusters;
    vector<Track>   tracks;
    int             nTrigs        = 0;
    int             nSigs         = 0;
    Bool_t          pass          = kFALSE;
    Bool_t          hasBadHitTime = kFALSE;
  };

  Event::Event() {
    trigs    = vector<Signal>();
    sigs     = vector<Signal>();
    trigHits = vector<Hit>();
    sigHits  = vector<Hit>();
    clusters = vector<Cluster>();
    tracks   = vector<Track>();
    nTrigs   = 0;
    nSigs    = 0;
    id       = 0;
  }

  Event::Event(vector<Signal> triggers, vector<Signal> signals, EventID eID) {
    trigs    = triggers;
    sigs     = signals;
    trigHits = vector<Hit>();
    sigHits  = vector<Hit>();
    clusters = vector<Cluster>();
    tracks   = vector<Track>();
    id       = eID.ID();
    nTrigs   = 0;
    nSigs    = 0;
  }

  Event::Event(vector<Signal> triggers, vector<Signal> signals, vector<Hit> wHits, vector<Hit>    tHits,    vector<Cluster> clusts, vector<Track> trks, unsigned long ID) {
    trigs    = triggers;
    sigs     = signals;
    trigHits = tHits;
    sigHits  = wHits;
    clusters = clusts;
    tracks   = trks;
    id       = ID;
    update();
  }

  Event::Event(const Event &e) {
    trigs    = e.TrigSignals();
    sigs     = e.WireSignals();
    trigHits = e.TriggerHits();
    sigHits  = e.WireHits();
    clusters = e.Clusters();
    tracks   = e.Tracks();
    id       = e.ID();
    update();
  }
  
  vector<Signal> Event::WireSignals() const {
    return sigs;
  }
  
  vector<Signal> Event::TrigSignals() const {
    return trigs;
  }
  
  vector<Hit> Event::TriggerHits() const {
    return trigHits;
  }

  vector<Hit> Event::WireHits() const {
    return sigHits;
  }

  vector<Cluster> Event::Clusters() const {
    return clusters;
  }

  vector<Cluster> Event::mClusters() {
    return clusters;
  }

  vector<Track> Event::Tracks() const {
    return tracks;
  }

  unsigned long Event::ID() const {
    return id;
  }

  Bool_t Event::Pass() const {
    return pass;
  }

  void Event::AddTriggerHit(Hit h) {
    trigHits.push_back(h);
    nTrigs++;
  }

  void Event::AddSignalHit(Hit h) {
    sigHits.push_back(h);
    nSigs++;
  }

  void Event::SetPassCheck(Bool_t b) {
    pass = b;
  }
  
  void Event::AddCluster(Cluster c) {
    clusters.push_back(c);
  }

  void Event::AddTrack(Track t) {
    tracks.push_back(t);
  }

  void Event::update() {
    nTrigs = trigHits.size();
    nSigs  = sigHits.size();
  }

  void Event::CheckClusterTime() {
    for (Cluster c : clusters) {
      for (Hit h : c.Hits()) {
	if (h.CorrTime()>200 || h.CorrTime()<-200) {
	  hasBadHitTime = kTRUE;
	  return;
	}
      }
    }
  }

  Bool_t Event::AnyWireHit(unsigned int TDC, unsigned int Channel) {
    for (Hit h : sigHits) {
      if (h.TDC() == TDC && h.Channel() == Channel)
	return kTRUE;
    }
    return kFALSE;
  }

  void Event::Draw() {}
}

#endif
