#ifndef MUON_TRIGGERMATCH
#define MUON_TRIGGERMATCH

#include "Signal.cpp"
#include "Separator.cpp"
#include "Hit.cpp"
#include "TimeCorrection.cpp"

int rollover_bindiff_cal(int a, int b, int rollover){
	int bindiff;
	bindiff = a-b;
	if (bindiff > rollover/2)	bindiff -= rollover;
	else if (bindiff < - (rollover/2)) bindiff += rollover;
	return bindiff;
}
void TriggerFind(Separator *s){
	int adc_bin = 0;
	Hit h;
	for (auto trig : s->TrigSignals()) {
    if (trig.Type() == Signal::RISING) {
      for (auto trig2 : s->TrigSignals()) {
        if ((trig2.Type() == Signal::FALLING) && (!trig2.Paired())) {
          adc_bin = rollover_bindiff_cal(trig2.Edge(),trig.Edge(),524288);  //254288 = 2^19
          if ((adc_bin > 0) && (adc_bin<2048)){  //2048 = 400ns/25*128
          	trig2.SetPaired();
          	h = Hit(trig.Edge(), adc_bin, trig.Edge(), trig.Edge(), trig.TDC(), trig.Channel());
        		s->AddTriggerHit(h);
          	break;
          }
        }
      }//for (auto trig2 : s->TrigSignals())
    }//if (trig.Type() == Signal::RISING)
  }//for (auto trig : s->TrigSignals())
}

void TriggerMatch(Separator *s, int matchwindow, int matchoffset, TimeCorrection tc){
	TriggerFind(s);
	
	int drift_bin;
	int adc_bin;
	Hit h;
	for (auto trig : s->TriggerHits()){
		event e;    //each trigger represents an event
		for (auto lsig : s->LEdgeSignals()){ 
			drift_bin = rollover_bindiff_cal(lsig.Edge()-trig.LEdge());
			if((BCID_diff > matchoffset) && (BCID_diff <= matchoffset + matchwindow)){  //match succeeds
				for (auto tsig : s->TEdgeSignals()){					
					if((!tsig.Paired()) && (tsig.SameTDCChan(lsig))){
						adc_bin = rollover_bindiff_cal(tsig.Edge(),lsig.Edge(),524288);  //254288 = 2^19
						if ((adc_bin > 0) && (adc_bin<2048)){  //2048 = 400ns/25*128   pair succeeds
	          	lsig.SetPaired();
	          	corr_time = drift_bin*25.0/128.0 - tc.SlewCorrection(adc_bin*25.0/128.0);
	          	h = Hit(lsig.Edge(), adc_bin, drift_bin, corr_time, lsig.TDC(), lsig.Channel());
	        		e.AddSignalHit(h);	        		
	          	break;  //break for (auto tsig : s->TEdgeSignals())
	          } //((adc_bin > 0) && (adc_bin<2048))
					}//if(!tsig.Paired())
				}//for (auto tsig : s->TEdgeSignals())
			} //if match succeeds
		} //for (auto lsig : s->LEdgeSignals())
    //all hits matched out
    if(e.WireHits().size() != 0){
    	s->AddEvent(e);
    }
	} //for (auto trig : s->TriggerHits())
}

#endif
