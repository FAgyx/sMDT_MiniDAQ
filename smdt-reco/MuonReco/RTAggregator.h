#ifndef MUON_RT_AGGREGATOR
#define MUON_RT_AGGREGATOR

#include <iostream>
#include <vector>

#include "TString.h"

#include "MuonReco/RTParam.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Callable.h"

namespace MuonReco {
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

    double Eval          (Hit h, double deltaT0=0, double slewScaleFactor=1.0, double sigPropSF=1.0) override;
    double NormalizedTime(double time, int tdc_id, int ch_id) override;

    void   AddRT         (RTParam* rtp);
    void   InitMultiLayer();
    void   InitTube      (Geometry & geo);
    void   Initialize    (TString t0path, TString decodedDataPath);
    void   InitMaxDriftTime(Geometry & geo);

    std::vector<RTParam*> rts;

  private:
    TubeMap<int> RTvectorIndex;
  };
}

#endif
