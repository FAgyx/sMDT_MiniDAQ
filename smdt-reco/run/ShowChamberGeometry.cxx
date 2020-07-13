#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4GDMLParser.hh"
#include "G4VModularPhysicsList.hh"
#include "G4StepLimiterPhysics.hh"
#include "FTFP_BERT.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif


#include "MuonSim/G4TestStandConstruction.h"
#include "MuonSim/G4TCActionInitialization.h"

using namespace MuonSim;

int main(int argc, char* argv[])
{

  // load the spacer frame from GDML
  G4GDMLParser parser;
  parser.Read("raw/BMG.gdml");

  // construct the default run manager
  G4VModularPhysicsList* physicsList = new FTFP_BERT;
  physicsList->RegisterPhysics(new G4StepLimiterPhysics());
  G4RunManager* runManager = new G4RunManager;
  runManager->SetUserInitialization(new G4TestStandConstruction(parser.GetWorldVolume()));
  runManager->SetUserInitialization(physicsList);
  runManager->SetUserInitialization(new G4TCActionInitialization);
  runManager->Initialize();
  

  // create UI manager and show geometry
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

#ifdef G4UI_USE
  G4UIExecutive* ui = new G4UIExecutive(argc, argv);
#ifdef G4VIS_USE
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();
  UImanager->ApplyCommand("/control/execute conf/vis.mac");
#endif
  ui->SessionStart();
#ifdef G4VIS_USE
  delete visManager;
#endif 
  delete ui;
#endif
  delete runManager;
  return 0;
}
