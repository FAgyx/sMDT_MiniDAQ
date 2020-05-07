#include "src/Hit.cpp"
#include "src/RecoObject.cpp"

#ifndef MUON_CLUSTER
#define MUON_CLUSTER


namespace Muon {
  class Cluster : public RecoObject {
  public:
    Cluster ();
    Cluster (Hit h);
    ~Cluster()      {};
    
    void        Merge(Cluster c);
    vector<Hit> Hits ();
    int         Size ();

    void        Draw () override;
    
  private:
    vector<Hit> hits;    
  };
  
  Cluster::Cluster() {
    hits = vector<Hit>();
  }
  
  Cluster::Cluster(Hit h) {
    hits.push_back(h);
  }

  vector<Hit> Cluster::Hits() {
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

#endif
