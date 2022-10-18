#ifndef MUON_DriftTubeSD
#define MUON_DriftTubeSD

#include "G4VSensitiveDetector.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "g4root.hh"

#include "MuonSim/G4DriftTubeHit.h"
#include "MuonSim/G4TestStandConstruction.h"
#include "MuonSim/G4TCRunAction.h"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

namespace MuonSim {
  /*! \class G4DriftTubeSD G4DriftTube.h "MuonSim/G4DriftTubeSD.h"
   *  \brief Class to create hits in the drift tube sensitive gas volume
   *
   *  Concrete class derived from pure virtual base class G4VSensitiveDetector
   *  The user must instantiate this class and register it to the singleton 
   *  G4SDManager in the construtor of a concrete class derived from 
   *  G4VUserDetectorConstruction.
   *
   *  \author Kevin Nelson
   *          kevin.nelson@cern.ch
   *  \date   3 June 2020
   */
  class G4DriftTubeSD : public G4VSensitiveDetector {
  public:

    /*! \brief Constructor 
     *
     *  Call superclass constructor, initialize protected data members,
     *  and insert the static collection name into the protected member
     *  G4VSensitiveDetector::collectionName
     */
    G4DriftTubeSD(G4String name);

    /*! \brief Destructor
     *  
     *  No deletion is required, any allocated pointers are deleted by Geant
     */
    virtual ~G4DriftTubeSD();

    /*! \brief Assocate hit collections with this event
     *
     *  Instantiate the protected member fHitsCollection and add it to 
     *  hce, the hit collection of this event
     *  
     *  \param hce The hit collection of this event
     */
    virtual void   Initialize (G4HCofThisEvent *hce) override;


  protected:

    /*! \brief Instantiate hit objects and store in the hits collection
     *
     *  Called by G4SteppingManager when a G4Step passes through the 
     *  G4LogicalVolume paired with this sensitive detector.
     *  \returns G4bool true if successfully processes hits
     */
    virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

    G4DriftTubeHitsCollection* fHitsCollection; //< Aggregates MuonSim::DriftTubeHit objects.
    G4int fHCID;                                //< Hit collection ID corresponding to the drift tubes

  public:
    static G4String myCollectionName;           //< Create a static string for collection name for consistent access across the code
  };
}

#endif
