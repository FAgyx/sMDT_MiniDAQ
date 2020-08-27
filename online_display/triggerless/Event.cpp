#include "Signal.cpp"
#include "Hit.cpp"


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
  class Event : {
  public:
    Event();
    Event(vector<Hit> wHits, vector<Cluster> clusts, vector<Track> trks);
    Event(const Event &e);
    vector<Hit>     WireHits   () const;
    vector<Cluster> Clusters   () const;
    vector<Track>   Tracks     () const;
    bool            Pass       () const;

    vector<Cluster> mClusters();

    void   AddSignalHit (Hit h);
    void   SetPassCheck (bool b);
    void   AddCluster   (Cluster c);
    void   AddTrack     (Track t);
    bool   AnyWireHit   (int TDC, int Channel);

    // temporary
    void   CheckClusterTime();
  private:
    vector<Hit>     sigHits;
    vector<Cluster> clusters;
    vector<Track>   tracks;
    bool            pass          = false;
    bool            hasBadHitTime = false;
  };

  Event::Event() {
    sigHits  = vector<Hit>();
    clusters = vector<Cluster>();
    tracks   = vector<Track>();
  }

  Event::Event(vector<Hit> wHits) {
    sigHits  = wHits;
    clusters = vector<Cluster>();
    tracks   = vector<Track>();
  }



  Event::Event(const Event &e) {
    sigHits  = e.WireHits();
    clusters = e.Clusters();
    tracks   = e.Tracks();
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


  bool Event::Pass() const {
    return pass;
  }


  void Event::AddSignalHit(Hit h) {
    sigHits.push_back(h);
  }

  void Event::SetPassCheck(bool b) {
    pass = b;
  }
  
  void Event::AddCluster(Cluster c) {
    clusters.push_back(c);
  }

  void Event::AddTrack(Track t) {
    tracks.push_back(t);
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

  bool Event::AnyWireHit(unsigned int TDC, unsigned int Channel) {
    for (Hit h : sigHits) {
      if (h.TDC() == TDC && h.Channel() == Channel)
        return kTRUE;
      }
    return false;
  }

}

#endif
