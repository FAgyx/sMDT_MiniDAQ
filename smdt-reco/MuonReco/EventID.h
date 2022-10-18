#ifndef MUON_EVENTID
#define MUON_EVENTID

#include <bitset>
#include <iostream>

namespace MuonReco {
  /*! \class EventID EventID.h "MuonReco/EventID.h"
   * \brief A  nice wrapper to a HPTDC word corresponding to a header with the event number 
   * 
   * The packet format is:
   *     32 bits total
   *      4 bit "type"     (is this a group header, tdc header, etc) 
   *      4 bit "tdc"      (which board did this come from) 
   *     12 bit "event id"    
   *     12 bit "bunch id"   
   *
   * the true event number is event_id + 4096*bunch_id 
   * 
   * For further details, see HPTDC Documentation 
   * pages 21-22 
   */
  class EventID {
  public:
    EventID();
    EventID(unsigned int word);
    unsigned int  Type();
    unsigned int  TDC();
    unsigned long ID();
  private:
    unsigned int  type, tdc;
    unsigned long id;
  };

}

#endif
