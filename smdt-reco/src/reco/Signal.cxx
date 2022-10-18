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

    _type    = (word & 0x7fffffff) >> 28;
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
    isCSM2 = (word & 0x80000000) ? 1 : 0;
  }

  Signal::Signal(unsigned int _type, unsigned int _tdc,unsigned int _channel, unsigned int _isCSM2,
		 unsigned long _id, double _time_in_ns, bool _isFirstSignal) {
    type          = _type;
    tdc           = _tdc;
    channel       = _channel;
    isCSM2        = _isCSM2;
    id            = _id;
    time_in_ns    = _time_in_ns;
    isFirstSignal = _isFirstSignal;
  }

  Signal::Signal(const Signal &lhs) {
    isCSM2        = lhs.TDC() >= 18;
    type          = lhs.Type();
    tdc           = lhs.TDC() - (isCSM2)*18;
    channel       = lhs.Channel();
    isCSM2        = lhs.TDC() >= 18;
    id            = lhs.EvtID();
    time_in_ns    = lhs.Time();
    isFirstSignal = lhs.IsFirstSignal();
  }

  void Signal::SetIsCSM2(bool b) {
    isCSM2 = b;
  }

  unsigned int Signal::Type() const {
    return type;
  }

  unsigned int Signal::TDC() const {
    if (isCSM2) return tdc+(unsigned int)18;
    else return tdc;
  }

  unsigned int Signal::Channel() const {
    return channel;
  }

  double Signal::Time() const {
    return time_in_ns;
  }
  
  unsigned long Signal::EvtID() const {
    return id;
  }
  
  bool Signal::SameTDCChan(Signal other) const {
    if (this->Channel() == other.Channel() && this->TDC() == other.TDC()) 
      return 1;
    else 
      return 0;
  }

  bool Signal::IsFirstSignal() const {
    return isFirstSignal;
  }
  
  void Signal::SetIsFirstSignal(bool b) {
    isFirstSignal = b;
  }
}
