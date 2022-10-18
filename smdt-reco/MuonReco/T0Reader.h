#ifndef MuonReco_T0Reader
#define MuonReco_T0Reader

#include "TFile.h"
#include "TTree.h"
#include "TVectorD.h"

#include "MuonReco/Geometry.h"
#include "MuonReco/T0Fit.h"

namespace MuonReco {
  /*! \class T0Reader T0Reader.h "MuonReco/T0Reader.h"
   *  \brief Simple access to fit parameters from T0Fit class
   *
   *  The T0Fit class, as used in the DoT0Fit macro, writes spectrum fit 
   *  parameters to a TTree.  This class simplifies setting pointers and 
   *  reading out data from TTrees.  It implements the singleton pattern
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
  class T0Reader {
  public:
    virtual ~T0Reader();

    static T0Reader* GetInstance(TString t0path);
    
    void UpdatePath           (TString t0path);
    void GetEntry             (int iEntry);
    void SetBranchAddresses   (int* tdc, int* ch, int* layer, int* col, TVectorD* fitParams);
    int  GetEntries           ();

  protected:
    T0Reader(TString t0path);
    
  private:
    TFile* t0file = 0;
    TTree* fitData = 0;
  };  
}

#endif
