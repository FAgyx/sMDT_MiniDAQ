#include "src/RTParam.cpp"
#include "src/Geometry.cpp"
#include "src/Callable.cpp"

#ifndef MUON_RT_AGGREGATOR
#define MUON_RT_AGGREGATOR

namespace Muon {
  /******************************************
   * Class to implement the Eval interface  *
   * of the RT Parameterization object      *
   * and encapsulate an underlying          *
   * representation in which multiple       *
   * independent parameterizations are used *
   * for different regions of the chamber   *
   *                                        *
   * Author:        Kevin Nelson            *
   * Date:          July 16th, 2019         *
   * Last Modified: July 19th, 2019         *
   *                                        *
   ******************************************
   */
  class RTAggregator : public Callable {
  public:
    RTAggregator();
    ~RTAggregator();

    double Eval          (Hit h) override;
    double NormalizedTime(double time, int tdc_id, int ch_id) override;

    void   AddRT         (RTParam* rtp);
    void   InitMultiLayer(Geometry & geo);
    void   InitTube      (Geometry & geo);
    void   Initialize    (TString t0path, TString decodedDataPath);

    vector<RTParam*> rts;

  private:
    int RTvectorIndex[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];
  };

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
      cout << "Cannot configure RTAggregator object in "         << endl;
      cout << "multilayer mode without precisely 2 RT functions" << endl;
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
      cout << "Cannot configure RTAggregator object in" << endl;
      cout << "tube mode with numtubes != this->rts.size()" << endl;
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

#endif
