#include "MuonSim/G4TestStandConstruction.h"

/*! Implementation file for MuonSim::G4TestStandConstruction
 * 
 * \author Kevin Nelson
 *         kevin.nelson@cern.ch
 * \date   3 June 2020
 * 
 */

namespace MuonSim {
  
  G4double G4TestStandConstruction::pRMin     = 7.1    * CLHEP::mm;  // inner tube radius
  G4double G4TestStandConstruction::pRMax     = 7.5    * CLHEP::mm;  // outer tube radius 
  G4double G4TestStandConstruction::pDz       = 564.5  * CLHEP::mm;  // half tube length
  G4double G4TestStandConstruction::pDPhi     = 360    * CLHEP::deg; // angle of the segment
  G4double G4TestStandConstruction::pSPhi     = 0      * CLHEP::deg; // angle start position
  G4double G4TestStandConstruction::pRMaxWire = 25.0   * CLHEP::um;  // wire radius
    
  G4int    G4TestStandConstruction::nTubesPerLayer         = 54;
  G4int    G4TestStandConstruction::nLayersPerMultiLayer = 4;
  G4int    G4TestStandConstruction::nMultiLayers         = 2;
    
  G4double G4TestStandConstruction::layerSpacing         = 13.0769836 * CLHEP::mm;
  G4double G4TestStandConstruction::multiLayerSpacing    = 224.231    * CLHEP::mm;
  G4double G4TestStandConstruction::columnSpacing        = 15.1       * CLHEP::mm;
    
  G4String G4TestStandConstruction::TubeLogicalVolumeName   = "sMDT_tube";
  G4String G4TestStandConstruction::LayerLogicalVolumeName  = "sMDT_layer";
  G4String G4TestStandConstruction::LayerPhysicalVolumeName = "sMDT_layer_%d";
    
  G4VisAttributes G4TestStandConstruction::airVisualisation = G4VisAttributes();
  G4VisAttributes G4TestStandConstruction::alVisualisation = G4VisAttributes();
  G4VisAttributes G4TestStandConstruction::wireVisualisation = G4VisAttributes();


  void G4TestStandConstruction::ResetConstants() {

    G4TestStandConstruction::pDz               = MuonReco::Geometry::tube_length * CLHEP::m / 2.0;  // half tube length
    G4TestStandConstruction::nTubesPerLayer    = MuonReco::Geometry::MAX_TUBE_COLUMN;
    G4TestStandConstruction::multiLayerSpacing = MuonReco::Geometry::ML_distance * CLHEP::mm;
  }


  G4TestStandConstruction::G4TestStandConstruction(TString frameType/*=""*/) {
    spacerType = frameType;
  }

  G4VPhysicalVolume* G4TestStandConstruction::Construct() {
    // initialize materials and visualisation parameters
    ConstructMaterials();
    
    // construct the world volume, filled with air
    G4double world_hx = 2.0*CLHEP::m;
    G4double world_hy = 2.0*CLHEP::m;
    G4double world_hz = 2.0*CLHEP::m;
    G4Box* worldBox = new G4Box("World", world_hx, world_hy, world_hz);

    G4Material* Air = G4Material::GetMaterial("G4_AIR");

    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, Air, "World");
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(0,
						     G4ThreeVector(0*CLHEP::m,0*CLHEP::m,0*CLHEP::m),
						     worldLog,
						     "World",
						     0,
						     false,
						     0);

    // build the geometry
    worldLog->SetVisAttributes(airVisualisation);
    AddTubes(worldLog);

