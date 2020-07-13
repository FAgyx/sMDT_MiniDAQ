#ifndef MUON_G4TCEventAction
#define MUON_G4TCEventAction

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4UserEventAction.hh"
#include "globals.hh"
#include "g4root.hh"

#include "MuonSim/G4DriftTubeHit.h"
#include "MuonSim/G4TCRunAction.h"
#include "MuonSim/ReconstructionStrategy.h"
#include "MuonSim/MCTruthRecoStrategy.h"
#include "MuonSim/RootIO.h"

#include "MuonReco/ConfigParser.h"

namespace MuonSim {

  /*! \class G4TCEventAction G4TCEventAction.h "MuonSim/G4TCEventAction.h"
   *  \brief Class to manage actions taken at the event-by-event level
   * 
   *  Implementation of one of Geant4's optional UserAction classes for the 
   *  Unversity of Michigan sMDT cosmic ray test stand. This class books
   *  histograms for a particular event and analyzes processed events.
   *  This class is derived from G4UserEventAction.
   *  
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   3 June 2020
   */
  class G4TCEventAction : public G4UserEventAction {
  public: 
    /*! \brief Constructor
     * 
     *  Call superclass constructor G4UserEventAction, instantiate members
     */
    G4TCEventAction(MuonReco::ConfigParser cp);
    
    /*! \brief Destructor
     *
     *  No deletions necessary, any referenced pointers are managed elsewhere
     */
    virtual ~G4TCEventAction();

    /*! Get Hit collection IDs for this event
     */    
    virtual void BeginOfEventAction(const G4Event*) override;

    /*! \brief Fill histograms in the analysis manager.
     * 
     * Histograms may be booked in the class G4TCRunAction
     * or in this class's BeginOfEventAction method
     */
    virtual void EndOfEventAction  (const G4Event*) override;

  protected:
    G4int fHCID; //< Hit collection ID for the drift tubes
    ReconstructionStrategy* reco;
    int runNumber;
  };
}

#endif
