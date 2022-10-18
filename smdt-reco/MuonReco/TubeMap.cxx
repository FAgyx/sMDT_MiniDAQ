
namespace MuonReco {

  template<class storedType>
  TubeMap<storedType>::TubeMap() {
    _nLayers  = 1;
    _nColumns = 1;
    ResetVector();
  }

  template<class storedType>
  TubeMap<storedType>::TubeMap(size_t nLayers, size_t nColumns) {
    _nLayers  = nLayers;
    _nColumns = nColumns;
    ResetVector();
  }

  template<class storedType>
  void TubeMap<storedType>::ResetVector() {
    vec = std::vector<storedType>();
    vec.resize(_nLayers*_nColumns);
  }

  template<class storedType>
  storedType TubeMap<storedType>::get(size_t layer, size_t column) const {
    return vec.at(toIndex(layer, column));
  }

  template<class storedType>
  storedType TubeMap<storedType>::getMin() const {
    return *std::min_element(vec.begin(), vec.end());
  }

  template<class storedType>
  storedType TubeMap<storedType>::getMax() const {
    return *std::max_element(vec.begin(), vec.end());
  }

  template<class storedType>
  void TubeMap<storedType>::set(size_t layer, size_t column, storedType input) {
    vec.at(toIndex(layer, column)) = input;
  }

  template<class storedType>
  size_t TubeMap<storedType>::toIndex(size_t layer, size_t column) const {
    return column*_nLayers + layer;
  }

}
