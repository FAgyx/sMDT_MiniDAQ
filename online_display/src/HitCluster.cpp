#include "src/Cluster.cpp"
#include "src/Event.cpp"
#include "src/Geometry.cpp"
#include "src/Hit.cpp"

void DoHitClustering(Event *e, Geometry geo) {
  Bool_t merge = kTRUE;
  vector<Cluster> clusters;

  for (auto hit : e->WireHits()) {
    clusters.push_back(Cluster(hit));
  }

  while (merge && clusters.size() >=2) {
    merge = kFALSE;
    for (int i = 0; i < clusters.size(); i++) {
      for (int j = i+1; j < clusters.size(); j++) {
        if (geo.AreAdjacent(clusters.at(i),clusters.at(j))) {
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
