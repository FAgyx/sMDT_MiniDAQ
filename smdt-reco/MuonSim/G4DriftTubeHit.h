#ifndef MUON_DriftTubeHit
#define MUON_DriftTubeHit

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4AttDefStore.hh"
#include "G4AttDef.hh"
#include "G4AttValue.hh"
#include "G4UIcommand.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"


namespace MuonSim {
  /*! \class G4DriftTubeHit G4DriftTubeHit.h "MuonSim/G4DriftTubeHit.h"
   *  
   *  Class to store MC truth information from hits on the G4DriftTubeSD
   *  class.  Inherits from G4VHit.
   *
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   17 July 2020
   */
  class G4DriftTubeHit : public G4VHit {
  public:
    G4DriftTubeHit();
    G4DriftTubeHit(G4int l, G4int c);
    G4DriftTubeHit(const G4DriftTubeHit &right);
    virtual ~G4DriftTubeHit();
    
    const G4DriftTubeHit& operator=(const G4DriftTubeHit &right);
    int operator==(const G4DriftTubeHit &right) const;

    inline void *operator new(size_t);
    inline void operator delete(void *aHit);

    virtual void Draw() override;
    virtual const std::map<G4String, G4AttDef>* GetAttDefs() const override;
    virtual std::vector<G4AttValue>* CreateAttValues() const override;
    virtual void Print() override;

    void SetLayer(G4int l) { fLayer = l; }
    G4int GetLayer() const { return fLayer; }

    void SetColumn(G4int c) { fColumn = c; }
    G4int GetColumn() const { return fColumn; }

    void SetTime(G4double t) { fTime = t; }
    G4double GetTime() const { return fTime; }

    void SetWorldPos(G4ThreeVector xyz) { fWorldPos = xyz; }
    G4ThreeVector GetWorldPos() const { return fWorldPos; }

    void SetLocalPos(G4ThreeVector xyz) { fLocalPos = xyz; }
    G4ThreeVector GetLocalPos() const { return fLocalPos; }
    
    void SetDirection(G4ThreeVector dir) { fDirection = dir; }
    G4ThreeVector GetDirection() const { return fDirection; }

    void SetIonizationEnergy(G4double e) { fIonEnergy = e; }
    G4double GetIonizationEnergy() const { return fIonEnergy; }

    void SetStepLength(G4double l) { fStepLength = l; }
    G4double GetStepLength() const { return fStepLength; }
    
  private:
    G4int fLayer;
    G4int fColumn;
    G4double fTime;
    G4ThreeVector fLocalPos;
    G4ThreeVector fWorldPos;
    G4ThreeVector fDirection;
    G4double fIonEnergy;
    G4double fStepLength;
  };

  typedef G4THitsCollection<G4DriftTubeHit> G4DriftTubeHitsCollection;
  
  extern G4ThreadLocal G4Allocator<G4DriftTubeHit>* G4DriftTubeHitAllocator;

  inline void* G4DriftTubeHit::operator new(size_t) {
    if (!G4DriftTubeHitAllocator)
      G4DriftTubeHitAllocator = new G4Allocator<G4DriftTubeHit>;
    return (void*) G4DriftTubeHitAllocator->MallocSingle();
  }

  inline void G4DriftTubeHit::operator delete(void* aHit) {
    G4DriftTubeHitAllocator->FreeSingle((G4DriftTubeHit*) aHit);
  }
}

#endif  
