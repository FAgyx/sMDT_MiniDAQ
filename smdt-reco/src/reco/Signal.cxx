#include "MuonReco/Signal.h"

namespace MuonReco {

  Signal::Signal() {
    Signal(0x00000000, EventID());
  }
  
  Signal::Signal(unsigned int word, EventID eID) {
    std::bitset<4>  _type;
    std::bitset<4>  _tdc;
    std::bitset<5>  _channel;
    std::bitset<12> _coarse;
    std::bitset<7>  _fine;

    unsigned int coarse, fine;

    _type    = word >> 28;
    _tdc     = word >> 24;
    _channel = word >> 19;
    _coarse  = word >> 7;
    _fine    = word;

    type    = static_cast<unsigned int>((_type.to_ulong()));
    tdc     = static_cast<unsigned int>((_tdc.to_ulong()));
    channel = static_cast<unsigned int>((_channel.to_ulong()));
    
    coarse = static_cast<unsigned int>((_coarse.to_ulong()));
    fine   = static_cast<unsigned int>((_fine.to_ulong()));

    time_in_ns = (coarse + fine / 128.0 ) * 25.0;

    id = eID.ID();
    isFirstSignal = 0;
  }

  unsigned int Signal::Type() {
    return type;
  }

  unsigned int Signal::TDC() {
    return tdc;
  }

  unsigned int Signal::Channel() {
    return channel;
  }

  double Signal::Time() {
    return time_in_ns;
  }
  
  unsigned long Signal::EvtID() {
    return id;
  }
  
  bool Signal::SameTDCChan(Signal other) {
    if (this->Channel() == other.Channel() && this->TDC() == other.TDC()) 
      return 1;
    else 
      return 0;
  }

  bool Signal::IsFirstSignal() {
    return isFirstSignal;
  }
  
  void Signal::SetIsFirstSignal(bool b) {
    isFirstSignal = b;
  }
}
