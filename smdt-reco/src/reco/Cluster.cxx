#include "MuonReco/Cluster.h"

namespace MuonReco {  
  Cluster::Cluster() {
    hits = std::vector<Hit>();
  }
  
  Cluster::Cluster(Hit h) {
    hits.push_back(h);
  }

  std::vector<Hit> Cluster::Hits() {
    return hits;
  }
  
  void Cluster::Merge(Cluster c) {
    for (Hit h : c.Hits()) {
      hits.push_back(h);
    }
  }
  
  int Cluster::Size() {
    return hits.size();
  }


  void Cluster::Draw() {}
}

