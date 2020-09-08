/*
  Code to merge two .dat files from two CSM boards offline
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

//
// Useful functions for datawords
//

bool isGroupHeader(unsigned int word) {
  return (word & 0xf0000000) == 0x00000000;
}

bool isGroupTrailer(unsigned int word) {
  return (word & 0xf0000000) == 0x10000000;
}

bool isTDCHeader(unsigned int word) {
  return (word & 0xf0000000) == 0x20000000;
}

bool isTDCTrailer(unsigned int word) {
  return (word & 0xf0000000) == 0x30000000;
}

bool isRisingEdge(unsigned int word) {
  return (word & 0xf0000000) == 0x40000000;
}

bool isFallingEdge(unsigned int word) {
  return (word & 0xf0000000) == 0x50000000;
}

int getHeaderEvtID(unsigned int word) {
  return (int)((0x00000fff) & (word >> 12));
}

int getWordTDC(unsigned int word) {
  return (int)((0x0000000f) & (word >> 24));
}

int getEdgeChannel(unsigned int word) {
  return (int)((0x0000001f) & (word >> 19));
}

std::string wordToString(unsigned int word) {
  if (isGroupHeader(word)) {
    std::ostringstream ss;
    ss << "Group Header for event " << getHeaderEvtID(word);
    return ss.str();
  }
  else if (isGroupTrailer(word)) {
    std::ostringstream ss;
    ss << "Group Trailer for event " << getHeaderEvtID(word);
    return ss.str();
  }
  else if (isTDCHeader(word)) {
    std::ostringstream ss;
    ss << "TDC Header for event " << getHeaderEvtID(word);
    return ss.str();
  }
  else if (isTDCTrailer(word)) {
    std::ostringstream ss;
    ss << "TDC Trailer for event " << getHeaderEvtID(word);
    return ss.str();
  }
  else if (isRisingEdge(word)) {
    std::ostringstream ss;
    ss << "Rising edge on TDC " << getEdgeChannel(word);
    return ss.str();
  }
  else if (isFallingEdge(word)) {
    std::ostringstream ss;
    ss << "Falling edge on TDC " << getEdgeChannel(word);
    return ss.str();
  }
  else {
    return "Unkown word";
  }
}

int main(int argc, char* argv[]) {

  if (argc != 3) {
    std::cout << "Run this program with exactly two arguments (the paths to the two .dat files to merge)" 
	      << std::endl;
    return 1;
  }

  std::vector<unsigned int> file1Words = std::vector<unsigned int>();
  std::vector<unsigned int> file2Words = std::vector<unsigned int>();

  std::ifstream dataFlow1;
  dataFlow1.open(argv[1]);
  dataFlow1.seekg(0, dataFlow1.beg);

  std::ifstream dataFlow2;
  dataFlow2.open(argv[2]);
  dataFlow2.seekg(0, dataFlow2.beg);

  unsigned int word;
  unsigned int nLoop = 0;
  unsigned int maxWordCount = 100;

  while (dataFlow1.read((char*)&word, sizeof(word)) && nLoop < maxWordCount) {
    file1Words.push_back(word);
    nLoop++;
  }

  nLoop = 0;
  while (dataFlow2.read((char*)&word, sizeof(word)) && nLoop < maxWordCount) {
    file2Words.push_back(word);
    nLoop++;
  }

  std::cout << "File 1 words: " << std::endl;
  for (int i = 0; i < file1Words.size(); i++) {
    char output[9];
    sprintf(output, "%08x", file1Words.at(i));
    std::cout << output << ": " << wordToString(file1Words.at(i)) << std::endl;
  }
  std::cout << std::endl;
  std::cout << "File 2 words: " << std::endl;
  for (int i = 0; i < file2Words.size(); i++) {
    char output[9];
    sprintf(output, "%08x", file2Words.at(i));
    std::cout << output << ": " << wordToString(file2Words.at(i)) << std::endl;
  }
  
  return 0;
}
