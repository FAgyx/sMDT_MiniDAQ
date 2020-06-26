
#include "src/EventDisplay.cpp"
#include "src/Geometry.cpp"
#include "src/Event.cpp"


void RunEventDisplay(TString filename = "run00187691_20190301.dat") {

  using namespace Muon;
  
  // open input file
  chdir("output");
  chdir(TString(filename + ".dir").Data());
  TFile *inputFile = new TFile(TString(filename + ".out.root").Data(), "r");
  
  TTree *eTree = (TTree*)inputFile->Get("eTree");
  Event *e = new Event();
  eTree->SetBranchAddress("event", &e);
  
  
  // set the geometry
  int runN = ((TObjString*)(TString(filename(3,256)).Tokenize("_")->At(0)))->String().Atoi();
  Geometry geo = Geometry();
  geo.SetRunN(runN);
  
  // while user has not exited, keep showing next event
  TString user_input = "c";
  int counter = 0;
  char buffer[256];
  int  bufflen = 256;
  cout << "here" << endl;
  cout << eTree->GetEntries() << endl;
  cout << user_input.CompareTo(TString("q")) << endl;
  while (user_input.CompareTo(TString("q")) && counter < eTree->GetEntries()) {
    eTree->GetEntry(counter);
    EventDisplay::DrawEvent(*e, geo, NULL);
    
    // get user input
    cout << "To continue, enter any key." 
	 << "To exit,     enter q" << endl;
    cin.getline(buffer, bufflen);
    user_input = TString(buffer);
    EventDisplay::Clear();
    counter++;
  }
  cout << "done" << endl;
}
