/*
  Code to merge two .dat files from two CSM boards offline
  
  For more information about the encoding of data words from the HPTDC
  visit:
  https://cds.cern.ch/record/1067476/files/cer-002723234.pdf
  
  Author: Kevin Nelson
  Date:   September 8th, 2020
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

unsigned int setCSMBit(unsigned int word) {
  return (word | 0x80000000);
}

unsigned int addHeaderOffset(unsigned int word, unsigned int offset) {
  return word + (offset << 12);
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
    return "Unkown word";
  }
}

void ReadFromStream(std::ifstream& dataFlow, unsigned int* word, bool& status) {
  dataFlow.read((char*)word, sizeof(*word));
  if (dataFlow) status = 1;
  else    status = 0;
}

int main(int argc, char* argv[]) {
  int metadataSize = 44; // 11 4-byte words
  //int metadataSize = 0; // no metadata for clean file
  int CSM2offset = 8;
  if (argc != 3) {
    std::cout << "Run this program with exactly two arguments (the paths to the two .dat files to merge)" 
	      << std::endl;
    return 1;
  }

  // print out first 100 words from both files

  std::vector<unsigned int> file1Words = std::vector<unsigned int>();
  std::vector<unsigned int> file2Words = std::vector<unsigned int>();

  std::ifstream dataFlow1;
  dataFlow1.open(argv[1]);
  dataFlow1.seekg(0, dataFlow1.beg);

  std::ifstream dataFlow2;
  dataFlow2.open(argv[2]);
  dataFlow2.seekg(0, dataFlow2.beg); // ignore metadata

  unsigned int word = 0, word2 = 0;
  unsigned int nLoop = 0;
  unsigned int maxWordCount = 1000;

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
  
  // do file merging, ignoring metadata
  dataFlow1.seekg(0, dataFlow1.beg);
  dataFlow2.seekg(0, dataFlow2.beg);

  std::cout << "Starting position " << dataFlow1.tellg() << std::endl;
  
  FILE* MergedDataFile = fopen("merged.dat", "wb");
  bool status  = 1;
  bool status2 = 1;
  unsigned int nWords  = 0;
  unsigned int bunch1  = 0;
  unsigned int bunch2  = 0;
  std::vector<unsigned int> mergedWords = std::vector<unsigned int>();

  // initialize status code and word values
  //ReadFromStream(dataFlow1,&word,  status);
  //ReadFromStream(dataFlow2,&word2, status2);


  // read bulk
  while (status && status2) {    
    if (isHeader(word) && isHeader(word2) && 
	getHeaderEvtID(word, bunch1) == getHeaderEvtID(word2,bunch2)) {

      // if both words are identical headers, only write once
      // but also need to add together the word count from both headers
      word = (word & 0xfffff000) | ((word & 0x00000fff) + (word2 & 0x00000fff));

      if ((word  & 0x00fff000) == 0x00fff000) ++bunch1;
      if ((word2 & 0x00fff000) == 0x00fff000) ++bunch2;

      fwrite(&word, sizeof(word), 1, MergedDataFile);
      if (mergedWords.size() < maxWordCount) {mergedWords.push_back(word);}

      ReadFromStream(dataFlow1, &word,  status);
      ReadFromStream(dataFlow2, &word2, status2);
      continue;
    }
    else if (isEdge(word)) {
      // if file 1 shows an edge, write it and read next value
      fwrite(&word, sizeof(word), 1, MergedDataFile);
      // printf("word1=%08X\n", word);
      if (mergedWords.size() < maxWordCount) {mergedWords.push_back(word);}

      ReadFromStream(dataFlow1, &word,  status);
      continue;
    }
    else if (isEdge(word2)) {
      // if file 2 shows an edge, write it and read next value
      // printf("word2=%08X\n", word2);
      word2 = setCSMBit(word2);
      fwrite(&word2, sizeof(word2), 1, MergedDataFile);
      if (mergedWords.size() < maxWordCount) mergedWords.push_back(word2);

      ReadFromStream(dataFlow2,&word2, status2);
      word2 = setCSMBit(word2);
      continue;
    }
    else {
      if (getHeaderEvtID(word, bunch1) < getHeaderEvtID(word2, bunch2)) {
	if ((word  & 0x00fff000) == 0x00fff000) ++bunch1;

	fwrite(&word, sizeof(word), 1, MergedDataFile);
	if (mergedWords.size() < maxWordCount) mergedWords.push_back(word);
	
	ReadFromStream(dataFlow1, &word,  status);
	continue;
      }
      else {
	if ((word2 & 0x00fff000) == 0x00fff000) ++bunch2;

	word2 = setCSMBit(word2);
	fwrite(&word2, sizeof(word2), 1, MergedDataFile);
	if (mergedWords.size() < maxWordCount) mergedWords.push_back(word2);

	ReadFromStream(dataFlow2,&word2, status2);
	word2 = setCSMBit(word2);
	continue;
      }
    }
  }

  // clean up trailing words
  while (status) {
    fwrite(&word, sizeof(word), 1, MergedDataFile);
    ReadFromStream(dataFlow1,&word, status);
  }
  while (status2) {
    word2 = setCSMBit(word2);
    fwrite(&word2, sizeof(word2), 1, MergedDataFile);
    ReadFromStream(dataFlow2,&word2, status2);
  }
  
  fclose(MergedDataFile);

  std::cout << std::endl << "Merged File words: " << std::endl;
  for (int i = 0; i < mergedWords.size(); i++) {
    char output[9];
    sprintf(output, "%08x", mergedWords.at(i));
    std::cout << output << ": " << wordToString(mergedWords.at(i)) << std::endl;
  }

  return 0;
}
