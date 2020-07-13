#ifndef MUON_SIGNAL
#define MUON_SIGNAL

#include <bitset>

#include "MuonReco/EventID.h"

namespace MuonReco {

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
    double        Time(); // in nano seconds
    bool          SameTDCChan(Signal other);
    bool          IsFirstSignal();
    void          SetIsFirstSignal(bool b);

    static const short GROUP_HEADER  = 0;
    static const short GROUP_TRAILER = 1;
    static const short TDC_HEADER    = 2;
    static const short TDC_TRAILER   = 3;
    static const short RISING        = 4;
    static const short FALLING       = 5;

  private:
    unsigned int  type, tdc, channel;
    unsigned long id;
    double      time_in_ns;
    bool        isFirstSignal = false;
  };
}
#endif
