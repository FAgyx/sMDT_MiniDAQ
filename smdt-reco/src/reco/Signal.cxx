#include "MuonReco/Signal.h"
#ifndef WIDTH_RES
#define WIDTH_RES 1
#endif
#ifndef NEWTDC_NUMBER
#define NEWTDC_NUMBER 9
#endif

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
    std::bitset<8>  _AMT_width;
    std::bitset<11>  _AMT_edge;

    unsigned int coarse, fine;
    unsigned int AMT_width, AMT_edge;

    _type    = word >> 28;
    _tdc     = word >> 24;
    _channel = word >> 19;
    _coarse  = word >> 7;
    _fine    = word;
    _AMT_width = word >>11;
    _AMT_edge = word;

    type    = static_cast<unsigned int>((_type.to_ulong()));
    tdc     = static_cast<unsigned int>((_tdc.to_ulong()));
    channel = static_cast<unsigned int>((_channel.to_ulong()));
    
    coarse = static_cast<unsigned int>((_coarse.to_ulong()));
    fine   = static_cast<unsigned int>((_fine.to_ulong()));

    AMT_width = static_cast<unsigned int>((_AMT_width.to_ulong()));
    AMT_edge = static_cast<unsigned int>((_AMT_edge.to_ulong()));

    if(tdc != NEWTDC_NUMBER){
      time_in_ns = (coarse + fine / 128.0 ) * 25.0;
      adcTime == 0;
      edgeword = (coarse * 128 + fine) / 4;    //LSB = 25ns/128
    }
    else{
      time_in_ns = AMT_edge / 32.0 * 25.0; 
      edgeword = AMT_edge;
      if(WIDTH_RES == 0) adcTime = AMT_width / 32.0 * 25.0;
      else if (WIDTH_RES == 1) adcTime = AMT_width / 32.0 * 25.0 * 2;
      else if (WIDTH_RES == 2) adcTime = AMT_width / 32.0 * 25.0 * 4;
      else if (WIDTH_RES == 3) adcTime = AMT_width / 32.0 * 25.0 * 8;
      else adcTime == 0;
    }

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

  double Signal::ADCTime() {
    return adcTime;
  }

  unsigned int Signal::EdgeWord() {
    return edgeword;
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
