#include "MuonReco/ArgParser.h"

namespace MuonReco {

  /*! \brief Constructor
   *
   *  Populates the underlying map data structure with key-value pairs
   *  Any command line key argument that is only two characters, ie -h,
   *  will be mapped to itself
   *  
   */
  ArgParser::ArgParser(int argc, char* argv[]) {
    for (int i = 1; i < argc; i+=2) {
      TString key = TString(argv[i]);
      if (key.Length() == 2) i--;
      TString val = TString(argv[i+1]);

      _map[key] = val;
    }
  }

  /*! \brief Destructor */
  ArgParser::~ArgParser() {}

  /*! Check for the existence of the key in the underlying map
   *
   *  \param key The key to check
   */
  bool ArgParser::hasKey(TString key) {
    for (auto it : _map) {
      if (!it.first.CompareTo(key)) return true;
    }
    return false;
  }


  /*! Cast TString value stored in key as a double using TString::Atof()
   *
   *  \param key The key, including any hyphens i.e. --conf
   */
  double ArgParser::getDouble(TString key) {
    if (this->hasKey(key)) {
      return _map[key].Atof();
    }
    else {
      std::cout << "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }

  /*! Cast TString value stored in key as an integer using TString::Atoi()
   *
   *  \param key The key, including any hyphens i.e. --conf
   */
  int ArgParser::getInt(TString key) {
    if (this->hasKey(key)) {
      return _map[key].Atoi();
    }
    else {
      std::cout<< "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }

  /*! Return value stored in key as TString
   *
   *  \param key The key, including any hyphens i.e. --conf
   */
  TString ArgParser::getTString(TString key) {
    if (this->hasKey(key)) {
      return _map[key];
    }
    else {
      std::cout<< "Key " << key << " not found in ArgParser" << std::endl;
      throw 1;
    }
  }

  /*! Cast TString value stored in key as a boolean
   *
   *  \param key The key, including any hyphens i.e. --conf
   */
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
