#ifndef MUON_SIGNAL
#define MUON_SIGNAL

#include <bitset>
#include <iostream>

#include "MuonReco/EventID.h"

namespace MuonReco {

  /*! \class Signal Signal.h "MuonReco/Signal.h"
   * \brief Wrapper to TDC word on non-trigger channel
   * 
   * Signal is a nice wrapper to a HPTDC word
   * corresponding to a signal leading or falling 
   * edge time
   *
   * The packet format is: 
   *     32 bits total 
   *      4 bit type         (identified rising/falling edge)   
   *      4 bit tdc num      (which board did this come from)
   *      5 bit channel num  (which channel on the board) 
   *     12 bit coarse time  
   *      7 bit fine time   
   *
   * the true event time in nanoseconds is 
   *
   * 25ns*(coarse + fine/128)    
   *
   * For further details, see HPTDC Documentation 
   * https://cds.cern.ch/record/1067476/files/cer-002723234.pdf
   * pages 21-22
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
  class Signal {
  public:
    Signal();
    Signal(unsigned int word, EventID eID);
    Signal(unsigned int _type, unsigned int _tdc, unsigned int _channel, unsigned int _isCSM2,
	   unsigned long _id, double _time_in_ns, bool _isFirstSignal);
    Signal(const Signal &lhs);

    unsigned int  Type() const;
    unsigned int  TDC() const;
    unsigned int  Channel() const;
    unsigned long EvtID() const;
    double        Time() const; // in nano seconds
    bool          SameTDCChan(Signal other) const;
    bool          IsFirstSignal() const;
    void          SetIsFirstSignal(bool b);
    void          SetIsCSM2(bool b);

    static const short GROUP_HEADER  = 0;
    static const short GROUP_TRAILER = 1;
    static const short TDC_HEADER    = 2;
    static const short TDC_TRAILER   = 3;
    static const short RISING        = 4;
    static const short FALLING       = 5;

  private:
    unsigned int  type, tdc, channel, isCSM2;
    unsigned long id;
    double      time_in_ns;
    bool        isFirstSignal = false;
  };
}
#endif
