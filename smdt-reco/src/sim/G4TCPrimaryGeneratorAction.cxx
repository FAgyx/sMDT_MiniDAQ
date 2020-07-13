#include "MuonSim/G4TCPrimaryGeneratorAction.h"

namespace MuonSim {
  G4TCPrimaryGeneratorAction::G4TCPrimaryGeneratorAction(const G4String& particleName /*= "mu-"*/,
							 G4double energy/* = 1.*MeV*/,
							 G4ThreeVector position /*=G4ThreeVector(0.5,0.5,2)*/,
							 G4ThreeVector momentumDirection /*= G4ThreeVector(0,0,-1)*/) :
    fParticleName(particleName), 
    fParticleEnergy(energy),
    fParticleMomentumDirection(momentumDirection),
    fParticlePosition(position) {
    Init();
  }


  G4TCPrimaryGeneratorAction::G4TCPrimaryGeneratorAction(MuonReco::ConfigParser &cp) {
    // use cp to set default energy and particle type

    fRandomizeEnergy             = cp.items("Primary").getBool("RandomizeEnergy",            0, 0);
    fRandomizePosition           = cp.items("Primary").getBool("RandomizePosition",          0, 0);
    fRandomizeMomentumDirection  = cp.items("Primary").getBool("RandomizeMomentumDirection", 0, 0);
    fParticleName                = cp.items("Primary").getStr("Particle");

    if (fRandomizeEnergy) {
      fParticleEnergy            = 0;
    }
    else {
      fParticleEnergy            = cp.items("Primary").getDouble("Energy")*CLHEP::GeV;
    }


    if (fRandomizePosition) {
      fParticlePosition          = G4ThreeVector(0,0,0);
    }
    else {
      std::vector<double> input = cp.items("Primary").getDoubleVector("Position");
      if (input.size() != 3) {
	G4cout << "ERROR in G4TCPrimaryGeneratorAction configuration."<< G4endl
	       << "Position must be specified as a three vector in Position=x:y:z format" << G4endl
	       << "Or use the flag RandomizePosition = 1" << G4endl;
	throw 1;
      }
      fParticlePosition          = G4ThreeVector(input.at(0), input.at(1), input.at(2));
    }


    if (fRandomizeMomentumDirection) {
      fParticleMomentumDirection = G4ThreeVector(0,0,0);
    }
    else {
      std::vector<double> input = cp.items("Primary").getDoubleVector("MomentumDirection");
      if (input.size() != 3) {
	G4cout << "ERROR in G4TCPrimaryGeneratorAction configuration."<< G4endl
	       << "Momentum direction must be specified as a three vector in MomentumDirection=x:y:z format" << G4endl
	       << "Or use the flag RandomizeMomentumDirection = 1" << G4endl;
	throw 1;
      }
      fParticleMomentumDirection = G4ThreeVector(input.at(0), input.at(1), input.at(2));
    }


    Init();
  }

  G4TCPrimaryGeneratorAction::~G4TCPrimaryGeneratorAction() {
    delete fParticleGun;
  }

  void G4TCPrimaryGeneratorAction::Init() {
    G4int nofParticles = 1;
    fParticleGun = new G4ParticleGun(nofParticles);
    
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle = particleTable->FindParticle(fParticleName);

    fParticleGun->SetParticleDefinition(particle);

    SetKinematics();
  }


  void G4TCPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    // here you can randomize the particle kinematics
    if (fRandomizePosition) {
      G4double x = G4UniformRand()*G4TestStandConstruction::columnSpacing*
	G4TestStandConstruction::nTubesPerLayer;
      G4double y = G4UniformRand()*G4TestStandConstruction::pDz*2.0; 
      G4double z = 1.0*CLHEP::m;
      fParticlePosition = G4ThreeVector(x,y,z);
    }
    if (fRandomizeEnergy) {

    }
    if (fRandomizeMomentumDirection) {
      
    }

    // then generate the primary(s)
    SetKinematics();
    fParticleGun->GeneratePrimaryVertex(anEvent);
  }

  void G4TCPrimaryGeneratorAction::SetKinematics() {
    fParticleGun->SetParticleEnergy(fParticleEnergy);
    fParticleGun->SetParticlePosition(fParticlePosition);
    fParticleGun->SetParticleMomentumDirection(fParticleMomentumDirection);
  }
}
