#include "MuonReco/Optimizer.h"

namespace MuonReco {
  /*! \brief Constructor
   * 
   *  Initializes protected data members
   */
  Optimizer::Optimizer() {
    chiSq     = 0;    
    e         = new Event();
    indexLow  = 0;
    indexHigh = 0;
    hitIndex  = 0;
    ignore    = std::vector<int>();
    verbose   = kTRUE;
  }
  
  /*! \brief Destructor */
  Optimizer::~Optimizer() {
    if (chiSqDist) delete chiSqDist;
  }

  /*! \brief Set the target tree
   * 
   *  The protected pointer dataset will now point to the target tree
   *  The tree must have a branch named "event" of type MuonReco::Event
   *  The number of entries for training is by default set to the number of 
   *  entries in the tree
   */
  void Optimizer::setTarget(TTree* target) {    
    dataset = target;
    dataset->SetBranchAddress("event", &e);
    setRangeFull();
  }

  /*! \brief Set range in number of events
   *  
   * The algorithm will now only optimize over a subset of the target tree, 
   * from [low, high)
   */
  void Optimizer::setRange(int low, int high) {
    indexLow  = low;
    indexHigh = high;
  }
  
  /*! \brief Set algorithm to optimize over full tree 
   */
  void Optimizer::setRangeFull() {
    indexLow  = 0;
    indexHigh = dataset->GetEntries();
  }
  
  /*! \brief Set algorithm to optimize over a single event in the tree
   */
  void Optimizer::setRangeSingle(int index) {
    indexLow  = index;
    indexHigh = index + 1;
  }

  /*! \brief Ignore a single hit with the provided index
   */
  void Optimizer::setIgnoreSingle(int index) {
    ignore.clear();
    ignore.push_back(index);
  }
  
  /*! \brief Ignore hit with this index
   */
  void Optimizer::setIgnore(int index) {
    ignore.push_back(index);
  }

  /*! \brief Do not ignore any hits in this event 
   */
  void Optimizer::setIgnoreNone() {
    ignore.clear();
    ignoredLayer = -1;
  }

  /*! \brief Ignore a given layer
   */
  void Optimizer::setIgnoreLayer(int iLayer) {
    hitIndex = -1;
    ignoredLayer = iLayer;
    auto clusters = e->Clusters();
    for (Cluster c : clusters) {
      for (Hit h : c.Hits()) {
        hitIndex++;
	if (h.Layer() == iLayer) setIgnore(hitIndex);
      }
    }
  }

  /*! \brief Return layer of first ignored hit
   */
  int Optimizer::getIgnoredLayer() {
    return ignoredLayer;
  }

  /*! \brief If b=true, print out output during optimization procedure
   * 
   * If the optimizer is not verbose, cout will be redirected to null
   */
  void Optimizer::setVerbose(Bool_t b) {
    verbose = b;
  }

  /*! \brief Convergence tolerance
   * 
   * If any parameter changes more than the tolerance on any iteration of the 
   * optimization, continue optimizing
   */
  void Optimizer::setTolerance(double tol) {
    tolerance = tol;
  }

  /*! \brief Return true if this Hit is ignored
   */
  Bool_t Optimizer::skip() {
    //hitIndex++;
    for (auto it = ignore.begin(); it != ignore.end(); ++it) {
      if (*it == hitIndex) return kTRUE;
    }
    return kFALSE;
  }

  /*! \brief Return true if any hits are ignored
   */
  Bool_t Optimizer::anySkip() {
    if (ignore.size() != 0) return kTRUE;
    return kFALSE;
  }

  /*! \brief Add dependent optimization
   * 
   * This optimizer will wait for its dependencies to finish, then start
   */
  void Optimizer::addDependency(Optimizer* opt) {
    dependencies.push_back(opt);
  }

  /*! \brief add simultaneous optimization
   * 
   * A simultaneous optimization will re-use data read out from target tree
   * to prevent multiple redundant file IO
   */
  void Optimizer::addSimultaneous(Optimizer* opt) {
    simultaneous.push_back(opt);
  }

