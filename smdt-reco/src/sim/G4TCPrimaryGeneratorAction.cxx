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
    fRandomizeParticle           = cp.items("Primary").getBool("RandomizeParticle",          0, 0);
    fSoftElectronFraction        = cp.items("Primary").getDouble("SoftElectronFraction", 0.3, 0);
    
    if (fRandomizeParticle) {
      fParticleName = "mu-";
    }
    else {
      fParticleName = cp.items("Primary").getStr("Particle");
    }


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
    pdgCode = particle->GetPDGEncoding();
    
    SetKinematics();
  }


  void G4TCPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

    // here you can randomize the particle kinematics
    theta = 0;
    phi = 0;
    do {
      if (fRandomizePosition) {
	G4double x = G4UniformRand()*G4TestStandConstruction::columnSpacing*
	  G4TestStandConstruction::nTubesPerLayer;
	G4double y = G4UniformRand()*G4TestStandConstruction::pDz*2.0; 
	G4double z = 2.1*MuonReco::Geometry::getMeanYPosition()*CLHEP::mm;
	fParticlePosition = G4ThreeVector(x,y,z);
      }
      if (fRandomizeMomentumDirection) {
	phi   = G4UniformRand()*CLHEP::twopi;
	theta;
	bool accepted = false;
	while (!accepted) { // use rejection sampling for cos^2 distribution
	  G4double x = G4UniformRand()*CLHEP::halfpi - 0.5*CLHEP::halfpi;
	  G4double y = G4UniformRand();
	  if (cos(x)*cos(x) > y) {
	    accepted = true;
	    theta = x;
	  }
	}
	fParticleMomentumDirection = G4ThreeVector(sin(theta)*cos(phi), sin(theta)*sin(phi), -1.*cos(theta));
      }
    }
    while(!WillPassThroughChamber());


    if (fRandomizeParticle) {
      if (G4UniformRand() < fSoftElectronFraction) { // approximately 30% of cosmics are soft electrons
	fParticleName = "e-";
      }
      else {
	fParticleName = "mu-";
      }
      G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
      G4ParticleDefinition* particle = particleTable->FindParticle(fParticleName);
      fParticleGun->SetParticleDefinition(particle);      
      pdgCode = particle->GetPDGEncoding();
    }

    if (fRandomizeEnergy) {
      if (pdgCode == 13) {
	if (G4UniformRand() < 0.39244) { // derived from double exponential fit to PDG cosmic ray data
	  fParticleEnergy = TruncatedExponential(5.52641905*CLHEP::GeV, 0.6*CLHEP::GeV);
	}
	else {
	  fParticleEnergy = TruncatedExponential(1.418029*CLHEP::GeV, 0.6*CLHEP::GeV);
	}
      }
      else {
	fParticleEnergy = TruncatedExponential(50*CLHEP::MeV, 0) + 10*CLHEP::MeV;
      }
    }

    // then generate the primary(s)
    SetKinematics();
    analysisManager->FillNtupleIColumn(G4TCRunAction::NPDGCode,  pdgCode);
    analysisManager->FillNtupleDColumn(G4TCRunAction::NTheta,    theta*180./CLHEP::pi);
    analysisManager->FillNtupleDColumn(G4TCRunAction::NMomentum, fParticleEnergy/CLHEP::GeV);
    fParticleGun->GeneratePrimaryVertex(anEvent);
  }
  
  G4double G4TCPrimaryGeneratorAction::TruncatedExponential(G4double mean, G4double cut) {
    G4double result;
    while (true) {
      G4double cdf = G4UniformRand();
      result = -mean * log(1-cdf);

      if (result < cut) {
	if (G4UniformRand() < exp(-result/mean)/exp(-cut/mean)) break;
      }
      else break;

    }
    return result;
  }

  bool G4TCPrimaryGeneratorAction::WillPassThroughChamber() {
    // read some geometric constants
    double xMax = G4TestStandConstruction::columnSpacing * G4TestStandConstruction::nTubesPerLayer;
    double yMax = 2* G4TestStandConstruction::pDz;
    double zTopPlane = G4TestStandConstruction::multiLayerSpacing + 
      (G4TestStandConstruction::nLayersPerMultiLayer-1)*G4TestStandConstruction::layerSpacing;

    // check if passes through top plane
    double scaleFactor = (fParticlePosition.z()-zTopPlane)/abs(fParticleMomentumDirection.z());
    G4ThreeVector topPlaneNominalPosition = fParticlePosition + scaleFactor * fParticleMomentumDirection;
    if (topPlaneNominalPosition.x() < 0 || topPlaneNominalPosition.x() > xMax ||
	topPlaneNominalPosition.y() < 0 || topPlaneNominalPosition.y() > yMax) return false;
    
    // check if passes through the bottom plane
    scaleFactor = fParticlePosition.z()/abs(fParticleMomentumDirection.z());
    G4ThreeVector bottomPlaneNominalPosition = fParticlePosition + scaleFactor * fParticleMomentumDirection;
    if (bottomPlaneNominalPosition.x() < 0 || bottomPlaneNominalPosition.x() > xMax ||
	bottomPlaneNominalPosition.y() < 0 || bottomPlaneNominalPosition.y() > yMax) return false;

    return true;
  }

  void G4TCPrimaryGeneratorAction::SetKinematics() {
    fParticleGun->SetParticleEnergy(fParticleEnergy);
    fParticleGun->SetParticlePosition(fParticlePosition);
    fParticleGun->SetParticleMomentumDirection(fParticleMomentumDirection);
  }
}
