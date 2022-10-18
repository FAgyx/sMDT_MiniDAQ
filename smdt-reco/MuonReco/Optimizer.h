#ifndef MUON_OPTIMIZER
#define MUON_OPTIMIZER

#include <vector>
#include <iostream>

#include "TMatrixD.h"
#include "TTree.h"
#include "TDecompLU.h"
#include "TF1.h"
#include "Math/ProbFuncMathCore.h"

#include "MuonReco/Parameterization.h"
#include "MuonReco/Event.h"
#include "MuonReco/Geometry.h"

namespace MuonReco {
  /*! \class Optimizer Optimizer.h "MuonReco/Optimizer.h"
   * \brief Optimize any parametric function of Hit objects with known derivatives
   * 
   * Class to optimize some parameterization over a set of data, 
   * which is a TTree of MuonReco::Event objects
   *
   * Parameterization is minimized using linear residuals from chi squared
   *
   * Parameterization may represent RT function or track fitting
   *                                      
   * \author:        Kevin Nelson          
   *                 kevin.nelson@cern.ch
   * \date:          May 31, 2019          
   *
   */
  class Optimizer : virtual public Parameterization {
  public:
    Optimizer();
    ~Optimizer();

    bool              optimize       (bool init=kTRUE);
    void              optSingle      (TTree* target, int index);
    void              setTarget      (TTree* target);
    void              setMaxResidual (double max);
    void              setRange       (int low, int high);
    void              setRangeFull   ();
    void              setRangeSingle (int index);
    void              setIgnoreSingle(int index);
    void              setIgnoreNone  ();
    void              setIgnore      (int index);
    void              setIgnoreLayer (int iLayer);
    void              setTolerance   (double tol);
    void              setMaxIteration(int iter);
    void              addDependency  (Optimizer* opt);
    void              addSimultaneous(Optimizer* opt);
    void              setVerbose     (bool b);
    double            getMaxResidual ();
    int               getMaxHitIndex ();
    int               getIgnoredLayer();
    int               getNIterations () {return iteration;}
    double            getChiSqNDF    ();
    double            getChiSq       ();
    double            getDOF         ();
    double            getSystShift   ();
    Event*            getCurrentEvent();

    double            updateParam    ();
    double            doOneIteration (bool init=kTRUE);
    bool              anySkip        ();
    void              increment      (double dist, double res, double err, Hit h);
    void              resetMatrices  ();
    bool              skip        ();
    virtual void      constrain(TMatrixD* delta);

    void pullParameter(int index, double sigma, Bool_t up, double approxError, Bool_t bias);
    double ChiSquareTF1Hook(double* x, double* p);
    double LogLikelihoodHook(double* x, double* p);

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
    int                ignoredLayer = -1;
    int                maxIter   = 1000;
    int                iteration = 0;
    double             tolerance = 1e-3;
    std::vector<int>   ignore;
    int                hitIndex;
    bool               verbose;
    TMatrixD           G, Y;
    TF1*               chiSqDist = 0;
    TF1*               f_LLH = 0;
    double             systShift = 0;
    int                pullParameterIndex = 0;

    double recalculateChiSq();
    

    friend class ResolutionResult;
  };

}
#endif
