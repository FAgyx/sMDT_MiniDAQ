#include <iostream>

#include "TString.h"
#include "TTree.h"
#include "TFile.h"

#include "MuonReco/EventDisplay.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/Event.h"

using namespace MuonReco;

//int RunEventDisplay() {
int main(int argc, char* argv[]) {

  TString filename = "run00187691_20190301.dat";
  
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
  
  // initialize the event display
  EventDisplay ed = EventDisplay();

  // while user has not exited, keep showing next event
  TString user_input = "c";
  int counter = 0;
  char buffer[256];
  int  bufflen = 256;

  while (user_input.CompareTo(TString("q")) && counter < eTree->GetEntries()) {
    eTree->GetEntry(counter);
    ed.DrawEvent(*e, geo, NULL);

    // get user input
    std::cout << "To continue, enter any key." 
	      << "To exit,     enter q" << std::endl;
    std::cin.getline(buffer, bufflen);
    user_input = TString(buffer);
    ed.Clear();
    counter++;
  }
  std::cout << "done" << std::endl;
  return 0;
}
