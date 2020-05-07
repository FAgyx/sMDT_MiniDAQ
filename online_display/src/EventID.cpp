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

#ifndef MUON_EVENTID
#define MUON_EVENTID

namespace Muon {

  /*
   * "EventID" is a nice wrapper to a HPTDC word
   * corresponding to a header with the event number
   * 
   * The packet format is:
   * 32 bits total
   *  4 bit "type"     (is this a group header, tdc header, etc)
   *  4 bit "tdc"      (which board did this come from)
   * 12 bit "event id" 
   * 12 bit "bunch id"
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
  
  EventID::EventID() {
    EventID(0x00000000);
  }
    
  EventID::EventID(unsigned int word) {
    bitset<4>  _type;
    bitset<4>  _tdc;
    bitset<12> _event_id;
    bitset<12> _bunch_id;

    _type     = word >> 28;
    _tdc      = word >> 24;
    _event_id = word >> 12;
    _bunch_id = word;

    unsigned long event_id, bunch_id;

    type     = static_cast<unsigned int >((_type.to_ulong()));
    tdc      = static_cast<unsigned int >((_tdc.to_ulong()));
    event_id = static_cast<unsigned long>((_event_id.to_ulong()));
    bunch_id = static_cast<unsigned long>((_bunch_id.to_ulong()));

    this->id = 4096UL*bunch_id + event_id;
  }

  unsigned int EventID::Type() {
    return this->type;
  }

  unsigned int EventID::TDC() {
    return this->tdc;
  }

  unsigned long EventID::ID() {
    return this->id;
  }

}

#endif
