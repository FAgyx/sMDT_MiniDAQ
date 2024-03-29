#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4GDMLParser.hh"
#include "G4VModularPhysicsList.hh"
#include "G4StepLimiterPhysics.hh"
#include "FTFP_BERT.hh"


#include "MuonSim/G4TestStandConstruction.h"
#include "MuonSim/G4TCActionInitialization.h"

#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"

using namespace MuonSim;
using namespace MuonReco;

int main(int argc, char* argv[])
{

  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry"));
  G4TestStandConstruction::ResetConstants();
  TString spacerType = cp.items("General").getStr("Spacer", "BMG", 0);

  // construct the default run manager
  G4VModularPhysicsList* physicsList = new FTFP_BERT;
  physicsList->RegisterPhysics(new G4StepLimiterPhysics());
  G4RunManager* runManager = new G4RunManager;
  runManager->SetUserInitialization(new G4TestStandConstruction(spacerType));
  runManager->SetUserInitialization(physicsList);
  runManager->SetUserInitialization(new G4TCActionInitialization(ap.getTString("--conf")));

  // initialize the G4 kernel
  runManager->Initialize();
  

  // create UI manager and show geometry
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  UImanager->ApplyCommand("/run/verbose 1");
  //UImanager->ApplyCommand("/event/verbose 1");
  //UImanager->ApplyCommand("/tracking/verbose 1");

  // start a run
  int numberOfEvent = cp.items("General").getInt("NEvents");
  runManager->BeamOn(numberOfEvent);

  // job termination
  delete runManager;
  return 0;
}
