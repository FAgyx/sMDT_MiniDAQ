#include "triggerless/Signal.cpp"
#include "triggerless/Hit.cpp"
#include "triggerless/Event.cpp"
#ifndef MUON_SEPARATOR
#define MUON_SEPARATOR

namespace Muon {

  /***********************************************
   * Separator is a container for all reconstructed  *
   * objects in the reco chain for sMDT chambers *
   *                                             *
   * Author: Kevin Nelson                        *
   * Date:   May    2019                         *
   * Update: May 21 2019                         *
   ***********************************************
   */
  class Separator {
  public:
    Separator();
    Separator(vector<Signal> triggers, vector<Signal> ledge_sigs, vector<Signal> tedge_sigs);
    Separator(const Separator &e);

    vector<Signal>  TrigSignals () const;
    vector<Signal>  LEdgeSignals() const;
    vector<Signal>  TEdgeSignals() const;
    vector<Hit>     TriggerHits () const;
    vector<Event>   Events      () const;
    unsigned int    ID          () const;


    void   AddTriggerHit(Hit h);
    void   AddEvent(Event e);

  private:
    unsigned int    id;
    vector<Signal>  trigs;
    vector<Signal>  ledgesigs;
    vector<Signal>  tedgesigs;
    vector<Hit>     trigHits;
    vector<Event>   events;
  };

  Separator::Separator() {
    id       = 0;
    trigs    = vector<Signal>();
    ledgesigs= vector<Signal>();
    tedgesigs= vector<Signal>();
    trigHits = vector<Hit>();
    events   = vector<Event>();
  }

  Separator::Separator(vector<Signal> triggers, vector<Signal> ledge_sigs, vector<Signal> tedge_sigs) {
    id       = 0;
    trigs    = triggers;
    ledgesigs= ledge_sigs;
    tedgesigs= tedge_sigs;
    trigHits = vector<Hit>();
    events   = vector<Event>();
  }

  Separator::Separator(const Separator &s) {
    id       = s.ID();
    trigs    = s.TrigSignals();
    ledgesigs= s.LEdgeSignals(); 
    tedgesigs= s.TEdgeSignals();   
    trigHits = s.TriggerHits();
    events   = s.Events();
  }
  
  vector<Signal> Separator::LEdgeSignals() const {
    return ledgesigs;
  }

  vector<Signal> Separator::TEdgeSignals() const {
    return tedgesigs;
  }
  
  vector<Signal> Separator::TrigSignals() const {
    return trigs;
  }  

  vector<Hit> Separator::TriggerHits() const {
    return trigHits;
  }

  vector<Event> Separator::Events() const {
    return events;
  }

  void Separator::AddTriggerHit(Hit h) {
    trigHits.push_back(h);
  }

  void Separator::AddEvent(Event e) {
    events.push_back(e);
  }

  unsigned int Separator::ID() const {
    return id;
  }
}
  

#endif
