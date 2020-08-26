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

    unsigned int  Type();
    unsigned int  TDC();
    unsigned int  Channel();
    unsigned long BCID();
    unsigned int  EdgeWord(); 
    Bool_t        SameTDCChan(Signal other);

    

    static const short RISING        = 4;
    static const short FALLING       = 5;

  private:
    unsigned int  type, tdc, channel, bcid, edge;
    unsigned long id;
    Bool_t        isFirstSignal = false;
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
    // bitset<8>  _AMT_width;
    // bitset<11>  _AMT_edge;

    unsigned int coarse, fine;
    unsigned int AMT_width, AMT_edge;

    _type    = word >> 28;
    _tdc     = word >> 24;
    _channel = word >> 19;
    _bcid    = word >> 7;
    _edge    = word;
    // _AMT_width = word >>11;
    // _AMT_edge = word;

    type    = static_cast<unsigned int>((_type.to_ulong()));
    tdc     = static_cast<unsigned int>((_tdc.to_ulong()));
    channel = static_cast<unsigned int>((_channel.to_ulong()));
    
    bcid = static_cast<unsigned int>((_bcid.to_ulong()));
    edge   = static_cast<unsigned int>((_edge.to_ulong()));

    // AMT_width = static_cast<unsigned int>((_AMT_width.to_ulong()));
    // AMT_edge = static_cast<unsigned int>((_AMT_edge.to_ulong()));

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


  unsigned int Signal::EdgeWord() {
    return edge;
  }
  
  unsigned long Signal::BCID() {
    return bcid;
  }
  
  Bool_t Signal::SameTDCChan(Signal other) {
    if (this->Channel() == other.Channel() && this->TDC() == other.TDC()) 
      return true;
    else 
      return false;
  }
}

#endif
