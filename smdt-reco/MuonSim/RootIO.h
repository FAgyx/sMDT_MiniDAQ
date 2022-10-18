#ifndef MuonSim_RootIO
#define MuonSim_RootIO

#include "TSystem.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"

#include "MuonReco/Event.h"
#include "MuonReco/IOUtility.h"

namespace MuonSim {
  /*! \class RootIO RootIO.h "MuonSim/RootIO.h"
   *  \brief Singleton for writing out to .root files
   *  
   *  Implementation of singleton pattern to provide 
   *  global access to a TTree and TFile.  Allows user
   *  to write output during a Geant4 session other than
   *  through AnalysisManager, in particular a TTree of
   *  MuonReco::Event objects for resolution measurement
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
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
