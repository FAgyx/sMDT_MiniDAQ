#include "MuonReco/ConfigParser.h"


namespace MuonReco {

  ConfigParser::ConfigParser(TString confPath) {
    std::ifstream infile(confPath);
    std::string raw_line;
    TString line, key, value;
    TObjArray* toks;

    TString current_section;
    while (std::getline(infile, raw_line)) {
      line = TString(raw_line);
      line = line.Strip(TString::kBoth, ' ');

      if (line.BeginsWith("[") && line.EndsWith("]")) {
        // line is section header
        current_section = line(1, line.Length()-2);
        if (line.Contains("Cuts")) {
          // Here, we want insertion order to matter
          // So, don't create the ParameterSet object
          // cutList.push_back(current_section);
          continue;
        }
        pSets[current_section] = ParameterSet();
      }
      else if (line.BeginsWith("#")) {
        // the line denotes a comment and should be ignored
        continue;
      }
      else {
        // line is not a section header, add to current section
        toks = line.Tokenize("=");
        if (toks->GetEntries() != 2) {
          toks = line.Tokenize(":");
        }
        if (toks->GetEntries() == 2) {
          key   = ((TObjString*)(toks->First()))->GetString();
          key   = key.Strip(TString::kBoth, ' ');
          value = ((TObjString*)(toks->Last()))->GetString();
          value = value.Strip(TString::kBoth, ' ');
          if (current_section.Contains("Cuts")){
            // Again, don't use the ParameterSet, instead use List
            // In order to preserve plotting, push back key
            // followed by value for the variable
            cutList.push_back({key,value});
          }
          else {
            pSets[current_section].add(key, value);
          }
        }
      }
    }
  }

  ParameterSet ConfigParser::items(TString key) {
    auto it = pSets.find(key);
    if (it != pSets.end()) {
      return pSets[key];
    }
    else {
      std::cout << "Attempting to access ParameterSet with header " << key
		<< "that does not appear in configuration file" << std::endl;
      return ParameterSet();
    }
  }

  void ConfigParser::Print() {
    auto it = pSets.begin();

    while (it != pSets.end()) {
      std::cout << "[" << it->first << "]" << std::endl;
      it->second.Print();
      it++;
    }
  }

}
