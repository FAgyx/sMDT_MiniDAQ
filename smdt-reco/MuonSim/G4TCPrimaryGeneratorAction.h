#ifndef MuonSim_G4TCPrimaryGeneratorAction
#define MuonSim_G4TCPrimaryGeneratorAction

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"
#include "Randomize.hh"
#include "g4root.hh"

#include "MuonSim/G4TestStandConstruction.h"
#include "MuonSim/G4TCRunAction.h"

#include "MuonReco/ConfigParser.h"

namespace MuonSim {
  /*! \class G4TCPrimaryGeneratorAction G4TCPrimaryGeneratorAction "MuonSim/G4TCPrimaryGeneratorAction.h"
   * 
   *  \brief Generator for the primary particle
   *
   *  Mandatory override of the primary generator
   *  for the University of Michigan sMDT cosmic
   *  ray test stand.
   *
   *  This class allows the user to define the 
   *  kinematics and species of the primary 
   *  particle, in this case cosmic muons
   *
   *  \author Kevin Nelson
   *          kevin.nelson@cern.ch
   *  \date   3 June 2020
   */
  class G4TCPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
  public:
    /*! \brief Constructor
     *  
     *  Instantiate the particle gun and set the default kinematic settings
     *  \param particleName Name for muons
     *  \param energy Default energy is 4GeV, but may be randomized by GeneratePrimaries()
     *  \param position Default position is above the detector, but may be randomized by GeneratePrimaries()
     *  \param momentumDirection Default is down, but is randomized by GeneratePrimaries()
     */
    G4TCPrimaryGeneratorAction(const G4String& particleName = "mu-",
			       G4double energy = 4.*CLHEP::GeV,
			       G4ThreeVector position= G4ThreeVector(0.5*CLHEP::m,0.5*CLHEP::m,2*CLHEP::m),
			       G4ThreeVector momentumDirection = G4ThreeVector(0,0,-1*CLHEP::m));

    G4TCPrimaryGeneratorAction(MuonReco::ConfigParser &cp);

    /*! \brief Destructor 
     * 
     *  Delete the particle gun pointer here
     */
    virtual ~G4TCPrimaryGeneratorAction();

    /*! \brief Generate the primary particle
     * 
     *  Generate a cosmic muon with kinematic settings randomized
     *  Mandatory override of pure virtual method
     *
     *  \param anEvent G4Event pointer to which a single primary will be added
     */
    virtual void GeneratePrimaries(G4Event* anEvent) override;

  protected:
    void Init();
    void SetKinematics();
    bool WillPassThroughChamber();
    G4double TruncatedExponential(G4double mean, G4double cut);

    G4ParticleGun* fParticleGun; //< the particle gun pointer
    G4String       fParticleName;
    G4double       fParticleEnergy;
    G4double       fSoftElectronFraction;
    G4double       theta;
    G4double       phi;
    G4ThreeVector  fParticlePosition;
    G4ThreeVector  fParticleMomentumDirection;
    G4bool         fRandomizeEnergy;
    G4bool         fRandomizePosition;
    G4bool         fRandomizeMomentumDirection;
    G4bool         fRandomizeParticle;
    G4int          pdgCode;
  };
}

#endif
