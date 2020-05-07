/*********************************
 *                               *
 * Define a trigger class        *
 * for reconstruction in         *
 * sMDT chambers                 *
 *                               *
 * Author:        Kevin Nelson   *
 * Date:          May 13, 2018   *
 * Last Modified: May 13, 2018   *
 *                               *
 *********************************
 */

#include "src/EventID.cpp"

#ifndef MUON_SIGNAL
#define MUON_SIGNAL

namespace Muon {

  /*
   * Signal is a nice wrapper to a HPTDC word
   * corresponding to a signal leading or falling
   * edge time
   * 
   * The packet format is:
   * 32 bits total
   *  4 bit type         (identified rising/falling edge)
   *  4 bit tdc num      (which board did this come from)
   *  5 bit channel num  (which channel on the board)
   * 12 bit coarse time
   *  7 bit fine time
   *
   * the true event time in nanoseconds is
   * 
   * 25ns*(coarse + fine/128)
   * 
   * For further details, see HPTDC Documentation
   * pages 21-22
   */
  class Signal {
  public:
    Signal();
    Signal(unsigned int word, EventID eID);

    unsigned int  Type();
    unsigned int  TDC();
    unsigned int  Channel();
    unsigned long EvtID();
    Double_t      Time(); // in nano seconds
    Bool_t        SameTDCChan(Signal other);
    Bool_t        IsFirstSignal();
    void          SetIsFirstSignal(Bool_t b);
    
    static const short GROUP_HEADER  = 0;
    static const short GROUP_TRAILER = 1;
    static const short TDC_HEADER    = 2;
    static const short TDC_TRAILER   = 3;
    static const short RISING        = 4;
    static const short FALLING       = 5;

  private:
    unsigned int  type, tdc, channel;
    unsigned long id;
    Double_t      time_in_ns;
    Bool_t        isFirstSignal = false;
  };

  Signal::Signal() {
    Signal(0x00000000, EventID());
  }
  
  Signal::Signal(unsigned int word, EventID eID) {
    bitset<4>  _type;
    bitset<4>  _tdc;
    bitset<5>  _channel;
    bitset<12> _coarse;
    bitset<7>  _fine;

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
    isFirstSignal = kFALSE;
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

  Double_t Signal::Time() {
    return time_in_ns;
  }
  
  unsigned long Signal::EvtID() {
    return id;
  }
  
  Bool_t Signal::SameTDCChan(Signal other) {
    if (this->Channel() == other.Channel() && this->TDC() == other.TDC()) 
      return kTRUE;
    else 
      return kFALSE;
  }

  Bool_t Signal::IsFirstSignal() {
    return isFirstSignal;
  }
  
  void Signal::SetIsFirstSignal(Bool_t b) {
    isFirstSignal = b;
  }
}

#endif
