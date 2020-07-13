#include "MuonReco/Event.h"

namespace MuonReco {

  Event::Event() {
    trigs    = std::vector<Signal>();
    sigs     = std::vector<Signal>();
    trigHits = std::vector<Hit>();
    sigHits  = std::vector<Hit>();
    clusters = std::vector<Cluster>();
    tracks   = std::vector<Track>();
    nTrigs   = 0;
    nSigs    = 0;
    id       = 0;
  }

  Event::Event(std::vector<Signal> triggers, std::vector<Signal> signals, EventID eID) {
    trigs    = triggers;
    sigs     = signals;
    trigHits = std::vector<Hit>();
    sigHits  = std::vector<Hit>();
    clusters = std::vector<Cluster>();
    tracks   = std::vector<Track>();
    id       = eID.ID();
    nTrigs   = 0;
    nSigs    = 0;
  }

  Event::Event(std::vector<Signal> triggers, std::vector<Signal> signals, std::vector<Hit> wHits, std::vector<Hit>    tHits,    std::vector<Cluster> clusts, std::vector<Track> trks, unsigned long ID) {
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
  
  std::vector<Signal> Event::WireSignals() const {
    return sigs;
  }
  
  std::vector<Signal> Event::TrigSignals() const {
    return trigs;
  }
  
  std::vector<Hit> Event::TriggerHits() const {
    return trigHits;
  }

  std::vector<Hit> Event::WireHits() const {
    return sigHits;
  }

  std::vector<Cluster> Event::Clusters() const {
    return clusters;
  }

  std::vector<Cluster> Event::mClusters() {
    return clusters;
  }

  std::vector<Track> Event::Tracks() const {
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