  /*! \brief Set maximum residual
   * 
   *  Units on max residual are in standard deviations
   *  The class MuonReco::Hit provides a static method
   *  to translate standard deviations to distance 
   *  using the nominal resolution of the tube
   *  
   */
  void Optimizer::setMaxResidual(double max) {
    maxResidual = max;
  }

  /*! \brief If max iteration is exceeded, optimization will terminate
   */
  void Optimizer::setMaxIteration(int iter) {
    maxIter = iter;
  }

  /*! \brief Return number of hits in this event that are clustered
   */
  int Optimizer::getMaxHitIndex() {
    hitIndex = -1;
    auto clusters = e->Clusters();
    for (Cluster c : clusters) {
      for (Hit h : c.Hits()) {
        hitIndex++;
      }
    }
    return hitIndex;
  }

  double Optimizer::getChiSq() {
    return chiSq;
  }

  double Optimizer::getDOF() {
    return DOF;
  }

  double Optimizer::getChiSqNDF() {
    return chiSq/DOF;
  }

  Event* Optimizer::getCurrentEvent() {
    return e;
  }

  /*! \brief Return maximum residual, in standard deviations
   */
  double Optimizer::getMaxResidual() {
    return maxResidual;
  }
  
  /*! \brief Shortcut to optimize a single event.
   * 
   * Sets target tree, range to one event.  Then runs optimization
   */
  void Optimizer::optSingle(TTree* target, int index) {
    setTarget(target);
    setRangeSingle(index);
    optimize();
  }

  /*! \brief Primary hook for optimizing any parameterization
   * 
   *  Tracks the number of iterations and, if verbose, will print output
   *  during optimization.
   *  \see Optimizer::doOneIteration for more details
   */
  Bool_t Optimizer::optimize(Bool_t init/*=kTRUE*/) {
    freopen("/dev/null", "w", stderr);

    Bool_t didConverge = kFALSE;
    double maxChange = 1;
    iteration = 0;

    if (verbose)  std::cout << "Optimizing..." << std::endl;

    while (maxChange > tolerance && iteration < maxIter) {
      std::cout.setstate(std::ios_base::failbit);
      maxChange = doOneIteration(init);
      std::cout.clear();
      if (verbose) {
	std::cout << "Iteration:   " << iteration << std::endl;
	std::cout << "Max Change:  " << maxChange << std::endl;
	std::cout << "Chi Squared: " << chiSq     << std::endl;
	std::cout << "DOF:         " << DOF       << std::endl;
	std::cout << std::endl;
      }
      iteration++;
      if (maxChange <= tolerance) didConverge = kTRUE;
    }
    if (verbose) { 
      std::cout << "Done" << std::endl;      
      this->Print();
      if (didConverge) std::cout << "Did converge" << std::endl;
      else std::cout << "Did not converge" << std::endl;
    }
    
    return didConverge;
  }
  
  /*! \brief Resize and clear internal matrices
   * 
   * In the case where the number of degrees of freedom has changed the matrices will be 
   * appropriately resized using Parameterization::size()
   */
  void Optimizer::resetMatrices() {
    hitIndex   = -1;
    chiSq      =  0;
    DOF        =  0;

    G.ResizeTo(TMatrixD(this->size(), this->size()));
    Y.ResizeTo(TMatrixD(this->size(), 1));
    for (int r = 0; r < this->size(); r++) {
      for (int c = 0; c < this->size(); c++) {
	G[r][c] = 0;
	Y[r][0] = 0;
      }
    }
  }

