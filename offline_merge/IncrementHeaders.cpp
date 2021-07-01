#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

bool isGroupHeader(unsigned int word) {
  return (word & 0x70000000) == 0x00000000;
}

bool isRising(unsigned int word) {
  return (word & 0x70000000) == 0x40000000;
}

bool isFalling(unsigned int word) {
  return (word & 0x70000000) == 0x50000000;
}

bool isNextHeader(unsigned int thisHeader, unsigned int prevHeader) {
  unsigned int thisID = (thisHeader & 0x00fff000) >> 12;
  unsigned int prevID = (prevHeader & 0x00fff000) >> 12;

  return ((prevID+1) % 4096) == thisID;
}

int main(int argc, char* argv[]) {

  if (argc != 2) {
    std::cout << "Run this program with exactly one argument: data file"
              << std::endl;
    return 1;
  }

  bool status = 1;
  unsigned int word = 0;
  unsigned long counter = 0;
  unsigned int prevheader = 0x00fff000;

  std::ifstream dataFlow;
  dataFlow.open(argv[1]);
  dataFlow.seekg(44, dataFlow.beg);

  std::cout << isNextHeader(0x00001000, 0x00000000) << std::endl;
  std::cout << isNextHeader(0x00fff000, 0x00ffe000) << std::endl;
  std::cout << isNextHeader(0x00000000, 0x00fff000) << std::endl;
  std::cout << isNextHeader(0x003ce000, 0x003ce000) << std::endl;
  std::cout << isNextHeader(0x00444000, 0x00443000) << std::endl;

  FILE* MergedDataFile = fopen("incremented.dat", "wb");
  
  while (status) {
    dataFlow.read((char*)(&word), sizeof(word));
    if (dataFlow) status = 1;
    else status = 0;

    //if (isGroupHeader(word)) word = (word & 0xff000fff) | ((word + 0x00008000) & 0x00fff000);
    
    if (!(isGroupHeader(word) || isRising(word) || isFalling(word))) {}
    else if (isGroupHeader(word) && !isNextHeader(word, prevheader)) {}
    else {
      fwrite(&word, sizeof(word), 1, MergedDataFile);
      if (isGroupHeader(word)) prevheader = word;
    }

    ++counter;
  }

  fclose(MergedDataFile);
}
