
#ifndef MUON_CONFIG_PARSER
#define MUON_CONFIG_PARSER

#include "MuonReco/ParameterSet.h"

#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <utility>

#include "TObjArray.h"
#include "TObjString.h"
#include "TGenericClassInfo.h"

namespace MuonReco {

  /*! \class ConfigParser ConfigParser.h "MuonReco/ConfigParser.h"
   * \brief Class to parse configuration files
   *
   * Configuration files are parsed into separate collections, a set 
   * of ParameterSet objects.  For more information on the configuration
   * file, see the mainpage.
   * 
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date 9 March 2020
   */ 
  class ConfigParser {

  public:
    /**
     * Default constructor
     */
    ConfigParser() {};

    /**
     * Parses the contents of the configuration file and store in the members of this object
     * 
     * @param confPath The path to the configuration file
     */
    ConfigParser(TString confPath);

    /**
     * Default destructor
     */
    virtual ~ConfigParser() {};

    /**
     * Provide access to a ParameterSet with the given heading
     *
     * @param key Heading of the ParameterSet in the configuration file
     */
    ParameterSet items(TString key);

    /**
     * Print the contents of the configuration file
     */
    void         Print();

    std::map<TString, ParameterSet>         pSets;   //! A key-value pair providing access to a set of related parameters
    std::vector<std::pair<TString,TString>> cutList; //! A list of key-value pairs representing the cuts applied to the dataset

  };

}

#endif
