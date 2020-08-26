#include "Signal.cpp"
#ifndef MUON_EVENT
#define MUON_EVENT

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
    Separator(vector<Signal> triggers, vector<Signal> signals, unsigned long iD);
    Separator(const Separator &e);

    vector<Signal>  TrigSignals() const;
    vector<Signal>  WireSignals() const;
    unsigned long   ID         () const;

  private:
    unsigned long   id;
    vector<Signal>  trigs;
    vector<Signal>  sigs;
  };

  Separator::Separator() {
    id       = 0;
    trigs    = vector<Signal>();
    sigs     = vector<Signal>();
  }

  Separator::Separator(vector<Signal> triggers, vector<Signal> signals, unsigned long iD) {
    id       = iD;
    trigs    = triggers;
    sigs     = signals;
  }

  Separator::Separator(const Separator &e) {
    trigs    = e.TrigSignals();
    sigs     = e.WireSignals();
    id       = e.ID();
  }
  
  vector<Signal> Separator::WireSignals() const {
    return sigs;
  }
  
  vector<Signal> Separator::TrigSignals() const {
    return trigs;
  }  

  unsigned long Separator::ID() const {
    return id;
  }
}
  

#endif
