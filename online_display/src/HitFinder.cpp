
#include "src/Event.cpp"
#include "src/Hit.cpp"
#include "src/Signal.cpp"
#include "src/TimeCorrection.cpp"

#ifndef MUON_HITFINDER
#define MUON_HITFINDER

int rollover_bindiff_cal(int a, int b, int rollover){
    int bindiff;
    bindiff = a-b;
    if (bindiff > rollover/2)   bindiff -= rollover;
    else if (bindiff < - (rollover/2)) bindiff += rollover;
    return bindiff;
}

void DoHitFinding(Event *e, TimeCorrection tc, double adc_time_lowlimit) {
  Hit h;
  double adc_time = 0;
  double trigger_ledge;
  // do trigger hit finding
  for (auto trig : e->TrigSignals()) {
    if (trig.Type() == Signal::FALLING) {

      for (auto trig2 : e->TrigSignals()) {
        if (trig2.Type() == Signal::RISING && trig.SameTDCChan(trig2)) {
          adc_time = rollover_bindiff_cal(trig2.Edge(),trig.Edge(),524288)*25.0/128.0;  //254288 = 2^19
          break;
        }
      }
      // have found adc time for this trigger
      // construct a hit and push back onto some hit vector
      if (trig.IsFirstSignal() && adc_time > 0) {
      // if (trig.IsFirstSignal() && adc_time != 0.0 && trig.Time() > 290 && trig.Time() < 350) {
        trigger_ledge = trig.Edge()*25.0/128.0;
        h = Hit(trigger_ledge, adc_time, trigger_ledge, trigger_ledge, trig.TDC(), trig.Channel());
        // h = Hit(trig.Time(), adc_time, trig.Time(), trig.Time(), trig.TDC(), trig.Channel());
        e->AddTriggerHit(h);
      }
    }
  } // end for: t_iter

  // do signal hit finding, can only do if triggers exist
  if (e->TrigSignals().size() != 0) {
    Signal selectTrigger = e->TrigSignals().at(0);
    double drift_time, corr_time;


    for (auto sig : e->WireSignals()) {
      if(sig.TDC() != NEWTDC_NUMBER) {  //HPTDC edge mode 
        if (sig.Type() == Signal::RISING) {
          drift_time = rollover_bindiff_cal(sig.Edge(),selectTrigger.Edge(),524288)*25.0/128.0;  //254288 = 2^19
          adc_time = 0.0;
        	for (auto sig2 : e->WireSignals()) {
        	  if (sig.SameTDCChan(sig2) && (sig2.Type() == Signal::FALLING)) {
              adc_time = rollover_bindiff_cal(sig2.Edge(),sig.Edge(),524288)*25.0/128.0;  //254288 = 2^19
        	    break;
        	  }
        	} // end for: s_iter2
        	if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {
          // if (sig.IsFirstSignal() && adc_time > 0) {
        	  corr_time = drift_time - tc.SlewCorrection(adc_time);
        	  
        	  h = Hit(sig.Edge()*25.0/128.0, adc_time, drift_time, corr_time, sig.TDC(), sig.Channel());
        	  e->AddSignalHit(h);
        	}
        }
      } // //end if:sig.TDC() 
      else{ //AMT pair mode
        adc_time = sig.ADCTime();
        drift_time = rollover_bindiff_cal(sig.EdgeAMT(),selectTrigger.Edge()/4%2048,2048)*25.0/32.0;  //2048 = 2^11
        if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {
          // if (sig.IsFirstSignal() && adc_time > 0) {
          corr_time = drift_time - tc.SlewCorrection(adc_time);           
          h = Hit(sig.EdgeAMT()*25.0/32.0, adc_time, drift_time, corr_time, sig.TDC(), sig.Channel());
          e->AddSignalHit(h);
        }
      }
    } // end for: s_iter
  } // end if: nonzero number of triggers
  // else if(adc_time_lowlimit == 0){ //if there is no trigger, calculate adc_time only
  else{
    for (auto sig : e->WireSignals()) {
      double drift_time;
      if(sig.TDC() != NEWTDC_NUMBER) {  //HPTDC edge mode 
        if (sig.Type() == Signal::RISING) {
          drift_time = -600.0;
          adc_time = 0.0;
          for (auto sig2 : e->WireSignals()) {
            if (sig.SameTDCChan(sig2) && (sig2.Type() == Signal::FALLING)) {
              adc_time = rollover_bindiff_cal(sig2.Edge(),sig.Edge(),524288)*25.0/128.0;  //254288 = 2^19
              // adc_time = sig2.Time() - sig.Time();
              break;
            }
          } // end for: s_iter2
          if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {
        // if (sig.IsFirstSignal()) {           
            h = Hit(sig.Edge()*25.0/128.0, adc_time, drift_time, drift_time, sig.TDC(), sig.Channel());
            e->AddSignalHit(h);
          }
        }
      } //end HPTDC edge mode
      else{  //AMT pair mode
        adc_time = sig.ADCTime();
        drift_time = -600.0;
        if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {
        // if (sig.IsFirstSignal()) {           
            h = Hit(sig.EdgeAMT()*25.0/32.0, adc_time, drift_time, drift_time, sig.TDC(), sig.Channel());
            e->AddSignalHit(h);
        }

      }//end AMT pair mode

      
    } // end for: s_iter

  }
}

#endif
