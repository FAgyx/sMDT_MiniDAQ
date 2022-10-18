#include <stdio.h>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <bitset>
#include <string>
#include <sstream>

#include "MuonReco/Signal.h"
#include "MuonReco/EventID.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"
#include "MuonReco/IOUtility.h"

#include "TGraph.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TCanvas.h"

using namespace MuonReco;
using namespace std;

bool isGroupHeader(unsigned int word) {
  return (word & 0x70000000) == 0x00000000;
}

bool isGroupTrailer(unsigned int word) {
  return (word & 0x70000000) == 0x10000000;
}

bool isTDCHeader(unsigned int word) {
  return (word & 0x70000000) == 0x20000000;
}

bool isTDCTrailer(unsigned int word) {
  return (word & 0x70000000) == 0x30000000;
}

bool isRisingEdge(unsigned int word) {
  return (word & 0x70000000) == 0x40000000;
}

bool isFallingEdge(unsigned int word) {
  return (word & 0x70000000) == 0x50000000;
}

bool isHeader(unsigned int word) {
  return isGroupHeader(word) || isTDCHeader(word);
}

bool isEdge(unsigned int word) {
  return isRisingEdge(word) || isFallingEdge(word);
}

unsigned int getHeaderEvtID(unsigned int word, unsigned int bunchid) {
  return ((0x00000fff) & (word >> 12)) + 4096*bunchid;
}

int getHeaderWordCount(unsigned int word) {
  return (int)(0x00000fff & word);
}

int getWordTDC(unsigned int word) {
  return (int)((0x0000000f) & (word >> 24));
}

int getEdgeChannel(unsigned int word) {
  return (int)((0x0000001f) & (word >> 19));
}

int getEdgeTDC(unsigned int word) {
  return (int)((0x0000000f) & (word >> 24));
}

void printSignal(Signal s) {
  if (s.Type() == Signal::RISING) 
    std::cout << "Rising  edge ";
  else if (s.Type() == Signal::FALLING) 
    std::cout << "Falling edge ";
  else 
    std::cout << "Type " << s.Type() << "       ";
  
  std::cout << " on tdc " << s.TDC() << " chan " << s.Channel() << std::endl;
}

std::string wordToString(unsigned int word) {
  if (isGroupHeader(word)) {
    std::ostringstream ss;
    ss << "Group Header for event " << getHeaderEvtID(word, 0);
    return ss.str();
  }
  else if (isGroupTrailer(word)) {
    std::ostringstream ss;
    ss << "Group Trailer for event " << getHeaderEvtID(word, 0);
    return ss.str();
  }
  else if (isTDCHeader(word)) {
    std::ostringstream ss;
    ss << "TDC Header for event " << getHeaderEvtID(word, 0);
    return ss.str();
  }
  else if (isTDCTrailer(word)) {
    std::ostringstream ss;
    ss << "TDC Trailer for event " << getHeaderEvtID(word, 0);
    return ss.str();
  }
  else if (isRisingEdge(word)) {
    std::ostringstream ss;
    ss << "  Rising edge on TDC " << getEdgeTDC(word) << " channel " << getEdgeChannel(word);
    return ss.str();
  }
  else if (isFallingEdge(word)) {
    std::ostringstream ss;
    ss << "  Falling edge on TDC " << getEdgeTDC(word) << " channel " << getEdgeChannel(word);
    return ss.str();
  }
  else {
    return "Unknown word";
  }
}


