#ifndef MUON_OPTIMIZER
#define MUON_OPTIMIZER

#include <vector>
#include <iostream>

#include "TMatrixD.h"
#include "TTree.h"

#include "MuonReco/Parameterization.h"
#include "MuonReco/Event.h"
#include "MuonReco/Geometry.h"

namespace MuonReco {
  /******************************************* 
   * Class to optimize some parameterization *
   * over a set of data, which is a TTree    *
   * of reconstructed events.                *
   *                                         * 
   * Parameterization is minimized using     *
   * linear residuals from chi squared       *
   *                                         *
   * Parameterization may represent RT func  *
   * or track fitting                        *
   *                                         *
   * Author:        Kevin Nelson             *
   * Date:          May 31, 2019             *
   * Last Modified: May 31, 2019             *
   *                                         *
   *******************************************
   */
  class Optimizer : virtual public Parameterization {
  public:
    Optimizer();
    ~Optimizer();

    bool            optimize       (bool init=kTRUE);
    void              optSingle      (TTree* target, int index);
    void              setTarget      (TTree* target);
    void              setMaxResidual (double max);
    void              setRange       (int low, int high);
    void              setRangeFull   ();
    void              setRangeSingle (int index);
    void              setIgnoreSingle(int index);
    void              setIgnoreNone  ();
    void              setTolerance   (double tol);
    void              setMaxIteration(int iter);
    void              addDependency  (Optimizer* opt);
    void              addSimultaneous(Optimizer* opt);
    void              setVerbose     (bool b);
    double            getMaxResidual ();
    int               getMaxHitIndex ();
    double            getChiSqNDF    ();
    double            getChiSq       ();
    double            getDOF         ();
    Event*            getCurrentEvent();

    double            updateParam    ();
    double            doOneIteration (bool init=kTRUE);
    bool            anySkip        ();
    void              increment      (double dist, double res, double err, Hit h);
    void              resetMatrices  ();
    bool            skip        ();

    std::vector<Optimizer*> dependencies;
    std::vector<Optimizer*> simultaneous;
  private:
    TTree*             dataset;
    Event*             e;
    double             chiSq;
    double             DOF;
    double             maxResidual;
    int                indexLow;
    int                indexHigh;
    int                maxIter   = 1000;
    int                iteration = 0;
    double             tolerance = 1e-3;
    std::vector<int>   ignore;
    int                hitIndex;
    bool               verbose;
    TMatrixD           G, Y;

    friend class ResolutionResult;
  };

}
#endif
