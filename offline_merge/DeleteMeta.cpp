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

bool isError(unsigned int word){
	return (word & 0xf0000000) == 0x60000000;
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

int check_EVID(char * filename){
	std::ifstream dataFlow;
  dataFlow.open(filename);
  bool status  = 1;
  int previous_EVID = -1;
  unsigned int word;
  status = dataFlow.read((char*)&word, sizeof(word));
  while(status){  	
  	if(isHeader(word)){
  		if (getHeaderEvtID(word) != (previous_EVID+1)%4096){
  			std::cout<<"EVID check failed"<<std::endl;
  			return 0;
  		}
  		else{
  			previous_EVID = getHeaderEvtID(word);
  		}
  	}
  	status = dataFlow.read((char*)&word, sizeof(word));
  }//while
  std::cout<<"EVID check succeeded"<<std::endl;
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
  sprintf(outfile_name,"%s",argv[1]);
  for(int i=0;;i++){
  	if(outfile_name[i]==0){
  		outfile_name[i-1]=0;
  		outfile_name[i-2]=0;
  		outfile_name[i-3]=0;
  		outfile_name[i-4]=0;
  		break;
  	}
  }
  sprintf(outfile_name,"%s_clean.dat",outfile_name);


  std::ifstream dataFlow1;
  dataFlow1.open(argv[1]);

  unsigned int word;
  unsigned int nLoop = 0;
  unsigned int maxWordCount = 100;

  dataFlow1.seekg(metadataSize, dataFlow1.beg);  
  fp_outfile = fopen(outfile_name, "wb");
  bool status  = 1;
  unsigned int nWords1 = 0;
  std::vector<unsigned int> mergedWords = std::vector<unsigned int>();

  // initialize status code and word values
  ReadFromStream(dataFlow1, &word,  status,  nWords1);
  int previous_EVID = -1;

  // read bulk
  while (status) {    
    if(isHeader(word) && (getHeaderEvtID(word) == (previous_EVID+1)%4096) && 
    	(getHeaderWordCount(word)<0x0F0) &&(word!=0x1000)) {
      // consecutive evid
      fwrite(&word, sizeof(word), 1, fp_outfile);
      previous_EVID = getHeaderEvtID(word);
      ReadFromStream(dataFlow1, &word,  status,  nWords1);      
    }
    else if (isEdge(word)) {
      // if file 1 shows an edge, write it and read next value
      fwrite(&word, sizeof(word), 1, fp_outfile);
      ReadFromStream(dataFlow1, &word,  status,  nWords1);
    }
    else if (isError(word)) {
    	//remove data until next header word
    	while(1){
    		ReadFromStream(dataFlow1, &word,  status,  nWords1);
    		if(isHeader(word))break;
    	}
    }
    else{
    	if((previous_EVID == 0) &&(word==0x1000)) {
    		unsigned int word_tmp;
    		ReadFromStream(dataFlow1, &word_tmp,  status,  nWords1);
    		if (isHeader(word_tmp) && ((getHeaderEvtID(word_tmp) == 2) || 
    			getHeaderWordCount(word_tmp)>0xF00)){
    			fwrite(&word, sizeof(word), 1, fp_outfile);
		      previous_EVID = getHeaderEvtID(word);
		      word = word_tmp;
		      continue;
    		}
    		else{
    			dataFlow1.seekg(-4, dataFlow1.cur);
    		}
    	}
	
      // non consecutive evid
      // std::cout<<"previous_EVID=0x"<<std::hex<<previous_EVID<<std::endl;
      dataFlow1.seekg(-16, dataFlow1.cur);
    	for(int i =0;i<15;i++){
    		ReadFromStream(dataFlow1, &word,  status,  nWords1);
    		// printf("current_word=%08X\n", word);
    	}
    	// dataFlow1.seekg(-44, dataFlow1.cur);

    	if(isHeader(word) && (getHeaderEvtID(word) == ((previous_EVID+1)%4096))){
    		//metadata found
    		fwrite(&word, sizeof(word), 1, fp_outfile);
	      previous_EVID = getHeaderEvtID(word);
	      ReadFromStream(dataFlow1, &word,  status,  nWords1); 
    	}
    	else{
    		dataFlow1.seekg(-44, dataFlow1.cur);
    		unsigned int word_tmp;
    		ReadFromStream(dataFlow1, &word_tmp,  status,  nWords1);
	      if (isEdge(word_tmp)){
	        //dropped events found
	        fwrite(&word, sizeof(word), 1, fp_outfile);
	        std::cout<<"Warning! Events dropped. previous_EVID=0x"<<std::hex
	        <<previous_EVID<<"	current_EVID=0x"<<getHeaderEvtID(word)<<std::endl;
	        previous_EVID = getHeaderEvtID(word);
	        word = word_tmp;
	        return 0;
    		}
    		else{
    			//error found
    			std::cout<<"Error! previous_EVID=0x"<<std::hex<<
    			previous_EVID<<"	current_EVID=0x"<<std::hex<<getHeaderEvtID(word)<<std::endl;
          return 0;
    		}
    	}//else metadata
    }//else non consecutive evid
  }//while

  
  fclose(fp_outfile);

  std::cout << std::endl << "File clean finished." << std::endl;

  check_EVID(outfile_name);

  return 0;
}

