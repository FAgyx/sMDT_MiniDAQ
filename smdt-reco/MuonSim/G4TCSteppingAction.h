#ifndef MUONSIM_G4UserSteppingAction
#define MUONSIM_G4UserSteppingAction

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"
#include "g4root.hh" // includes AnalysisManager

#include "MuonSim/G4TCRunAction.h"

namespace MuonSim {
  /*! \class G4TCSteppingAction G4TCSteppingAction.h "MuonSim/G4TCSteppingAction.h"
   *  \brief Action to be taken at each step of the simulation
   * 
   *  Define actions to be taken at each step of the Geant4 simulation
   *  for the University of Michigan cosmic test stand.
   *  Implementation of an optional derived class
   * 
   *  This class is useful for retrieving MC truth information from particles
   *  for instance for getting truth information of particles leaving the 
   *  world volume
   * 
   *  \author Kevin Nelson
   *          kevin.nelson@cern.ch
   *  \date   3 June 2020
   */
  class G4TCSteppingAction : public G4UserSteppingAction {
  public:

    /*! \brief Constructor
     *
     *  
     */
    G4TCSteppingAction();

    /*! \brief Destructor
     * 
     *  No pointers to delete, any pointer referenced is managed by Geant
     */
    virtual ~G4TCSteppingAction();

    /*! \brief Actions to perform on each step
     * 
     *  Optional override of stepping actions
     *  Fills histograms with MC truth information for this step
     *  \see G4TCRunAction for all histograms booked for this run
     *
     *  \param astep The step this action references
     */
    virtual void UserSteppingAction(const G4Step* astep) override;

  };
}
#endif
