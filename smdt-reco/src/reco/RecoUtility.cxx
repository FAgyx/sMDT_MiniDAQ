#include "MuonReco/RecoUtility.h"

namespace MuonReco {

  RecoUtility::RecoUtility() {
    MIN_HITS_NUMBER     = 6;
    MAX_HITS_NUMBER     = 12;
    MAX_TIME_DIFFERENCE = 8.0*25.0; // 200 ns is 8 coarse time steps

    MIN_CLUSTER_SIZE    = 2;
    MAX_CLUSTER_SIZE    = 216;

    MIN_CLUSTERS_PER_ML = 1;
    MAX_CLUSTERS_PER_ML = 1;
  }

  RecoUtility::RecoUtility(ParameterSet ps) : RecoUtility() {
    Configure(ps);
  }

  void RecoUtility::Configure(ParameterSet ps) {
    CHECK_TRIGGERS      = ps.getBool  ("CHECK_TRIGGERS",      1,        0);

    MIN_HITS_NUMBER     = ps.getInt   ("MIN_HITS_NUMBER",     6,        0);
    MAX_HITS_NUMBER     = ps.getInt   ("MAX_HITS_NUMBER",     12,       0);
    MAX_TIME_DIFFERENCE = ps.getDouble("MAX_TIME_DIFFERENCE", 8.0*25.0, 0);

    MIN_CLUSTER_SIZE    = ps.getInt   ("MIN_CLUSTER_SIZE",    2,        0);
    MAX_CLUSTER_SIZE    = ps.getInt   ("MAX_CLUSTER_SIZE",    216,      0);

    MIN_CLUSTERS_PER_ML = ps.getInt   ("MIN_CLUSTERS_PER_ML", 1,        0);
    MAX_CLUSTERS_PER_ML = ps.getInt   ("MAX_CLUSTERS_PER_ML", 1,        0);
  }

  bool RecoUtility::CheckEvent(Event e) {
    
    // need precisely one trigger for data
    if (CHECK_TRIGGERS && e.TriggerHits().size() != 1) {
      //std::cout << "Rejecting event for not having precisely one trigger (has " << e.TriggerHits().size() << ")" << std::endl;
      return NOTPASTEVENTCHECK;
    }
    // need at least 4 wire hits
    if (e.WireHits().size() < MIN_HITS_NUMBER) {
      //std::cout << "Rejecting event for having too few hits (has " << e.WireHits().size() << ")" << std::endl;
      //std::cout << "Event has " << e.WireSignals().size() << " wire signals" << std::endl;
      return NOTPASTEVENTCHECK;
    }
    
    if (e.WireHits().size() > MAX_HITS_NUMBER) {
      //std::cout << "Rejecting event for having too many hits (has " << e.WireHits().size() << ")" << std::endl;
      return NOTPASTEVENTCHECK;
    }

    int nML0 = 0;
    int nML1 = 0;
    for (Cluster c : e.Clusters()) {
      if (Geometry::MultiLayer(c.Hits().at(0).Layer()) == 0)
	nML0++;
      if (Geometry::MultiLayer(c.Hits().at(0).Layer()) == 1) 
	nML1++;

      // check that no cluster is too large or small
      if (c.Size() < MIN_CLUSTER_SIZE || c.Size() > MAX_CLUSTER_SIZE) {
	//std::cout << "Rejecting event for clusters of wrong size present" << std::endl;
	return NOTPASTEVENTCHECK;      
      }
    }

    // check that there is the right number of clusters in each multilayer
    if (nML0 < MIN_CLUSTERS_PER_ML || nML0 > MAX_CLUSTERS_PER_ML ||
	nML1 < MIN_CLUSTERS_PER_ML || nML1 > MAX_CLUSTERS_PER_ML) {
      //std::cout << "Rejecting event for wrong number of clusers in each multilayer" << std::endl;
      //std::cout << "nML0 = " << nML0 << " nML1 = " << nML1 << std::endl;
      //std::cout << "N hits = " << e.WireHits().size() << std::endl;
      return NOTPASTEVENTCHECK;
    }

    // need the maximum time difference to be less than max time difference
    double max_time = 0;
    double min_time = 1e100;
    
    for (int i = 0; i < e.WireHits().size(); i++) {
      if (e.WireHits().at(i).TDCTime() > max_time) {
	max_time = e.WireHits().at(i).TDCTime();
      }
      if (e.WireHits().at(i).TDCTime() < min_time) {
	min_time = e.WireHits().at(i).TDCTime();
      }
    }
    if ((max_time - min_time <= MAX_TIME_DIFFERENCE) && (max_time - min_time >= 0)) {
      return PASTEVENTCHECK;
    }
    else {
      //std::cout << "Rejecting event for hit timing" << std::endl;
      return NOTPASTEVENTCHECK;
    }
  } // end check event


  void RecoUtility::DoHitClustering(Event *e) {
    Bool_t merge = kTRUE;
    std::vector<Cluster> clusters;

    for (auto hit : e->WireHits()) {
      clusters.push_back(Cluster(hit));
    }

    while (merge && clusters.size() >=2) {
      merge = kFALSE;
      for (int i = 0; i < clusters.size(); i++) {
        for (int j = i+1; j < clusters.size(); j++) {
          if (Geometry::AreAdjacent(clusters.at(i),clusters.at(j))) {
            clusters.at(i).Merge(clusters.at(j));
            clusters.erase(clusters.begin()+j);
            merge = kTRUE;
            break;
          }
        } // end for: c2
        if (merge) break;
      } // end for: c1
    } // end while: merge

    // modify event
    for (auto c : clusters) {
      if (c.Size() >=2) {
        e->AddCluster(c);
      }
    }
  }



  void RecoUtility::DoHitFinding(Event *e, TimeCorrection tc, Geometry geo) {
    Hit h;
    double adc_time = 0.0;
    int layer, column;
    double hx, hy;
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
	if (trig.IsFirstSignal() && adc_time != 0.0 && trig.Time() > 290 && trig.Time() < 350) {
	  h = Hit(trig.Time(), adc_time, trig.Time(), trig.Time(), trig.TDC(), trig.Channel(), -1, -1, -1, -1);
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
	    if ((sig2.Type() == Signal::FALLING) && sig.SameTDCChan(sig2)) {
	      adc_time = sig2.Time() - sig.Time();
	      break;
	    }
	  } // end for: s_iter2
	  if (sig.IsFirstSignal() && adc_time > 40) {
	    corr_time = drift_time - tc.SlewCorrection(adc_time);
	    
	    geo.GetHitLayerColumn(sig.TDC(), sig.Channel(), &layer, &column);
	    Geometry::GetHitXY(layer, column, &hx, &hy);
	    h = Hit(sig.Time(), adc_time, drift_time, corr_time, sig.TDC(), sig.Channel(), layer, column, hx, hy);
	    e->AddSignalHit(h);
	  }
	}
      } // end for: s_iter
    } // end if: nonzero number of triggers
  }
}
