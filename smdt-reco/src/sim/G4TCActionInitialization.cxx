#include "MuonSim/G4TCActionInitialization.h"

namespace MuonSim {
  G4TCActionInitialization::G4TCActionInitialization() : G4VUserActionInitialization(), _cp() {}

  G4TCActionInitialization::G4TCActionInitialization(TString confpath) : G4TCActionInitialization() {
    _cp = MuonReco::ConfigParser(confpath);
  }

  G4TCActionInitialization::~G4TCActionInitialization() {}

  void G4TCActionInitialization::BuildForMaster() const {
    SetUserAction(new G4TCRunAction);
  }

  void G4TCActionInitialization::Build() const {
    SetUserAction(new G4TCPrimaryGeneratorAction(_cp));
    SetUserAction(new G4TCRunAction(_cp));
    SetUserAction(new G4TCEventAction(_cp));
    SetUserAction(new G4TCSteppingAction);
  }

}
