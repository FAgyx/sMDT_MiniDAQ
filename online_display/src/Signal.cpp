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

    int           Type();
    int           TDC();
    int           Channel();
    unsigned long EvtID();
    int           Edge();  
    int           EdgeAMT();
    int           WidthAMT();
    double        ADCTime();
    Bool_t        SameTDCChan(Signal other);
    Bool_t        IsFirstSignal();
    void          SetIsFirstSignal(Bool_t b);
    
    static const short GROUP_HEADER  = 0;
    static const short GROUP_TRAILER = 1;
    static const short TDC_HEADER    = 2;
    static const short TDC_TRAILER   = 3;
    static const short RISING        = 4;
    static const short FALLING       = 5;
    static const short AMT_HEADER    = 10;

  private:
    int           type, tdc, channel, bcid, edge, widthAMT, edgeAMT;
    double        adcTime;
    unsigned long id;
    Bool_t        isFirstSignal = false;
  };

  Signal::Signal() {
    Signal(0x00000000, EventID());
  }
  
  Signal::Signal(unsigned int word, EventID eID) {
    bitset<4>  _type;
    bitset<4>  _tdc;
    bitset<5>  _channel;
    bitset<12> _bcid;
    bitset<19> _edge;
    bitset<8>  _width_AMT;
    bitset<11> _edge_AMT;

    _type    = word >> 28;
    _tdc     = word >> 24;
    _channel = word >> 19;
    _bcid    = word >> 7;
    _edge    = word;
    _width_AMT = word >>11;
    _edge_AMT  = word;

    type    = static_cast<int>((_type.to_ulong()));
    tdc     = static_cast<int>((_tdc.to_ulong()));
    channel = static_cast<int>((_channel.to_ulong()));    
    bcid    = static_cast<int>((_bcid.to_ulong()));
    edge    = static_cast<int>((_edge.to_ulong()));

    widthAMT = static_cast<int>((_width_AMT.to_ulong()));
    edgeAMT  = static_cast<int>((_edge_AMT.to_ulong()));

    // if(tdc != NEWTDC_NUMBER){
    //   time_in_ns = (coarse + fine / 128.0 ) * 25.0;
    //   adcTime = 0;
    //   edgeword = (coarse * 128 + fine) / 4;    //LSB = 25ns/128
    // }
    // else{
    if(tdc == NEWTDC_NUMBER){
      if(WIDTH_RES == 0) adcTime = widthAMT / 32.0 * 25.0;
      else if (WIDTH_RES == 1) adcTime = widthAMT / 32.0 * 25.0 * 2;
      // else if (WIDTH_RES == 1) adcTime = widthAMT %4+1;
      else if (WIDTH_RES == 2) adcTime = widthAMT / 32.0 * 25.0 * 4;
      else if (WIDTH_RES == 3) adcTime = widthAMT / 32.0 * 25.0 * 8;
      else adcTime = 0;
    }

    id = eID.ID();
    isFirstSignal = kFALSE;
  }

  int Signal::Type() {
    return type;
  }

  int Signal::TDC() {
    return tdc;
  }

  int Signal::Channel() {
    return channel;
  }

  int Signal::Edge() {
    return edge;
  }
  
  unsigned long Signal::EvtID() {
    return id;
  }

  int Signal::EdgeAMT() {
    return edgeAMT;
  }

  int Signal::WidthAMT() {
    return widthAMT;
  }

  double Signal::ADCTime(){
    return adcTime;
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
