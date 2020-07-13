#include "MuonReco/RTAggregator.h"

namespace MuonReco {

  RTAggregator::RTAggregator() {

  }
  
  RTAggregator::~RTAggregator() {

  }

  double RTAggregator::Eval(Hit h) {
    return rts.at(RTvectorIndex[h.TDC()*Geometry::MAX_TDC_CHANNEL + h.Channel()])->Eval(h);
  }

  double RTAggregator::NormalizedTime(double time, int tdc_id, int ch_id) {
    return rts.at(RTvectorIndex[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id])->NormalizedTime(time, tdc_id, ch_id);
  }

  void RTAggregator::AddRT(RTParam* rtp) {
    rts.push_back(rtp);
  }

  void RTAggregator::Initialize(TString t0path, TString decodedDataPath) {
    for (RTParam* rt : rts) rt->Initialize(t0path, decodedDataPath);
  }
  
  void RTAggregator::InitMultiLayer(Geometry & geo) {
    if (rts.size() != 2) {
      std::cout << "Cannot configure RTAggregator object in "         << std::endl;
      std::cout << "multilayer mode without precisely 2 RT functions" << std::endl;
      return;
    }
    int hitL, hitC, ML;
    for (int tdc = 0; tdc < Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan < Geometry::MAX_TDC_CHANNEL; chan++) {
	geo.GetHitLayerColumn(tdc, chan, &hitL, &hitC);
	ML = geo.MultiLayer(hitL);
	RTvectorIndex[tdc*Geometry::MAX_TDC_CHANNEL + chan] = ML;
	if (ML) {
	  rts.at(0)->SetIgnoreTube(tdc, chan);
	}
	else {
	  rts.at(1)->SetIgnoreTube(tdc, chan);
	}
      }
    }
  }
  
  void RTAggregator::InitTube(Geometry & geo) {
    int numTubes = 0;
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (tdc != geo.TRIGGER_MEZZ && geo.IsActiveTDCChannel(tdc, chan)) {
	  numTubes++;
	}
      }
    }
    if (rts.size() != numTubes) {
      std::cout << "Cannot configure RTAggregator object in" << std::endl;
      std::cout << "tube mode with numtubes != this->rts.size()" << std::endl;
      return;
    }

    for (RTParam* rt : rts) rt->SetIgnoreAll();

    int index = 0;
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (tdc != geo.TRIGGER_MEZZ && geo.IsActiveTDCChannel(tdc, chan)) {
	  RTvectorIndex[tdc*Geometry::MAX_TDC_CHANNEL + chan] = index;
	  rts.at(index)->SetActiveTube(tdc, chan);
	  index++;
	}
      }
    }
    
  }

}
