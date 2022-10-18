#include "MuonReco/RTAggregator.h"

namespace MuonReco {

  RTAggregator::RTAggregator() {
    RTvectorIndex = TubeMap<int>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
    rts = std::vector<RTParam*>();
  }

  RTAggregator::~RTAggregator() {

  }

  double RTAggregator::Eval(Hit h, double deltaT0/*=0*/, double slewScaleFactor/*=1.0*/, double sigPropSF/*=1.0*/) {
    return rts.at(RTvectorIndex.get(h.Layer(), h.Column()))->Eval(h, deltaT0, slewScaleFactor);
  }

  double RTAggregator::NormalizedTime(double time, int layer, int column) {
    return rts.at(RTvectorIndex.get(layer, column))->NormalizedTime(time, layer, column);
  }

  void RTAggregator::AddRT(RTParam* rtp) {
    rts.push_back(rtp);
  }

  void RTAggregator::Initialize(TString t0path, TString decodedDataPath) {
    for (RTParam* rt : rts) rt->Initialize(t0path, decodedDataPath);
  }
  
  void RTAggregator::InitMultiLayer() {
    if (rts.size() != 2) {
      std::cout << "Cannot configure RTAggregator object in "         << std::endl;
      std::cout << "multilayer mode without precisely 2 RT functions" << std::endl;
      return;
    }
    int hitL, hitC, ML;
    for (int hitL = 0; hitL < Geometry::MAX_TUBE_LAYER; hitL++) {
      for (int hitC = 0; hitC < Geometry::MAX_TUBE_COLUMN; hitC++) {
	ML = Geometry::MultiLayer(hitL);
	RTvectorIndex.set(hitL, hitC, ML);
	if (ML) {
	  rts.at(0)->SetIgnoreTube(hitL, hitC);
	}
	else {
	  rts.at(1)->SetIgnoreTube(hitL, hitC);
	}
      }
    }
  }
  
  void RTAggregator::InitTube(Geometry& geo) {
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
    int hitL, hitC;
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (tdc != geo.TRIGGER_MEZZ && geo.IsActiveTDCChannel(tdc, chan)) {
	  geo.GetHitLayerColumn(tdc, chan, &hitL, &hitC);
	  RTvectorIndex.set(hitL, hitC, index);
	  rts.at(index)->SetActiveTube(tdc, chan);
	  index++;
	}
      }
    }
    
  }


  void RTAggregator::InitMaxDriftTime(Geometry& geo) {
    double nRT = rts.size();
    TubeMap<double>* tF = &(rts.at(0)->tF);

    int hitL, hitC;

    // find quartiles of drift time
    TH1D* h_tmax = new TH1D("h_tmax", "", 100, tF->getMin(), tF->getMax()+1);
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
        if (tdc != geo.TRIGGER_MEZZ && geo.IsActiveTDCChannel(tdc, chan)) {
          geo.GetHitLayerColumn(tdc, chan, &hitL, &hitC);
	  h_tmax->Fill(tF->get(hitL, hitC));
	}
      }
    }
    h_tmax->Scale(1.0/h_tmax->GetEntries());
    TH1*  c_tmax = h_tmax->GetCumulative();

    // set which tubes to ignore for each rt
    for (size_t irt=0; irt<nRT; ++irt) {
      float min_tmax;
      if (irt==0) 
	min_tmax = tF->getMin();
      else
	min_tmax = c_tmax->GetBinCenter(c_tmax->FindFirstBinAbove(irt/nRT));
      float max_tmax;
      if (irt==nRT-1) 
	max_tmax = tF->getMax()+1;
      else 
	max_tmax = c_tmax->GetBinCenter(c_tmax->FindFirstBinAbove((irt+1)/nRT));

      rts.at(irt)->SetIgnoreAll();
      int nTubes = 0;
      for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
	for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	  if (tdc != geo.TRIGGER_MEZZ && geo.IsActiveTDCChannel(tdc, chan)) {
	    geo.GetHitLayerColumn(tdc, chan, &hitL, &hitC);
	    if (tF->get(hitL, hitC) >= min_tmax && tF->get(hitL, hitC) < max_tmax) {
	      RTvectorIndex.set(hitL, hitC, irt);
	      rts.at(irt)->SetActiveTube(hitL, hitC);
	    }
	  }
	}
      }
    }
  }
}