    // add the spacer frame from gdml 
    if (!spacerType.CompareTo("BMG")) {
      G4GDMLParser parser;
      parser.Read("raw/BMG.gdml");
      spacerFrame = parser.GetWorldVolume();
      G4LogicalVolume* frameLogical = spacerFrame->GetLogicalVolume()->GetDaughter(0)->GetLogicalVolume();
      frameLogical->SetMaterial(G4Material::GetMaterial("G4_Al"));
      frameLogical->SetVisAttributes(alVisualisation);

      G4RotationMatrix  rotation = G4RotationMatrix();
      rotation.rotateZ(90*CLHEP::deg);
      G4ThreeVector position = G4ThreeVector(nTubesPerLayer*columnSpacing + 4.54*pRMax, 
					     0,
					     pRMax + 3*layerSpacing + .2*pRMax);
      G4Transform3D transform = G4Transform3D(rotation, position);

      G4VPhysicalVolume* framePhys  = new G4PVPlacement(transform,
							frameLogical,
							"SpacerFrame",
							worldLog,
							false,
							0);
      
      //frameLogical->SetVisAttributes(alVisualisation);
    }
    else if (!spacerType.CompareTo("BIS1")) {
      /*
      G4GDMLParser parser;
      parser.Read("raw/BIS1.gdml");
      spacerFrame = parser.GetWorldVolume();
      G4LogicalVolume* frameLogical = spacerFrame->GetLogicalVolume()->GetDaughter(0)->GetLogicalVolume();
      frameLogical->SetMaterial(G4Material::GetMaterial("G4_Al"));
      frameLogical->SetVisAttributes(alVisualisation);

      G4RotationMatrix  rotation = G4RotationMatrix();
      rotation.rotateZ(90*CLHEP::deg);
      rotation.rotateY(90*CLHEP::deg);
      G4ThreeVector position = G4ThreeVector(0,
					     -20.0*CLHEP::mm,
					     -5.0*CLHEP::mm);

      G4Transform3D transform = G4Transform3D(rotation, position);

      G4VPhysicalVolume* framePhys  = new G4PVPlacement(transform,
                                                        frameLogical,
                                                        "SpacerFrame",
                                                        worldLog,
                                                        false,
                                                        0);
      */
    }
    else {
      // no spacer frame
    }
    return worldPhys;
  }

  void G4TestStandConstruction::AddTubes(G4LogicalVolume* worldLog) {
    // use singletons to get any necessary refrences
    G4Material* Al    = G4Material::GetMaterial("G4_Al");
    G4Material* Air   = G4Material::GetMaterial("G4_AIR");
    G4Material* WRe   = G4Material::GetMaterial("W-Re");
    G4Material* ArCo2 = G4Material::GetMaterial("ArCo2");

    // create the (single) pointers to tube and layer logical volumes
    G4Tubs* tubeSolid = new G4Tubs("tubeSolid", pRMin,     pRMax,     pDz, pSPhi, pDPhi);
    G4Tubs* wireSolid = new G4Tubs("wireSolid", 0,         pRMaxWire, pDz, pSPhi, pDPhi);
    G4Tubs* gasSolid  = new G4Tubs("gasSolid",  pRMaxWire, pRMin,     pDz, pSPhi, pDPhi);

    G4LogicalVolume* tubeLog = new G4LogicalVolume(tubeSolid, Al, TubeLogicalVolumeName);
    tubeLog->SetVisAttributes(alVisualisation);
    G4LogicalVolume* wireLog = new G4LogicalVolume(wireSolid, WRe, "wireLogical");
    wireLog->SetVisAttributes(wireVisualisation);
    fTubeGasLogical          = new G4LogicalVolume(gasSolid,  ArCo2, "gasLogical");
    fTubeGasLogical->SetVisAttributes(airVisualisation);

    // place a wire in each tube and fill with gas
    new G4PVPlacement(G4Transform3D(), wireLog,         "wirePhysical", tubeLog, 0, 0);
    new G4PVPlacement(G4Transform3D(), fTubeGasLogical, "gasPhysical",  tubeLog, 0, 0);

    // create the logical volume for a single layer
    G4Box* layerMotherBox = new G4Box("layerMotherBox", pRMax*nTubesPerLayer, pRMax, pDz); 
    G4LogicalVolume* layerLog = new G4LogicalVolume(layerMotherBox, Air, LayerLogicalVolumeName);
    layerLog->SetVisAttributes(airVisualisation);
    G4PVReplica* layer = new G4PVReplica("Layer", tubeLog, layerLog, kXAxis, nTubesPerLayer, columnSpacing);

    // place the logical volume several times
    for (G4int iLayer = 0; iLayer < nLayersPerMultiLayer*nMultiLayers; iLayer++) {
      AddLayer(worldLog, iLayer);    
    }
  }

  void G4TestStandConstruction::AddLayer(G4LogicalVolume* worldLog, G4int iLayer) {
    // use singletons to get any necessary refrences
    G4LogicalVolumeStore* logStoreInstance = G4LogicalVolumeStore::GetInstance();
    G4LogicalVolume*      layerLog         = logStoreInstance->GetVolume(LayerLogicalVolumeName);

    // create a unique placement for this layer
    char layerName[100];
    std::sprintf(layerName, LayerPhysicalVolumeName, iLayer);

    G4double x = (iLayer % 2 == 0) ?    0 : columnSpacing/2.0;
    x += pRMax;
    x += nTubesPerLayer*columnSpacing/2.0;
    G4double z = (iLayer % 4)*layerSpacing + std::floor(iLayer/4) * multiLayerSpacing;
    z += pRMax;

    G4RotationMatrix  rotation = G4RotationMatrix();
    rotation.rotateX(90*CLHEP::deg);
    G4ThreeVector position = G4ThreeVector(x, pDz, z);
    G4Transform3D transform = G4Transform3D(rotation, position);

    G4VPhysicalVolume* layerPlacement = new G4PVPlacement(transform,
							  layerLog, 
							  layerName, 
							  worldLog, false, iLayer);
  }

  void G4TestStandConstruction::ConstructSDandField() {
    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    G4String SDname;
    
    G4VSensitiveDetector* tubeGasDetector = new G4DriftTubeSD(SDname="/driftTube");
    SDman->AddNewDetector(tubeGasDetector);
    if (fTubeGasLogical)
      fTubeGasLogical->SetSensitiveDetector(tubeGasDetector);
    else 
      G4cout << "WARNING: SENSITIVE DETECTOR NOT FOUND" << G4endl;
  }

  void G4TestStandConstruction::ConstructMaterials() {
    G4NistManager* man = G4NistManager::Instance();
    man->FindOrBuildMaterial("G4_Al");
    man->FindOrBuildMaterial("G4_AIR");
    G4Material* Ar  = man->FindOrBuildMaterial("G4_Ar");
    G4Material* CO2 = man->FindOrBuildMaterial("G4_CARBON_DIOXIDE");

    // create the tungsten-rhenium wire mixture
    G4double z, a, fractionmass, density;
    G4String name, symbol;
    G4int ncomponents;
    
    a = 183.84 * CLHEP::g/CLHEP::mole;
    G4Element* elW = new G4Element(name="Tungsten", symbol="W", z=74., a);
    
    a = 186.21 * CLHEP::g/CLHEP::mole;
    G4Element* elRe = new G4Element(name="Rhenium", symbol="Re", z=75., a);

    density = 19.3 * 0.97 * CLHEP::g/CLHEP::cm3 + 21.02 * 0.03 * CLHEP::g/CLHEP::cm3;
    G4Material* WRe = new G4Material(name="W-Re", density, ncomponents=2);
    WRe->AddElement(elW, fractionmass=97*CLHEP::perCent);
    WRe->AddElement(elRe,fractionmass=03*CLHEP::perCent);


    // create the argon-co2 gas mixture
    density = 3.*(0.97*Ar->GetDensity() + 0.03*CO2->GetDensity());
    G4Material* ArCo2 = new G4Material(name="ArCo2", density, ncomponents=2);
    ArCo2->AddMaterial(Ar,  fractionmass=97*CLHEP::perCent);
    ArCo2->AddMaterial(CO2, fractionmass=03*CLHEP::perCent);

    // construct the visualisation for each material
    airVisualisation.SetVisibility(0);
    airVisualisation.SetDaughtersInvisible(0);

    alVisualisation.SetColor(132./255., 135./255., 137./255.);

    wireVisualisation.SetColor(218./255., 165./255., 32./255.);
  }
}
