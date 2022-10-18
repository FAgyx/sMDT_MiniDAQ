#ifndef MUON_ARGPARSER
#define MUON_ARGPARSER

#include "TString.h"

#include <iostream>
#include <map>

namespace MuonReco {
  /*! \mainpage sMDT Test Chamber Reconstruction and Simulation
   * 
   *  \tableofcontents
   * 
   *  \section sec_intro Introduction
   *
   * For a complete discussion of the procedure and results, see ATL-COM-MUON-2020-045 on CDS.
   * 
   * This code is designed to reconstruct raw data files from the University of Michigan sMDT test chamber, as well as simulate the test chamber in Geant4.
   * 
   * The code produces diagnostic plots of raw data, fits \f$t_0\f$ from drift time spectra, auto-calibrates an r(t) function, runs a Geant4 simulation of the test chamber, fits tracks, and produces a final resolution measurement of the chamber.
   *
   * \section sec_install Installation Guide
   *
   * Simply clone the repository with your preferred method (HTTPS, SSH, etc) i.e. run 
   * \code{.sh}
   * git clone ssh://git@gitlab.cern.ch:7999/kenelson/smdt-reco.git
   * \endcode
   * 
   * This package is standalone, so it does not run off of an AnalysisBase or Athena build.  The only dependencies are CMake, ROOT, and Geant4, which are avaiable on CVMFS.
   * 
   * \section sec_build Building the code
   * 
   * The code is compiled using CMake.  On a CVMFS mounted machine, simply source the setup script in the top-level directory
   * \code{.sh}
   * cd smdt_reco
   * source setup.sh
   * \endcode
   *
   * This script will setup ROOT, CMake, and Geant4, set environment variables, and create aliases to built binaries.
   *
   * Next, build the code
   * \code{.sh}
   * mkdir build
   * cd build
   * cmake ..
   * make
   * \endcode
   * 
   * 
   * \section sec_getdata Acquiring Raw Data
   * 
   * Raw data files are not tracked by git (for obvious reasons!).  They are hosted on eos and avaliable for download.  They should be stored in a raw data directory under the top level (on the same level as your build area).  To download, run 
   * 
   * \code{.sh}
   * cd $SMDT_DIR
   * mkdir raw
   * cd raw
   * scp scp <YOUR_USERNAME>@lxplus.cern.ch:/eos/user/k/kenelson/smdt-reco-files/raw/<DESIRED_RAW_FILE> .
   * \endcode
   *
   * There are several raw data files on the order of GB in this area.  At a minimum you must download the file Rt_BMG_6_1.dat, which is a txt file initializaiton of the r(t) function.
   *
   *
   * \section sec_run Running the code
   *
   * After building there will be several executables in your build area.  The setup script creates aliases for these so they may be quickly run.  For instance, running 
   *
   * \code{.sh}
   * decodeRawData --conf conf/run188194.conf
   * \endcode
   *
   * From the top-level directory will create an output area with plots showing diagnostics of the raw data.
   * 
   * In order to produce a resolution measurement, run these commands in order:
   * 
   * \code{.sh}
   * decodeRawData --conf conf/run188194.conf
   * doT0Fit --conf conf/run188194.conf
   * autoCalibration --conf conf/run188194.conf
   * residuals --conf conf/run188194.conf
   * residuals -h --conf conf/run188194
   * resolution --conf conf/run188194.conf
   * \endcode
   *
   * Where you should replace the configuration file with the one appropriate for your run.
   * The last two commands create residual distributions using the biased and unbiased methods, respectively.  These distributions will appear in the directory output/run<N>/fitResiduals and output/run<N>/hitResiduals
   *
   * In the following sections I will detail the configuration file and the output from each of the above commands
   * 
   * \subsection sec_conf The Configuration File
   *
   * All of the macros for this project are configurable by a text file.  These text files are found in the conf directory, underneath the top level.  An example configuration file is shown below:
   *
   *     [General]
         RawFileName         = run00188512_20200709_1.dat
         RunNumber           = 188512
         IsMC                = 0
         
         [RecoUtility]
         MIN_HITS_NUMBER     = 8
         MAX_HITS_NUMBER     = 8
         MAX_TIME_DIFFERENCE = 200
         MIN_CLUSTER_SIZE    = 4
         MAX_CLUSTER_SIZE    = 4
         MIN_CLUSTERS_PER_ML = 1
         MAX_CLUSTERS_PER_ML = 1
         TRIGGER_OFFSET      = 100
         
         [Geometry]
         RunNumber           = 188512
         TriggerMezz         = 5
         TriggerChannel      = 23
         ActiveTDCs          = 0 : 1 : 2 : 3 : 4 : 7 : 8 : 9
         TDCMultilayer       = 0 : 0 : 0 : 1 : 1 : 1 : 0 : 1
         TDCColumn           = 1 : 2 : 3 : 1 : 2 : 3 : 4 : 4
         
         [AutoCalibration]
         Parameterization = Chebyshev
         MinEvent         =       0
         NEvents          =  100000
         
         [Resolution]
         DeconvolutionRun = 9
   *
   * The supported options include:
   *
   * | Section | Attribute   | Description |
   * | :----   | :----       | :----       |
   * | General | RawFileName | Location of raw file (underneath sMDT/raw directory) |
   * | ^ | RunNumber   | Run Number for this run (used in naming convention for output path) |
   * | ^       | IsMC        | If nonzero, mark this run as Monte Carlo.  Will skip T0 fitting and adopt a nominal maximum drift time. |
   * | RecoUtility | MIN_HITS_NUMBER | The minimum number of clustered hits for a reconstructed Event |
   * | ^           | MAX_HITS_NUMBER | The maximum number of clustered hits for a reconstructed Event |
   * | ^           | MAX_TIME_DIFFERENCE | The maximum difference in hit arrival time (in nanoseconds) |
   * | ^           | MIN_CLUSTER_SIZE | Minimum size of a reconstructed cluster |
   * | ^           | MAX_CLUSTER_SIZE | Maximum size of a reconstructed cluster |
   * | ^           | MIN_CLUSTERS_PER_ML | Minimum number of reconstructed clusters per multilayer |
   * | ^           | MAX_CLUSTERS_PER_ML | Maximum number of reconstructed clusters per multilayer |
   * | ^           | TRIGGER_OFFSET | Triggers are searched for in a narrow time window to reduce noise.  Data taken before run 188512 generally perform well with an offset of zero, and after with an offset of 100 nanoseconds |
   * | Geometry    | RunNumber | Depreciated. |
   * | ^           | TriggerMezz | For cosmic data, the mezzanine card number for the trigger system. |
   * | ^           | TriggerChannel | For cosmic data, the channel number on the trigger mezzanine card corresponding to the trigger.  The signal on this channel is the coincidence of PMTs on the scintillator. |
   * | ^           | ActiveTDCs | List of integers separated by colons.  Each number is the number of the mezzanine card for each TDC readout board. |
   * | ^           | TDCMultiLayer | List of integers separated by colons.  Each number is the multilayer (0 or 1) in which the active TDC mezzanine card is installed.  Directly corresponds to the ActiveTDCs list, so the number in the nth position in both lists describes the same mezzanine card. |
   * | ^           | TDCColumn | List of integers separated by colons.  Each number is the column ( 0 through 8) position of the mezzanine card.  Like TDCMultilayer, these numbers correspond directly to the order specified in ActiveTDCs. |
   * | AutoCalibration | Parameterization | Chebyshev or LinearInterpolation.  This specifies the way the r(t) function will be described, either as 10 term Chebyshev polynomial, or 100 distinct points with lienar interpolation.  Chebyshev is recommended. |
   * | ^            | MinEvent | First event number to be used.  Typically 0. |
   * | ^            | NEvents  | Number of events used for autocalibration.  Typically 100000 |
   * | Resolution   | DeconvolutionRun | Run number from which to retrieve mutliple scattering response distributions for deconvoluting observed residual distribution.  Should correspond to a MC run with the same conditions as the data run (i.e. was shielding used to cut low energy cosmic electrons?) |
   *
   *
   * \subsection sec_run_decode Decoding Raw Data
   * \code{.sh}
   * decodeRawData --conf conf/run<N>.conf
   * \endcode
   *
   * This step reads the raw data file and reconstructs MuonReco::Event objects and stores them in a TTree.  These events are then passed to the next steps of the data analysis.  The raw data simply encodes the time stamp of rising and falling edges on each channel.  Each dataword is 32 bits and represents a single rising or falling edge.  The 32 bit dataword is broken down as: 
   * - 4 bit signal type (rising, falling, error code, etc.)
   * - 4 bit TDC number 
   * - 5 bit channel number (0-23)
   * - 12 bit coarse timing (25 nanosecond resolution)
   * - 7 bit fine timing (25/128 ns resolution)
   *
   * The code matches rising and falling edges on the same channel and mezzanine card and creates Hit objects.  Then, hits that are adjacent are clustered.  Finally, the event is marked as passing or failing the cuts specified in your configuration of MuonReco::RecoUtility.
   *
   * \subsubsection sec_sub_run_decode_output Output
   * 
   * This step has a significant amount of output, and it is worth spending some time to review it to ensure data quality.  In this step and all future steps, output will be written to a directory sMDT/output/run<RunNumber>.  The class MuonReco::IOUtility will handle recursive directory creation if they do not already exist.  This step creates a subdirectory events_passing with a sample of the first 100 events that pass the cuts specified in your configuration of MuonReco::RecoUtility.  Additionally, events_failing has the event display of the first 100 events that do not meet cuts.  Reviewing *both* passing and failing events is crucial to diagnosing problems.  The file Events.root contains the reconstructed MuonReco::Event objects in a TTree.  This is made possible by the generation of a dicitonary for all objects that inherit the MuonReco::RecoObject interface, which occurs when the code is compiled.  Additionally, ADC (pulse length) and TDC (drift time) spectra are plotted, but are better reviewed after completing the next step.
   *
   * \subsection sec_run_t0fit T0 Fitting
   * \code{.sh}
   * dot0Fit --conf conf/run<N>.conf
   * \endcode
   *
   * Fits the ADC and TDC spectra.  At this stage it is useful to review the plots and make sure the data quality is high enough.  High quality data will have a mean T0 slope of no more than 3 nanoseconds and a peak ADC of near 100.
   *
   * This step outputs a series of plots showing the fit for each spectrum with the fit in the T0Fits directory.  Additionally, a file T0.root contains all fit parameters in a TTree for compiling results from different runs into common storage.
   * 
   * \subsection sec_run_autocal AutoCalibration
   * \code{.sh}
   * autoCalibration --conf conf/run<N>.conf
   * \endcode
   *
   * Fit a r(t) function.  This is accomplished by the linearized-chi square matrix inversion technique implemented in MuonReco::Optimizer.  The initial guess is seeded as the gas-monitor r(t) avaliable for download from EOS.  
   *
   * This step creates output in a subdirectory autocalibration under the typical output area.  This directory has an image of the r(t) function.  At this step, check that the r(t) function has no idiosyncrasies (it should be relatively smooth and monotonically increasing).  The r(t) function itself is stored in a file autoCalibratedRT.root.
   *
   * \subsection sec_run_fit Fit (biased) Residuals
   * 
   * \code{.sh}
   * residuals --conf conf/run<N>.conf
   * \endcode
   *
   * Fit a straight line track to the hits.  Measure the residual (difference between hit radius and distance from track to center of wire) and create histograms.  An addtional degree of freedom is also provided by the T0 shift.  This is an event-by-event shift in global timing.
   * 
   * This step creates an output subdirectory fitResiduals.  It has several images, including the T0 shift distribution, chi-square distribution, residual distribution, and residual distribution as a function of radial position.  If the r(t) function found in the previous step was reasonable then the residual distribution should agree well with its double gaussian fit, the radial position distribution should be rather flat, and the chi-square distribution should match the theoretical prediction.
   *
   * \subsection sec_run_hit Hit (unbiased) Residuals
   *
   * \code{.sh}
   * residuals --conf conf/run<N>.conf -h
   * \endcode
   *
   * The same as the previous step, but this time run the code with the additional option -h, to specify hit residual calculation.
   * This method will leave out each hit, perform the track fitting, and the measure the residual of that single ignored hit.
   * Thus, for an event with N hits, N distinct track fits are performed.
   *
   * Output from this step is identical to the output from the fit residuals step, but this time is in a directory named "hitResiduals" under the standard output directory.
   *
   * 
   * 
   * \subsection sec_run_resolution Resolution
   * 
   * \code{.sh}
   * resolution --conf conf/run<N>.conf
   * \endcode
   * 
   *
   * In the final step, we calculate the resolution.  Simply, the resolution is the geometric mean of the amplitude weighted double gaussian fits: \f$\sigma=\sqrt{\sigma_f\times\sigma_h}\f$
   *
   * However, our procedure is not so simple.  We must account for multiple scattering by deconvoluting with a MC truth distribution.  Running the MC code is discussed in the next section.
   *
   * Output from this step is in a directory "resolution" under the standard output.  It will show the deconvolution step for the fit and hit residuals.  Each of these .png files shows the plot in "x" space, as well as the fourier transform in three separate plots: real, imaginary, and magnitude.
   *
   * The final result for the single hit resolution is simply output to the screen. 
   *
   * Raw data are written out to a single file for each run (see above, "Acquiring Raw Data").  
   *
   * \subsection sec_run_mc Monte Carlo
   *
   * \code{.sh}
   * runBeam --conf conf/run<N_MC>.conf
   * residuals --conf conf/run<N_MC>.conf
   * \endcode
   *
   * There is a major difference here, where the run number is not the number of the data run, but the number to which you would like to label the MC run.  Typically, I indexed MC runs with low values 0, 1, 2, 3...) while data runs were order 100,000.  This code will produce a tree of MuonReco::Event objects, just like the decodeRawData script.  After running the MC, you must run the residual (biased only!) specifying the MC configuration file.
   * 
   * \subsection Systematic Uncertainties
   * 
   * \code{.sh}
   * residuals  --conf conf/run<N>.conf --syst <SYST_NAME> [-d]
   * residuals  --conf conf/run<N>.conf --syst <SYST_NAME> [-d] -h
   * resolution --conf conf/run<N>.conf --syst <SYST_NAME> [-d]
   * \endcode
   * 
   * There are several systematic uncertainties that can be applied to the residual and resolution steps.  They are specified with the additional options --syst and -d.  The --syst option takes a string, one of "slew", "t0", "slope" or "intercept".  The -d option changes from a shift up 1 sigma to a shift down 1 sigma.
   * 
   */
  


  /*! \class ArgParser ArgParser.h "MuonReco/ArgParser.h"
   *  \brief Class to parser command line arguments
   * 
   *  Processes key-value pairs of command line arguments as TStrings.
   *  Keys may not have multiple values. 
   *
   *  \author Kevin Nelson                                                                                       
   *          kevin.nelson@cern.ch                                                                               
   *  \date   3 June 2020 
   */
  class ArgParser {
  public:
    ArgParser(int argc, char* argv[]);
    virtual ~ArgParser();

    bool    hasKey    (TString key);
    double  getDouble (TString key);
    int     getInt    (TString key);
    TString getTString(TString key);
    bool    getBool   (TString key);

  private:
    std::map<TString, TString> _map; //< the underlying map data structure
  };
}

#endif
