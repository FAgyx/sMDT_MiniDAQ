
#include "src/Event.cpp"
#include "src/Hit.cpp"
#include "src/Signal.cpp"
#include "src/TimeCorrection.cpp"

void DoHitFinding(Event *e, TimeCorrection tc, double adc_time_lowlimit) {
  Hit h;
  double adc_time = 0.0;
  // do trigger hit finding
  for (auto trig : e->TrigSignals()) {
    if (trig.Type() == Signal::RISING) {

      for (auto trig2 : e->TrigSignals()) {
        if (trig2.Type() == Signal::FALLING && trig.SameTDCChan(trig2)) {
          adc_time = trig2.Time() - trig.Time();
          break;
        }
      }
      // have found adc time for this trigger
      // construct a hit and push back onto some hit vector
      if (trig.IsFirstSignal() && adc_time != 0.0) {
      // if (trig.IsFirstSignal() && adc_time != 0.0 && trig.Time() > 290 && trig.Time() < 350) {
        h = Hit(trig.Time(), adc_time, trig.Time(), trig.Time(), trig.TDC(), trig.Channel());
        e->AddTriggerHit(h);
      }
    }
  } // end for: t_iter

  // do signal hit finding, can only do if triggers exist
  if (e->TrigSignals().size() != 0) {
    Signal selectTrigger = e->TrigSignals().at(0);
    double drift_time, corr_time;

    for (auto sig : e->WireSignals()) {
      if (sig.Type() == Signal::RISING) {
        drift_time = sig.Time() - selectTrigger.Time();

        adc_time = 0.0;
      	for (auto sig2 : e->WireSignals()) {
      	  if (sig.SameTDCChan(sig2) && (sig2.Type() == Signal::FALLING)) {
      	    adc_time = sig2.Time() - sig.Time();
      	    break;
      	  }
      	} // end for: s_iter2
      	if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {
        // if (sig.IsFirstSignal() && adc_time > 0) {
      	  corr_time = drift_time - tc.SlewCorrection(adc_time);
      	  
      	  h = Hit(sig.Time(), adc_time, drift_time, corr_time, sig.TDC(), sig.Channel());
      	  e->AddSignalHit(h);
      	}
      }
    } // end for: s_iter
  } // end if: nonzero number of triggers
  else if(adc_time_lowlimit == 0){ //if there is no trigger, calculate adc_time only
    for (auto sig : e->WireSignals()) {
      if (sig.Type() == Signal::RISING) {
        drift_time = -600.0;
        adc_time = 0.0;
        for (auto sig2 : e->WireSignals()) {
          if (sig.SameTDCChan(sig2) && (sig2.Type() == Signal::FALLING)) {
            adc_time = sig2.Time() - sig.Time();
            break;
          }
        } // end for: s_iter2
        if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {        
          h = Hit(sig.Time(), adc_time, drift_time, drift_time, sig.TDC(), sig.Channel());
          e->AddSignalHit(h);
        }
      }
    } // end for: s_iter

  }
}
