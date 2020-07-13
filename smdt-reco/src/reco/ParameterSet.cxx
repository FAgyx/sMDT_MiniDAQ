#include "MuonReco/ParameterSet.h"


namespace MuonReco {

  ParameterSet::ParameterSet() {
    myMap = std::map<TString, TString>();
  }

  std::vector<TString> ParameterSet::keys() {
    std::vector<TString> myKeys = std::vector<TString>();
    for (auto it = myMap.begin(); it != myMap.end(); it++) {
      myKeys.push_back(it->first);
    }
    return myKeys;
  }

  Bool_t ParameterSet::hasKey(TString key) {
    return myMap.find(key) != myMap.end();
  }

  void ParameterSet::add(TString key, TString value) {
    auto it = myMap.find(key);
    if (it != myMap.end()) {
      std::cout << "Attempting to add key " << key 
		<< " that is already in ParameterSet.  Value not changed." << std::endl;
    }
    else {
      myMap[key] = value;
    }
  }

  void ParameterSet::Print() {
    auto it = myMap.begin();
    while (it != myMap.end()) {
      std::cout << it->first << ": " << it->second << std::endl;
      it++;
    }
    std::cout << std::endl;
  }

  double ParameterSet::Size() {
    return myMap.size();
  }

  TString ParameterSet::getStr(TString key, int index /*=-1*/) {
    if (!hasKey(key)) {
      std::cout << "Error, attempting to access key " << key << " that is not in ParameterSet" << std::endl;
    }
    
    if (index == -1) return myMap[key];
    else if (myMap[key].Tokenize(":")->GetEntries() > index)
      return ((TObjString*)(myMap[key].Tokenize(":")->At(index)))->GetString().Strip(TString::kBoth, ' ');
    else 
      return "";
  }

  TString ParameterSet::getStr(TString key, TString defaultVal, int index /*=-1*/) {
    if (!hasKey(key)) return defaultVal;

    try {
      return getStr(key, index);
    }
    catch (...) {
      return defaultVal;
    }
  }

  int ParameterSet::getInt(TString key, int index /*=-1*/) {
    if (index == -1) return myMap[key].Atoi();
    else return ((TObjString*)(myMap[key].Tokenize(":")->At(index)))->GetString().Strip(TString::kBoth, ' ').String().Atoi();
  }

  int ParameterSet::getInt(TString key, int defaultVal, int index) {
    if (!hasKey(key)) return defaultVal;

    try {
      return getInt(key, index);
    }
    catch (...) {
      return defaultVal;
    } 
  }
  
  double ParameterSet::getDouble(TString key, int index /*=-1*/) {
    if (index == -1) return myMap[key].Atof();
    else if (myMap[key].Tokenize(":")->GetEntries() > index)
      return ((TObjString*)(myMap[key].Tokenize(":")->At(index)))->GetString().Strip(TString::kBoth, ' ').String().Atof();
    else 
      throw 1;
  }

  double ParameterSet::getDouble(TString key, double defaultVal, int index /*=-1*/) {
    if (!hasKey(key)) return defaultVal;

    try {
      return getDouble(key, index);
    }
    catch (...) {
      return defaultVal;
    }
  }

  Bool_t ParameterSet::getBool(TString key, int index /*=-1*/) {
    TString value = ((TObjString*)(myMap[key].Tokenize(":")->At(index)))->GetString().Strip(TString::kBoth, ' ').String();
    return !(value.CompareTo("True")) || !(value.CompareTo("true"))
      || !(value.CompareTo("kTRUE")) || !(value.CompareTo("1"));
  }

  Bool_t ParameterSet::getBool(TString key, Bool_t defaultVal, int index) {
    if (!hasKey(key)) return defaultVal;

    try { 
      return getBool(key, index);
    }
    catch (...) {
      return defaultVal;
    }
  }

  std::vector<TString> ParameterSet::getStrVector(TString key)
  {
    if (!hasKey(key)) {
      std::cout << "Error, attempting to access key " << key << " that is not in ParameterSet" << std::endl;
      throw 1;
    }
    int size = myMap[key].Tokenize(":")->GetEntries();
    std::vector<TString> result = std::vector<TString>();
    result.reserve(size);
    for (int i=0; i<size; i++) result.push_back( getStr(key,i) );
    return result;
  }

  std::vector<TString> ParameterSet::getStrVector(TString key, TString defaultVal)
  {
    std::vector<TString> defaultVec;
    defaultVec.push_back(defaultVal);
    if (!hasKey(key)) return defaultVec;

    try {
      return getStrVector(key);
    }
    catch (...) {
      return defaultVec;
    }
  }

  std::vector<int> ParameterSet::getIntVector(TString key)
  {
    if (!hasKey(key)) {
      std::cout << "Error, attempting to access key " << key << " that is not in ParameterSet" << std::endl;
      throw 1;
    }

    int size = myMap[key].Tokenize(":")->GetEntries();
    std::vector<int> result;
    result.reserve(size);
    for (int i=0; i<size; i++) result.push_back( getInt(key,i) );
    return result;
  }

  std::vector<double> ParameterSet::getDoubleVector(TString key)
  {
    if (!hasKey(key)) {
      std::cout << "Error, attempting to access key " << key << " that is not in ParameterSet" << std::endl;
      throw 1;
    }
    int size = myMap[key].Tokenize(":")->GetEntries();
    std::vector<double> result;
    result.reserve(size);
    for (int i=0; i<size; i++) result.push_back( getDouble(key,i) );
    return result;
  }

}
