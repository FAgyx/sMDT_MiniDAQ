#include "MuonReco/Optimizer.h"

namespace MuonReco {
  Optimizer::Optimizer() {
    chiSq     = 0;    
    e         = new Event();
    indexLow  = 0;
    indexHigh = 0;
    hitIndex  = 0;
    ignore    = std::vector<int>();
    verbose   = kTRUE;
  }
  
  Optimizer::~Optimizer() {

  }

  void Optimizer::setTarget(TTree* target) {    
    dataset = target;
    dataset->SetBranchAddress("event", &e);
    indexHigh = dataset->GetEntries();
  }

  void Optimizer::setRange(int low, int high) {
    indexLow  = low;
    indexHigh = high;
  }
  
  void Optimizer::setRangeFull() {
    indexLow  = 0;
    indexHigh = dataset->GetEntries();
  }
  
  void Optimizer::setRangeSingle(int index) {
    indexLow  = index;
    indexHigh = index + 1;
  }

  void Optimizer::setIgnoreSingle(int index) {
    ignore.clear();
    ignore.push_back(index);
  }
  
  void Optimizer::setIgnoreNone() {
    ignore.clear();
  }

  void Optimizer::setVerbose(Bool_t b) {
    verbose = b;
  }

  void Optimizer::setTolerance(double tol) {
    tolerance = tol;
  }

  Bool_t Optimizer::skip() {
    hitIndex++;
    for (auto it = ignore.begin(); it != ignore.end(); ++it) {
      if (*it == hitIndex) return kTRUE;
    }
    return kFALSE;
  }

  Bool_t Optimizer::anySkip() {
    if (ignore.size() != 0) return kTRUE;
    return kFALSE;
  }

  void Optimizer::addDependency(Optimizer* opt) {
    dependencies.push_back(opt);
  }

  void Optimizer::addSimultaneous(Optimizer* opt) {
    simultaneous.push_back(opt);
  }


  void Optimizer::setMaxResidual(double max) {
    maxResidual = max;
  }

  void Optimizer::setMaxIteration(int iter) {
    maxIter = iter;
  }

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
  
  double Optimizer::getMaxResidual() {
    return maxResidual;
  }
  
  void Optimizer::optSingle(TTree* target, int index) {
    setTarget(target);
    setRangeSingle(index);
    optimize();
  }

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

  double Optimizer::updateParam() {
    
    DOF -= this->size();

    // update the parameterization
    if (verbose) {
      G.Print();
      Y.Print();
    }
    G.Invert();
    TMatrixD delta = -1.0 * G*Y;
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

}

