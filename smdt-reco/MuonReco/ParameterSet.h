#ifndef MUON_PARAMETER_SET 
#define MUON_PARAMETER_SET

#include <map>
#include <vector>
#include <iostream>

#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"

namespace MuonReco {

  /*! \class ParameterSet ParameterSet.h "MuonReco/ParameterSet.h"
   * \brief Provides access to sets of key-value pairs in a configuration file
   * 
   * A configuration file will have several headers with the syntax [Header]
   * followed by key-value pairs (follows the pythonic ConfigParser syntax)
   * A vector of parameters with the syntax
   * key= elem1 : elem2 : elem3
   * may be accessed by providing an index (starting at zero)
   * 
   * A default value may be provided to most accesses in case the key is not present
   * 
   * \author Kevin Nelson
   *         kevin.nelson@cern.ch
   * \date   9 March 2020
   */
  class ParameterSet {
  public:

    /**
     * Default constructor
     *
     * ParameterSet is always initialized empty and populated by ConfigParser
     * @see ConfigParser
     */
    ParameterSet();


    /**
     * Destructor
     */
    virtual ~ParameterSet() {};


    /**
     * Add a key-value TString pair to this set of parameters
     * 
     * Used by ConfigParser when reading in a configuration file
     * @see ConfigParser
     */
    void    add      (TString key, TString value);


    /**
     * Print the key value pairs as they appear in the text of the configuration file
     */
    void    Print    ();


    /**
     * Returns the size of the underlying map
     */
    double  Size();


    /**
     * Returns the keys in the underlying map
     */
    std::vector<TString> keys();


    /**
     * Check if the provided key is in the underlying map
     *
     * @param   key    parameter to be checked
     * @returns Bool_t true if key is in the underlying map
     */
    Bool_t  hasKey   (TString key);


    /** 
     * Access element as TString object with no default value
     * 
     * @param   key   parameter to be read
     * @param   index index for list-like accesses, defaults if not provided
     * @throws  1     if the key is not in ParameterSet
     * @returns       value in TString object
     */
    TString getStr   (TString key, int index=-1);


    /**
     * Access element as TString object with a default value
     * 
     * @param   key        parameter to be read
     * @param   defaultVal default value if key is not found
     * @param   index      index for list-like accesses
     * @returns            value in TString object
     */
    TString getStr   (TString key, TString defaultVal, int index=-1);   


    /**
     * Access element cast to integer with no default value
     * 
     * @param   key   parameter to be read
     * @param   index index for list-like accesses, defaults if not provided
     * @throws  1     if key is not in ParameterSet
     * @returns       value cast to integer by TString::Atoi()
     */
    int     getInt   (TString key, int index=-1);


    /**
     * Access element (cast to integer) with default value
     * 
     * If the key is not in ParameterSet the default value passed is returned.
     * index is a required parameter to avoid collision with non-defaulting getInt() method
     * 
     * @param   key        parameter to be read
     * @param   defaultVal default value if key is not found
     * @param   index      index for list-like accesses
     * @returns            value cast to integer by TString::Atoi() 
     */
    int     getInt   (TString key, int defaultVal, int index); 


    /**
     * Access element (cast to double) with no default value
     *
     * @param   key   parameter to be read
     * @param   index index for list-like accesses, defaults if not provided
     * @throws  1     if key is not in ParameterSet
     * @returns       value cast to double by TString::Atof()
     */
    double  getDouble(TString key, int index=-1);


    /**
     * Access element (cast to double) with default value
     *
     * @param   key        parameter to be read
     * @param   defaultVal default value if key is not found
     * @param   index      index for list-like accesses, defaults if not provided
     * @returns            value cast to double by TString::Atof()
     */
    double  getDouble(TString key, double defaultVal, int index=-1);


    /**
     * Access element (cast to Bool_t) with no default value
     * 
     * @param   key   parameter to be read
     * @param   index index for list-like accesses, defaults if not provided
     * @throws  1     if key is not in ParameterSet
     * @returns       value cast to Bool_t (True, kTRUE, true, or 1 are acceptable truth values, else kFALSE)
     */
    Bool_t  getBool  (TString key, int index=-1);


    /**
     * Access element (cast to Bool_t) with default value
     *
     * index is a required parameter to avoid collision with non-defaulting getBool() method, as Bool_t can be 
     * cast to int
     * 
     * @param   key        parameter to be read
     * @param   defaultVal default value if key is not found
     * @param   index      index for list-list accesses
     * @returns            value cast to Bool_t (True, kTRUE, true, or 1 are acceptable truth values, else kFALSE)
     */
    Bool_t  getBool  (TString key, Bool_t defaultVal, int index);


    /**
     * Access complete list of colon-separated parameters (cast to TString objects) indexed by provided key
     * 
     * list syntax is (not sensitive to whitespace)
     * key= elem1 : elem2 : ... : elemN
     * 
     * @param   key parameter to be read
     * @throws  1   if key is not in underlying map     
     * @returns     std::vector<TString> of all N elements listed after the provided key
     */
    std::vector<TString> getStrVector   (TString key);


    /**
     * Access complete list of colon-separated parameters (cast to TString objects) indexed by provided key
     * with a single default value
     * 
     * list syntax if (non sensitive to whitespace)
     * key= elem1 : elem2 : ... : elemN
     *
     * @param   key parameter to be read
     * @returns     std::vector<TString> of all N elements listed after the provided key
     *              or, if the key is not present push the default value onto an empty vector and return
     */
    std::vector<TString> getStrVector   (TString key, TString defaultVal);


    /**
     * Access compplete list of colon-separated parameters (cast to int) indexed by provided key
     *
     * list syntax is (not sensitive to whitespace)
     * key= elem1 : elem2 : ... : elemN
     * 
     * @param  key parameter to be read
     * @throws 1   if key is not in underlying map
     * @returns    std::vector<int> of all N elements listed after the provided key
     */
    std::vector<int>     getIntVector   (TString key);


    /**
     * Access complete list of colon-separated parameters (cast to double) indexed by provided key
     * 
     * list syntax is (not sensitive to whitespace)
     * key= elem1 : elem2 : ... : elemN
     *
     * @param  key parameter to be read
     * @throws 1   if key is not in underlying map
     * @returns    std::vector<double> of all N elements listed after the provided key
     */
    std::vector<double>  getDoubleVector(TString key);


  private:
    std::map<TString, TString> myMap; //! The underlying map object wrapped by this class

  };

}

#endif