  /*! \brief Increment internal matrices
   * 
   * The Optimizer class uses the matrix inversion linearized residual chi square methodology
   * to optimize a Parameterization.  This method checks if the residual is close enough to be
   * included based on its error and residual, then uses the derivative of the chi squared with 
   * respect to each parameter to increment the relevant matrices
   */
  void Optimizer::increment(double dist, double res, double err, Hit h) {
    if (dist>0 && dist < Geometry::radius && TMath::Abs(res) < maxResidual*err) {
      DOF++;

      // calculate all derivatives in context of this hit
      TMatrixD D = TMatrixD(this->size(),1);
      for (int j = 0; j < this->size(); j++) {
	D[j][0] = this->D(j, h);
      } // end for: calculate the derivatives

      // calculate the matrices
      for (int r = 0; r < this->size(); r++) {
	for (int c = 0; c < this->size(); c++) {
	  G[r][c] += D[r][0]*D[c][0]/(err*err);
	}
	Y[r][0] += res*D[r][0]/(err*err);
      }

      chiSq += TMath::Power((res/err), 2);
    }
  }
  
  /*! \brief Loop over ttree, optimizing this parameterization
   *
   * MuonReco::Optimizer has a client-server relationship with itself
   * Other Optimizers may be simultaneously or dependently optimized, 
   * managed in this method.
   * 
   * Dependencies are optimized first
   * 
   * This method also checks the event flag using MuonReco::Event::Pass()
   * which must be true for the event to be included in optimization
   * 
   * For event event, this method loops over hits and calculates the residual
   * and distance using virtual optimizer interface and calls Optimizer::increment
   * to update the internal matrices before Optimizer::updateParam to perform the 
   * matrix inversion
   */
  double Optimizer::doOneIteration(Bool_t init/*=kTRUE*/) {

    resetMatrices();
    for (Optimizer* sim : simultaneous) sim->resetMatrices();
    double res, dist, err;

    for (int i = indexLow; i < indexHigh; i++) {
      if (dependencies.size() != 0) {
	for (Optimizer* opt : dependencies) {
	  opt->optSingle(dataset, i);
	}
	dataset->SetBranchAddress("event", &e);
      }
      dataset->GetEntry(i);

      if (e->Pass()) {
	if (init && iteration == 0) {
	  this->Initialize(e);
	  for (Optimizer* sim : simultaneous) sim->Initialize(e);
	}
	auto clusters = e->Clusters();
	for (Cluster c : clusters) {
	  for (Hit h : c.Hits()) {

	    hitIndex++;
	    if (skip()) continue;

	    res  = this->Residual(h);
	    dist = this->Distance(h);
	    err  = Hit::RadiusError(dist);

	    increment(dist, res, err, h);
	    for (Optimizer* sim : simultaneous) {
	      if (!sim->skip())
		sim->increment(dist, res, err, h);
	    }
	  } // end for: clustered hits
	} // end for: clusters
      } // end if: event passed cuts
    } // end for: all hits in dataset

    double maxDelta = updateParam();
    double simDelta;
    for (Optimizer* sim : simultaneous) {      
      simDelta = sim->updateParam();
      if (simDelta > maxDelta) maxDelta = simDelta;
    }
    return maxDelta;
  }

  /*! \brief Invert the residual matrix, update coefficeients
   * 
   * In the case of a singular matrix, use on-diagonal elements to 
   * approximate size of shift needed
   */
  double Optimizer::updateParam() {
    
    DOF -= this->size();

    // update the parameterization
    if (verbose) {
      G.Print();
      Y.Print();
    }
    
    TDecompLU lu(G);
    if (!lu.Decompose()) {
      for (int r = 0; r<G.GetNrows(); r++) {
	for (int c = 0; c<G.GetNcols(); c++) {
	  if (r == c && G[r][c]) 
	    G[r][c] = 1.0/G[r][c];
	  else
	    G[r][c] = 0;
	}
      }
    }
    else G.Invert();

    TMatrixD delta = -1.0 * G*Y;
    constrain(&delta);
    
    Parameterization update = *this;

    if (verbose) {
      delta.Print();
    }

    for (int i = 0; i < this->size(); i++) 
      update.setParam(i, delta[i][0]);
    
    *this += update;

    // return the maximum change
    double maxDelta = 0;
    for (int r = 0; r < this->size(); r++) {
      if (TMath::Abs(delta[r][0]) > maxDelta)
	maxDelta = TMath::Abs(delta[r][0]);
    }
    return maxDelta;
  } // end method: updateParam()

