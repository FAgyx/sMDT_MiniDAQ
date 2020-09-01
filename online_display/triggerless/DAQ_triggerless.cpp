
// Muon Reconstruction includes
#include "triggerless/Signal.cpp"
#include "triggerless/Hit.cpp"
#include "triggerless/Event.cpp"
#include "triggerless/Separator.cpp"
#include "triggerless/TriggerMatch.cpp"
#include "triggerless/DAQmonitor.cpp"




using namespace std;
using namespace Muon;



int DAQ_triggerless(int portno){
	Separator s;
	Event e;
	Hit h;
	Signal sig;
	DAQ_monitor *p_DAQ_monitor = new DAQ_monitor(portno);
	p_DAQ_monitor->DataDecode_triggerless();
	return 1;
}