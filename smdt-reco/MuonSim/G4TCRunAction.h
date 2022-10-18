#ifndef MUON_G4TCRunAction
#define MUON_G4TCRunAction

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "g4root.hh"

#include "G4TestStandConstruction.h"

#include "MuonSim/RootIO.h"

#include "MuonReco/ConfigParser.h"
#include "MuonReco/IOUtility.h"

namespace MuonSim {

  /*! \class G4TCRunAction G4TCRunAction.h "MuonSim/G4TCRunACtion.h"
   * 
   *  \brief Action to be taken on each new run
   * 
   *  Implementation of one of Geant4's optional
   *  UserAction classes for the University of 
   *  Michigan sMDT cosmic ray test stand.
   *
   *  This class books and writes out histograms
   *  for the run
   * 
   *  \author Kevin Nelson
   *          kevin.nelson@cern.ch
   *  \date   3 June 2020
   */
  class G4TCRunAction : public G4UserRunAction {
  public:

    /*! \brief Constructor
     * 
     *  Calls the superclass constructor G4UserRunAction()
     *  Using the singleton pointer to the G4AnalysisManager, book
     *  histograms, create nTuples
     */
    G4TCRunAction();

    
    /*!
     * Constructor with parameters
     */
    G4TCRunAction(MuonReco::ConfigParser &cp);

    
    /*! \brief Destructor
     *
     *  Delete the analysis manager singleton
     */
    virtual ~G4TCRunAction();


    /*
     * Not yet implemented for this simulation.
     * Instantiate a derived class of G4Run and set variables
     * that affect physics tables.
     *
     * This method is invoked at BeamOn(), before the calcuation
     * of the physics tables
     * 
     * Optional override of virtual method
     */
    //virtual G4Run GenerateRun() override; 

    /*! \brief Book histograms for this run
     *
     * Optional override of virtual method
     */
    virtual void BeginOfRunAction(const G4Run*) override;

    /* \brief Close output for this run
     *
     * Optional override of virtual method
     */
    virtual void   EndOfRunAction(const G4Run*) override;

    friend class G4TCEventAction;

  protected:
    void InitHistos();
    
  public:
    // static variables for booking histograms consistently
    static G4int H1NHits; //< Geant uses histogram numbering, so create a static int to keep these consistent across multiple classes
    static G4int H1DeltaPhi; //< Create a static int to keep the histograms consistent
    static G4int H1IonEnergy; //< Create a static int to keep the histograms consistent
    static G4int H1dEdx;
    static G4int H1NRecoHits;
    static G4int H1Nelectrons;

    static G4int H2XYPos; //< Geant uses histogram numbering, so create a static int to keep these consistent across multiple classes
    
    static G4int NPDGCode;
    static G4int NTheta;
    static G4int NMomentum;
    static G4int NScatteringAngle;
    static G4int NEventPass;

    G4String outpath = "TEST.root";

  };  
}

#endif