int main(int argc, char* argv[]) {
  ArgParser    ap = ArgParser(argc, argv);
  ConfigParser cp = ConfigParser(ap.getTString("--conf"));
  int runN        = cp.items("General").getInt("RunNumber");

  TString filename = cp.items("General").getStr("RawFileName");


  unsigned int header_type;
  bitset<4> header;
  Signal sig;

  EventID currEventID;
  EventID prevEventID = EventID(0x00000000);
  vector<Signal>  sigVec = vector<Signal>();

  unsigned int bunch_id;
  unsigned int word;
  unsigned int prevWord = 0;
  int total_edges_CSM1 = 0;
  int total_edges_CSM2 = 0;
  int total_edges = 0;
  ifstream data_in_flow;
  data_in_flow.open(IOUtility::getRawInputFilePath(filename).Data());

  data_in_flow.seekg(0, data_in_flow.end);
  unsigned int data_in_flow_length = data_in_flow.tellg();
  data_in_flow.seekg(0, data_in_flow.beg);
  
  int userchar = 0;
  int nCSM1 = 0;
  int nCSM2 = 0;
  int nTrig = 0;
  bool hasCSM1r = 0;
  bool hasCSM2l = 0;
  unsigned int nsigs = 0;
  unsigned int counter = 0;
  char output[9];

  std::vector<int> total_trace = std::vector<int>();
  std::vector<int> csm1_trace  = std::vector<int>();
  std::vector<int> csm2_trace  = std::vector<int>();

  while (data_in_flow.read((char *) &word, sizeof(word))) {

    header = (word & 0x7fffffff) >> 28; // get the four bits header of this word
    header_type = static_cast<unsigned int>((header.to_ulong()));
    counter++;
    
    //std::cout << counter << ": " << wordToString(word) << std::endl;
    /*
    if (counter % 100 == 0) {
      std::cout << std::endl << std::endl;
      std::cout << "Press q to quit, any other key to continue" << std::endl << std::endl;
      userchar = getchar();
      if (userchar == 'q') break;
    }
    */

    if (header_type == Signal::GROUP_HEADER || header_type == Signal::TDC_HEADER || header_type == Signal::TDC_TRAILER) {
      currEventID = EventID((word & 0xfffff000) + bunch_id);
      if ((word & 0x00fff000) == 0x00fff000) bunch_id++;

      if (currEventID.ID() != prevEventID.ID()) {
	prevEventID = currEventID;
	// header for new event

	nsigs = (prevWord & 0x00000fff);
	
	if (nsigs > 1000) {
	  std::cout << std::hex << prevWord << std::endl;
	  std::cout << "counter: " << std::dec << counter << std::endl;
	}
	
	if (nsigs != sigVec.size()) std::cout << "Event " << currEventID.ID() << " header says " << nsigs << " but " << sigVec.size() << " were found " << std::endl;

	prevEventID = currEventID;
	prevWord    = word;

	
	nCSM1 = 0;
	nCSM2 = 0;
	nTrig = 0;
	hasCSM1r = 0;
	hasCSM2l = 0;
	for (Signal sig : sigVec) {
	  if (sig.TDC() <= 11) ++nCSM1;
	  if (sig.TDC() >= 18) ++nCSM2;
	  if (sig.TDC() == 14 && sig.Channel() == 23) ++nTrig;
	  if ((sig.TDC() == 24 || sig.TDC() == 18) && (sig.Channel() == 0 || sig.Channel() == 1 || sig.Channel() == 2 || sig.Channel() == 3)) hasCSM2l = true;
	  if ((sig.TDC() == 5 || sig.TDC() == 10) && (sig.Channel() == 20 || sig.Channel() == 21 || sig.Channel() == 22 || sig.Channel() == 23)) hasCSM1r = true;
	}

	/*
	std::cout << "Event " << prevEventID.ID() << " has " << nCSM1 << " on CSM 1" << std::endl;
	std::cout << "Event " << prevEventID.ID() << " has " << nCSM2 << " on CSM 2" << std::endl;
	std::cout << "Event " << prevEventID.ID() << " has " << nTrig << " on trigger" << std::endl;
	std::cout << std::endl;
	
	//std::cout << "Event " << prevEventID.ID() << std::endl;
	for (Signal sig : sigVec) {
	  printSignal(sig);
	}
	*/
	sigVec.clear();
	
	/*
	if ((prevEventID.ID()+1) % 100 == 0) {
	  std::cout << std::endl << std::endl;
	  std::cout << "Press q to quit, any other key to continue" << std::endl << std::endl;
	  userchar = getchar();
	  if (userchar == 'q') break;
	  }*/
	
      }
    } // end if: group/tdc header/trailer
    else if (header_type == Signal::RISING || header_type == Signal::FALLING) {
      sig = Signal(word, currEventID);
      sigVec.push_back(sig);
      if (sig.TDC() <= 11) ++total_edges_CSM1; 
      if (sig.TDC() >= 18) ++total_edges_CSM2;
      if (sig.TDC() <= 11 || sig.TDC() >= 18) ++total_edges;
    }
    if (total_edges% 100 == 0 && total_edges<100000) {
      total_trace.push_back(total_edges);
      csm1_trace .push_back(total_edges_CSM1);
      csm2_trace .push_back(total_edges_CSM2);
    }

  } // end while: read in from file
  std::cout << "total edges CSM 1: " << total_edges_CSM1 << std::endl;
  std::cout << "total edges CSM 2: " << total_edges_CSM2 << std::endl;
  
  TCanvas* c1 = new TCanvas("c1", "", 600, 800);
  TGraph* gr1 = new TGraph(csm1_trace.size(), &total_trace[0], &csm1_trace[0]);
  TGraph* gr2 = new TGraph(csm2_trace.size(), &total_trace[0], &csm2_trace[0]);
  gr1->GetXaxis()->SetTitle("Total edges (excluding trigger mezz)");
  gr1->GetYaxis()->SetTitle("CSM edges");
  gr1->SetLineColor(kBlue);
  gr2->SetLineColor(kRed);
  auto leg = new TLegend(0.6,0.1,0.9,0.3);
  leg->AddEntry(gr1, "CSM 1");
  leg->AddEntry(gr2, "CSM 2");
  gr1->Draw();
  gr2->Draw("same");
  leg->Draw();
  c1->Print("cumulative.png");
}
