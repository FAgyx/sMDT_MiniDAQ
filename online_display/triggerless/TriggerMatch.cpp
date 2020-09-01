
#include "triggerless/Signal.cpp"
#include "triggerless/Separator.cpp"
#include "triggerless/Hit.cpp"
#include "triggerless/Event.cpp"
#include "triggerless/TimeCorrection.cpp"

#ifndef MUON_TRIGGERMATCH
#define MUON_TRIGGERMATCH

using namespace std;
using namespace Muon;



int rollover_bindiff_cal(int a, int b, int rollover){
    int bindiff;
    bindiff = a-b;
    if (bindiff > rollover/2)   bindiff -= rollover;
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
          // printf("trigger adc_bin = %d\n",adc_bin);
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
  double corr_time;
  Hit h;
  for (auto trig : s->TriggerHits()){
    Event e;    //each trigger represents an event
    for (auto lsig : s->LEdgeSignals()){ 
      drift_bin = rollover_bindiff_cal(lsig.Edge(),trig.LEdge(),524288);
      if((drift_bin > matchoffset) && (drift_bin <= matchoffset + matchwindow)){  //match succeeds
        for (auto tsig : s->TEdgeSignals()){                    
          if((!tsig.Paired()) && (tsig.SameTDCChan(lsig))){
            adc_bin = rollover_bindiff_cal(tsig.Edge(),lsig.Edge(),524288);  //524288 = 2^19
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
    if(e.WireHits().size() != 0){
      s->AddEvent(e);
    }
  } //for (auto trig : s->TriggerHits())
  //all hits matched out   
}

#endif
