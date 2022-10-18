#include "MuonReco/Cluster.h"

namespace MuonReco {  
  /*! \brief Constructor */
  Cluster::Cluster() {
    hits = std::vector<Hit>();
  }
  
  /*! \brief Constructor 
   * Add this hit to list
   */
  Cluster::Cluster(Hit h) : Cluster() {
    hits.push_back(h);
  }

  /*! Get the vector of clustered hits 
   */
  std::vector<Hit> Cluster::Hits() {
    return hits;
  }
  
  /*! Push all hits in c onto this cluster's hit vector 
   */
  void Cluster::Merge(Cluster c) {
    for (Hit h : c.Hits()) {
      hits.push_back(h);
    }
  }
  
  /*! return size of underlying hit vector
   */
  int Cluster::Size() {
    return hits.size();
  }

  /*! Draw this cluster on the event display */
  void Cluster::Draw() {}
}

