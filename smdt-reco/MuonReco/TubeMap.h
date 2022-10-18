#ifndef MUONRECO_TUBEMAP_H
#define MUONRECO_TUBEMAP_H

#include <bits/stdc++.h>

namespace MuonReco {

  /******************************************
   * Class to encapsulate any value mapped  *
   * to tubes indexed by layer and column   *
   *                                        *
   * Avoids the issue of declaring an array *
   * of variable size.  Abstracts user from *
   * row/column wise storage scheme         *
   *                                        *
   * Author:        Kevin Nelson            *
   * Date:          January 29, 2021        *
   * Last Modified: January 29, 2021        *
   *                                        * 
   ******************************************
   */
  template <class storedType>
  class TubeMap {
  public:
    TubeMap();
    TubeMap(size_t nLayers, size_t nColumns);
    storedType get(size_t layer, size_t column) const;
    storedType getMin() const;
    storedType getMax() const;
    void set(size_t layer, size_t column, storedType input);

  private:
    std::vector<storedType> vec;
    void ResetVector();
    size_t toIndex(size_t layer, size_t column) const;
    size_t _nLayers;
    size_t _nColumns;
  };
}

// need "reverse include" for a templated implementation
#include "MuonReco/TubeMap.cxx"

#endif
