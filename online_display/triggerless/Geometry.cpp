#ifndef MUON_GEOMETRY
#define MUON_GEOMETRY


namespace Muon {

  /**********************************************
   *                                            *
   * An implementation of the singleton pattern *
   * to allow global access to one instance     *
   * of the Geometry object, which describes    *
   * the geometry of the muon chamber and       *
   * implements useful functions                *
   *                                            *
   * This class contains information about      *
   * mapping tdc/channel to spatial location    *
   * and should be updated with new run         *
   * configurations as new data is taken        *
   *                                            *
   **********************************************
   */
  class Geometry {
  public:
    Geometry();
    ~Geometry();
    void   SetRunN           ();
    bool   IsActiveTDC       (int tdc) const;
    bool   IsActiveTDCChannel(int tdc, int ch) const;


    static const Int_t MAX_TDC         = 18;
    static const Int_t MAX_TDC_CHANNEL = 24;
    static const Int_t MAX_TUBE_LAYER  =  8;
    static const Int_t MAX_TUBE_COLUMN = 54;
    static const Int_t MAX_TDC_COLUMN  =  6;
    static const Int_t MAX_TDC_LAYER   =  4;

    static constexpr Double_t layer_distance  = 13.0769836;
    static constexpr Double_t column_distance = 15.1;
    static constexpr Double_t radius          = 7.5;
    static constexpr Double_t ML_distance     = 224.231;
    static constexpr Double_t min_drift_dist  = 0.0;
    static constexpr Double_t max_drift_dist  = 7.1;

    short TRIGGER_CH;
    short TRIGGER_MEZZ;
    short TDC_ML [Geometry::MAX_TDC];
    short TDC_COL[Geometry::MAX_TDC];

  private:
    bitset<Geometry::MAX_TDC> isActiveTDC;

  };

  Geometry::Geometry() {
    TRIGGER_CH   = 23;
    TRIGGER_MEZZ =  7;
  } // end method: Geometry initialization


  Geometry::~Geometry() {
    
  }



  bool Geometry::IsActiveTDC(int tdc) const {
    return isActiveTDC[tdc];
  }

  bool Geometry::IsActiveTDCChannel(int tdc, int ch) const {
    if (tdc == TRIGGER_MEZZ)
      return 0;//ch == TRIGGER_CH;
    else
      return isActiveTDC[tdc];
  }

  void Geometry::SetRunN() {  
      TRIGGER_CH   = 23;
      TRIGGER_MEZZ =  7;

      TDC_ML[0]  = 0;
      TDC_ML[1]  = 0;
      TDC_ML[9]  = 0;
      TDC_ML[2]  = 0;
      TDC_ML[3]  = 1;
      TDC_ML[4]  = 1;
      TDC_ML[5]  = 1;
      TDC_ML[6]  = 1;


      TDC_COL[0]  = 1;
      TDC_COL[1]  = 2;
      TDC_COL[9]  = 3;
      TDC_COL[2]  = 4;
      TDC_COL[3]  = 1;
      TDC_COL[4]  = 2;
      TDC_COL[5]  = 3;
      TDC_COL[6]  = 4;


      isActiveTDC.reset();
      isActiveTDC[7]  = 1;
      isActiveTDC[0]  = 1;
      isActiveTDC[1]  = 1;
      isActiveTDC[2]  = 1;
      isActiveTDC[3]  = 1;
      isActiveTDC[4]  = 1;
      isActiveTDC[5]  = 1;
      isActiveTDC[6]  = 1;
      isActiveTDC[9]  = 1;

  } // end function: SetRunN

}

#endif

