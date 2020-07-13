#ifndef MUON_TCActionInitialization
#define MUON_TCActionInitialization

#include "G4VUserActionInitialization.hh"

#include "G4TCActionInitialization.h"
#include "G4TCPrimaryGeneratorAction.h"
#include "G4TCRunAction.h"
#include "G4TCEventAction.h"
#include "G4TCSteppingAction.h"

#include "MuonReco/ConfigParser.h"

namespace MuonSim {
  /*! class G4TCActionInitialization G4TCActionInitialization.h "MuonSim/G4TCActionInitialization.h"
   * 
   * \brief Register actions to the primary generator
   * 
   * Derived class for action initialization in the University 
   * of Michigan sMDT Test Chamber Geant4 simulation.
   * 
   * Mandatory derived class for a Geant4 simulation.
   *
   * This class instantiates and registers a new 
   * G4TCPrimaryGeneratorAction class (derived from
   * G4VUserPrimaryGeneratorAction), which generates
   * the particles for the events.
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   3 June 2020
   */
  class G4TCActionInitialization : public G4VUserActionInitialization {
  public:
    G4TCActionInitialization();

    G4TCActionInitialization(TString confpath);

    virtual ~G4TCActionInitialization();

    /*!
     * For multithreaded Geant, only initialize the UserRunAction
     */
    virtual void BuildForMaster() const override;

    /*!
     * Define user action classes using the base class method
     * G4VUserActionInitialization::SetUserAction()
     *
     * Mandatory override of pure virtual method
     */
    virtual void Build() const override;

  protected:
    mutable MuonReco::ConfigParser _cp;
  };
}

#endif
