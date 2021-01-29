
#include "src/Event.cpp"
#include "src/Hit.cpp"
#include "src/Signal.cpp"
#include "src/TimeCorrection.cpp"
#include "src/HitFinder.cpp"

#ifndef MUON_HITFINDER_BCID
#define MUON_HITFINDER_BCID

#define BCID_OFFSET 61




void DoHitFinding_BCID(Event *e, TimeCorrection tc, double adc_time_lowlimit) {
  Hit h;
  double adc_time = 0;
  double drift_time, corr_time;
  double trigger_ledge;
  // do trigger hit finding
  int bcid[Geometry::MAX_TDC] = {0};
  int header_found[Geometry::MAX_TDC] = {0};
  for (auto sig : e->WireSignals()) {
    if(sig.Type() == Signal::AMT_HEADER || sig.Type() == Signal::TDC_HEADER) {
      header_found[sig.TDC()] = 1;
      bcid[sig.TDC()] = sig.HeaderBCID(); 
    }
  }
  for (auto sig : e->WireSignals()) {
    if(sig.TDC() != NEWTDC_NUMBER) {  //HPTDC edge mode 
      if (sig.Type() == Signal::RISING) {
        if (header_found[sig.TDC()]) {
          drift_time = rollover_bindiff_cal(sig.Edge(),bcid[sig.TDC()]*128,524288)*25.0/128.0;  //254288 = 2^19
        }
        else drift_time = -5000;
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
    }//HPTDC edge mode 
    else {  //AMT edge mode
      adc_time = sig.ADCTime();
      if (header_found[sig.TDC()]) {
        drift_time = rollover_bindiff_cal(sig.EdgeAMT(),(bcid[sig.TDC()]-BCID_OFFSET+4096)%4096*32%2048,2048)*25.0/32.0;  //254288 = 2^19
      }
      else drift_time = -5000;
      if (sig.IsFirstSignal() && adc_time > adc_time_lowlimit) {
          // if (sig.IsFirstSignal() && adc_time > 0) {
        corr_time = drift_time - tc.SlewCorrection(adc_time);           
        h = Hit(sig.EdgeAMT()*25.0/32.0, adc_time, drift_time, corr_time, sig.TDC(), sig.Channel());
        e->AddSignalHit(h);
      }  
    }//AMT edge mode
  } //for (auto sig : e->WireSignals()) {


  for (auto sig : e->TrigSignals()) {
    if (sig.Type() == Signal::RISING) {
      if (header_found[sig.TDC()]) {
        drift_time = rollover_bindiff_cal(sig.Edge(),bcid[sig.TDC()]*128,524288)*25.0/128.0;  //254288 = 2^19
      }
      else drift_time = -5000;
      adc_time = 0.0;
      for (auto sig2 : e->TrigSignals()) {
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
    
  }
}

#endif
