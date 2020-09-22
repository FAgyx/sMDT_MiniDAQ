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

bool isHeader(unsigned int word) {
  return isGroupHeader(word) || isTDCHeader(word);
}

bool isEdge(unsigned int word) {
  return isRisingEdge(word) || isFallingEdge(word);
}

int getHeaderEvtID(unsigned int word) {
  return (int)((0x00000fff) & (word >> 12));
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



void ReadFromStream(std::ifstream& dataFlow, unsigned int* word, bool& status, unsigned int& counter) {
  status = dataFlow.read((char*)word, sizeof(*word));
  counter++;
}

int main(int argc, char* argv[]) {
  int metadataSize = 44; // 11 4-byte words

  if (argc != 2) {
    std::cout << "Run this program with the paths to the .dat file for metadata deletion" 
	      << std::endl;
    return 1;
  }
  char outfile_name[256];
  FILE *fp_outfile;
  sprintf(outfile_name,"%s.clean",argv[1]);


  std::ifstream dataFlow1;
  dataFlow1.open(argv[1]);

  unsigned int word;
  unsigned int nLoop = 0;
  unsigned int maxWordCount = 100;

  dataFlow1.seekg(metadataSize, dataFlow1.beg);  
  FILE* fp_outfile = fopen("outfile_name", "wb");
  bool status  = 1;
  unsigned int nWords1 = 0;
  std::vector<unsigned int> mergedWords = std::vector<unsigned int>();

  // initialize status code and word values
  ReadFromStream(dataFlow1, &word,  status,  nWords1);
  int previous_BCID = -1;

  // read bulk
  while (status) {    
    if (isHeader(word) && getHeaderEvtID(word) == (previous_BCID+1)%4096) {
      // consecutive evid
      // std::cout << "EvtID1=" << getHeaderEvtID(word) << " EvtID2=" << getHeaderEvtID(word2) << std::endl;
      fwrite(&word, sizeof(word), 1, fp_outfile);
      ReadFromStream(dataFlow1, &word,  status,  nWords1);
      previous_BCID = getHeaderEvtID(word);
      continue;
    }
    else if (isEdge(word)) {
      // if file 1 shows an edge, write it and read next value
      fwrite(&word, sizeof(word), 1, fp_outfile);
      ReadFromStream(dataFlow1, &word,  status,  nWords1);
      continue;
    }
    else{
      //non consecutive evid
      int index;
      unsigned int word_tmp;
      std::ifstream::pos_type p = dataFlow1.tellp();  //get the current position
      ReadFromStream(dataFlow1, &word_tmp,  status,  nWords1);
      if (isEdge(word_tmp){
        //the following word is an edge word, indicating events dropped
        fwrite(&word, sizeof(word), 1, fp_outfile);
        previous_BCID = getHeaderEvtID(word);
        dataFlow1.seekg(-1, p);
      } 
      else{
        //the following word is not an edge word, indicating metadata
        dataFlow1.seekg(10, p);
        ReadFromStream(dataFlow1, &word,  status,  nWords1);
        if (isHeader(word) && getHeaderEvtID(word) != (previous_BCID+1)%4096) {
          std::cout<<"error!"<<std::endl;
        }
      }
    }
  }//while

  
  fclose(fp_outfile);

  std::cout << std::endl << "Merged File words: " << std::endl;

  return 0;
}

