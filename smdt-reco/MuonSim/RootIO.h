#ifndef MuonSim_RootIO
#define MuonSim_RootIO

#include "TSystem.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

#include "MuonReco/Event.h"
#include "MuonReco/IOUtility.h"

namespace MuonSim {
  class RootIO {
  public:
    virtual ~RootIO();

    static RootIO* GetInstance(int runN);
    void SetTarget(MuonReco::Event* evt);
    void Fill();
    void Write();
    void Close();

  protected:
    RootIO(int runN);

  private:
    TFile* eFile;
    TTree* eTree;
  };
}

#endif
