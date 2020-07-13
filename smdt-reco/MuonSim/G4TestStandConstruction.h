#ifndef MUON_G4TestStandConstruction
#define MUON_G4TestStandConstruction

#include <stdio.h>
#include <math.h>

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "globals.hh"
#include "G4NistManager.hh"
#include "G4PVReplica.hh"
#include "G4VisAttributes.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include "CLHEP/Units/PhysicalConstants.h"

#include "MuonSim/G4DriftTubeSD.h"

namespace MuonSim {
  /*! \class G4TestStandConstruction G4TestStandConstruction.h "MuonSim/G4TestStandConstruction.h"
   * \brief Describe test stand geometry
   * 
   * Class to describe the geometry and sensitive
   * detector components of the University of 
   * Michigan sMDT cosmic ray test chamber.
   *
   * Inherits from G4VUserDetectorConstruction 
   * and overrides two pure virtual methods:
   * Construct() and ConstructSDandField()
   * 
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   3 June 2020
   */
  class G4TestStandConstruction : public G4VUserDetectorConstruction {
  public:

    /*! \brief Constructor
     *
     *  Save the gdml pointer for further use if necessary
     * 
     *  \param gdmlVol optional argument to pass a volume loaded from 
     *         the GDML parser.  Used to incorporate difficult to describe 
     *         volumes like the spacer frame of the BMG chamber
     */
    G4TestStandConstruction(G4VPhysicalVolume* gdmlVol = 0);

    /*! \brief Construct the geometry
     * 
     *  Mandatory override to describe the physical
     *  structure of the detector including materials
     *  and geometry.
     */
    virtual G4VPhysicalVolume* Construct() override;

    /*! \brief Construct the Sensitive Detector and fields
     * 
     *  Mandatory override to describe the sensitive
     *  detector components and the EM fields in the
     *  experimental hall.  Registers sensitive detector for
     *  the drift tubes to the G4SDManager singleton pointer
     */
    virtual void ConstructSDandField() override;

  private:
    /*! \brief Add the tubes to the detector geometry
     * 
     *  Private method to add all tubes to the world volume
     */
    void AddTubes(G4LogicalVolume* worldLog);
    
    /*! \brief Add a layer of tubes to the geometry
     *
     *  Private method for ease of adding multiple layers
     */
    void AddLayer(G4LogicalVolume* worldLog, G4int nLayer);
    
    /*! \brief Instantiate G4Material objects used in the detector
     * 
     *  Private method to create the materials.  Wire, aluminum, air, 
     *  and tube gas are all constructed.  Materials are later accessed
     *  from the static method G4Material::GetMaterial, as they are 
     *  automatically registered when created.
     */
    void ConstructMaterials();

  public:
    // tube dimensions
    static const G4double pRMin;     //< tube inner wall radius
    static const G4double pRMax;     //< tube outer wall radius
    static const G4double pDz;       //< tube half length in z
    static const G4double pDPhi;     //< tube delta phi (360 degrees)
    static const G4double pSPhi;     //< tube start phi (arbitrary)
    static const G4double pRMaxWire; //< wire thickness (radius)

    // chamber attributes
    static const G4int    nTubesPerLayer;       //< Tubes in a layer
    static const G4int    nLayersPerMultiLayer; //< Layers per multilayer
    static const G4int    nMultiLayers;         //< Number of multilayers
    static const G4double layerSpacing;         //< Distance betwen tubes in adjacent layers
    static const G4double multiLayerSpacing;    //< Spacing between the multilayers (from bottom tube of one multilayer to bottom tube of the next)
    static const G4double columnSpacing;        //< Distance between adjacent tubes in the same layer

    // naming convention
    static const G4String TubeLogicalVolumeName;   //< logical volume name for tube, which contains the logical volume for gas and wire inside it
    static const G4String LayerLogicalVolumeName;  //< Logical volume name for the abstract layer
    static const G4String LayerPhysicalVolumeName; //< Physical volume templated name for a concrete layer
    
    // visualisation
    static G4VisAttributes airVisualisation;  //< Visualisation settings for air
    static G4VisAttributes alVisualisation;   //< Visualisation settings for aluminum
    static G4VisAttributes wireVisualisation; //< Visualisation settings for wires
    
    // useful pointers
    G4VPhysicalVolume* spacerFrame     = 0; //< pointer to spacer frame from GDML
    G4LogicalVolume*   fTubeGasLogical = 0; //< pointer to the logical volume for the tube sensitive detector
  };
}

#endif
