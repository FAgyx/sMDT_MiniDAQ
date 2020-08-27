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
    Signal(unsigned int word);

    int  Type();
    int  TDC();
    int  Channel();
    int  BCID();
    int  Edge(); 
    bool SameTDCChan(Signal other);
    bool Paired();
    void SetPaired();    

    static const short RISING        = 4;
    static const short FALLING       = 5;

  private:
    int   type, tdc, channel, bcid, edge;
    bool  isFirstSignal = false;
    bool  paired = false;
  };

  Signal::Signal() {
    Signal(0x00000000);
  }
  
  Signal::Signal(unsigned int word) {
    bitset<4>  _type;
    bitset<4>  _tdc;
    bitset<5>  _channel;
    bitset<12> _bcid;
    bitset<19> _edge;

    _type    = word >> 28;
    _tdc     = word >> 24;
    _channel = word >> 19;
    _bcid    = word >> 7;
    _edge    = word;

    type    = static_cast<int>((_type.to_ulong()));
    tdc     = static_cast<int>((_tdc.to_ulong()));
    channel = static_cast<int>((_channel.to_ulong()));
    
    bcid    = static_cast<int>((_bcid.to_ulong()));
    edge    = static_cast<int>((_edge.to_ulong()));

    // AMT_width = static_cast<unsigned int>((_AMT_width.to_ulong()));
    // AMT_edge = static_cast<unsigned int>((_AMT_edge.to_ulong()));

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
  
  int Signal::BCID() {
    return bcid;
  }

  bool Signal::Paired() {
    return paired;
  }
  
  bool Signal::SameTDCChan(Signal other) {
    if (this->Channel() == other.Channel() && this->TDC() == other.TDC()) 
      return true;
    else 
      return false;
  }

  void Signal::SetPaired() {
    paired = true;
  }
}

#endif