  /**
   * hook to constrain the optimization steps
   */
  void Optimizer::constrain(TMatrixD* delta) {
    return;
  }

  /**
   * Private method to be used !only! by pullParameter() method.
   * This chi-sq calculation ignores the concept of "maximum residual"
   * in order to make chi-sq monotonically increase as parameters
   * are driven away from their best fit values
   */
  double Optimizer::recalculateChiSq() {
    double res, dist, err;
    hitIndex = -1;
    chiSq = 0;
    auto clusters = e->Clusters();
    for (Cluster c : clusters) {
      for (Hit h : c.Hits()) {
	
	hitIndex++;
	if (skip()) continue;
	
	dist = this->Distance(h);
	res  = this->Residual(h);
	err  = Hit::RadiusError(dist);
	if (dist>0 && dist < Geometry::radius) {
	  chiSq += TMath::Power((res/err), 2);
	}
      }
    }
    return chiSq;
  }

  double Optimizer::ChiSquareTF1Hook(double* x, double* p) {
    setParam((int)(p[0]), x[0]);
    return recalculateChiSq();
  }

  double Optimizer::LogLikelihoodHook(double* x, double* p) {
    setParam(pullParameterIndex, x[0]);
    
    double res, dist, err, sig;
    hitIndex = -1;
    double LLH = 1;
    auto clusters = e->Clusters();
    for (Cluster c : clusters) {
      for (Hit h : c.Hits()) {
	
	hitIndex++;
	if (skip()) continue;
	
	dist = this->Distance(h);
	res  = this->Residual(h);
	err  = Hit::RadiusError(dist);
	sig  = res/err;
	LLH += -0.5*sig*sig-TMath::Log(TMath::Sqrt(2*TMath::Pi())*err);
      }
    }
    return LLH;
  }

  /**
   * Function to displace a parameter from its best fit value by a 
   * given significance
   */
  void Optimizer::pullParameter(int index, double sigma, Bool_t up, double approxError, Bool_t bias) {
    // first, set all hits not used to be skipped
    // this will prevent LLH from using different data than original fit
    hitIndex = -1;
    auto clusters = e->Clusters();
    for (Cluster c : clusters) {
      double dist, res, err;
      for (Hit h : c.Hits()) {

	hitIndex++;
        if (skip()) continue;

        dist = this->Distance(h);
        res  = this->Residual(h);
        err  = Hit::RadiusError(dist);

	if (!(dist>0 && dist < Geometry::radius && TMath::Abs(res) < maxResidual*err)) {
	  ignore.push_back(hitIndex);
	}
      }
    }


    while (index >= this->size()) param.push_back(1.0);
    
    pullParameterIndex = index;

    double valMin      = (up)  ? this->operator[](index) : this->operator[](index) - approxError;
    double valMax      = (!up) ? this->operator[](index) : this->operator[](index) + approxError;

    if (bias) {
      valMin = this->operator[](index) - approxError;
      valMax = this->operator[](index) + approxError;
    }


    // calculate the target chi-square value to optimize towards
    double bestFitChiSq = chiSq;
    if (chiSqDist == 0) {
      f_LLH = new TF1("optLocalLLH", this, &Optimizer::LogLikelihoodHook, valMin, valMax,
		      0,"Optimizer", "LogLikelihoodHook");
    }

    double likelihoodTarget = f_LLH->Eval(this->operator[](index)) - sigma*sigma/2.0;


    double oldParam = this->operator[](index);
    double newParam = f_LLH->GetX(likelihoodTarget, valMin, valMax);

    setParam((int)(index), newParam);
    systShift = newParam - oldParam;
  }

  double Optimizer::getSystShift() {
    return systShift;
  }
}

