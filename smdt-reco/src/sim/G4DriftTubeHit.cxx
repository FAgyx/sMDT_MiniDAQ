#include "MuonSim/G4DriftTubeHit.h"

namespace MuonSim {
  G4ThreadLocal G4Allocator<G4DriftTubeHit>* G4DriftTubeHitAllocator;

  G4DriftTubeHit::G4DriftTubeHit() : G4VHit(), fLayer(-1), fColumn(-1), fTime(0.) {}

  G4DriftTubeHit::G4DriftTubeHit(const G4DriftTubeHit &right) : G4VHit() {
    fLayer    = right.fLayer;
    fColumn   = right.fColumn;
    fLocalPos = right.fLocalPos;
    fWorldPos = right.fWorldPos;
    fTime     = right.fTime;
  }

  G4DriftTubeHit::G4DriftTubeHit(G4int l, G4int c) : G4VHit(), fLayer(l), fColumn(c), fTime(0.), fLocalPos(0), fWorldPos(0) {}

  G4DriftTubeHit::~G4DriftTubeHit() {}

  const G4DriftTubeHit& G4DriftTubeHit::operator=(const G4DriftTubeHit & right) {
    fLayer    = right.fLayer;
    fColumn   = right.fColumn;
    fLocalPos = right.fLocalPos;
    fWorldPos = right.fWorldPos;
    fTime     = right.fTime;
    return *this;
  }

  int G4DriftTubeHit::operator==(const G4DriftTubeHit &/*right*/) const {
    return 0;
  }

  void G4DriftTubeHit::Draw() {
    G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance(); 
    if(pVVisManager) {
      G4Circle circle(fWorldPos);
      circle.SetScreenSize(4);
      circle.SetFillStyle(G4Circle::filled);
      G4Color color(0.,0.,1);
      G4VisAttributes attribs(color);
      circle.SetVisAttributes(attribs);
      pVVisManager->Draw(circle);
    }
  }

  const std::map<G4String,G4AttDef>* G4DriftTubeHit::GetAttDefs() const {
    G4bool isNew;
    std::map<G4String,G4AttDef>* store = G4AttDefStore::GetInstance("G4DriftTubeHit",isNew);
    
    if (isNew) {
      (*store)["HitType"] = G4AttDef("HitType", "Hit Type", "Physics", "",           "G4String");
      (*store)["Layer"]   = G4AttDef("Layer",   "Layer",    "Physics", "",           "G4int");
      (*store)["Column"]  = G4AttDef("Column",  "Column",   "Physics", "",           "G4int");
      (*store)["Time"]    = G4AttDef("Time",    "Time",     "Physics", "G4BestUnit", "G4double");
      (*store)["Pos"]     = G4AttDef("Pos",     "Position", "Physics", "G4BestUnit", "G4ThreeVector");
    }
    return store;
  }

  std::vector<G4AttValue>* G4DriftTubeHit::CreateAttValues() const {
    std::vector<G4AttValue>* values = new std::vector<G4AttValue>;

    values->push_back(G4AttValue("HitType","DriftTubeHit",""));
    values->push_back(G4AttValue("Layer",G4UIcommand::ConvertToString(fLayer),""));
    values->push_back(G4AttValue("Column",G4UIcommand::ConvertToString(fColumn),""));
    values->push_back(G4AttValue("Time",G4BestUnit(fTime,"Time"),""));
    values->push_back(G4AttValue("Pos",G4BestUnit(fWorldPos,"Length"),""));

    return values;
  }

  void G4DriftTubeHit::Print() {
    G4cout << "sMDT Hit: " << "Layer=" << fLayer 
	   << " Column=" << fColumn
	   << " Time=" << fTime/CLHEP::ns
	   << " local (x,y) " << fLocalPos.x() << ", " << fLocalPos.y() << G4endl;
  }

}
