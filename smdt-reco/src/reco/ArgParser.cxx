#include "MuonReco/ArgParser.h"

namespace MuonReco {
  ArgParser::ArgParser(int argc, char* argv[]) {
    for (int i = 1; i < argc; i+=2) {
      TString key = TString(argv[i]);
      TString val = TString(argv[i+1]);

      _map[key] = val;
    }
  }

  bool ArgParser::hasKey(TString key) {
    for (auto it : _map) {
      if (!it.first.CompareTo(key)) return true;
    }
    return false;
  }

  double ArgParser::getDouble(TString key) {
    if (this->hasKey(key)) {
      return _map[key].Atof();
    }
    else {
      std::cout << "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }

  int ArgParser::getInt(TString key) {
    if (this->hasKey(key)) {
      return _map[key].Atoi();
    }
    else {
      std::cout<< "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }

  TString ArgParser::getTString(TString key) {
    if (this->hasKey(key)) {
      return _map[key];
    }
    else {
      std::cout<< "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }

  bool ArgParser::getBool(TString key) {
    if (this->hasKey(key)) {
      return !!(_map[key].Atoi());
    }
    else {
      std::cout<< "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }
}
