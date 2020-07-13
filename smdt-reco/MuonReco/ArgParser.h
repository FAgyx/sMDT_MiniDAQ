#ifndef MUON_ARGPARSER
#define MUON_ARGPARSER

#include "TString.h"

#include <iostream>
#include <map>

namespace MuonReco {
  class ArgParser {
  public:
    ArgParser(int argc, char* argv[]);

    
    bool    hasKey    (TString key);
    double  getDouble (TString key);
    int     getInt    (TString key);
    TString getTString(TString key);
    bool    getBool   (TString key);

  private:
    std::map<TString, TString> _map;
  };
}

#endif
