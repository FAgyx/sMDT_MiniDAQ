# sMDT reconstruction and simulation codebase

This code is designed to reconstruct raw data files from the University of Michigan sMDT test chamber, as well as simulate the test chamber in Geant4.

It produces diagnostic plots of raw data, fits T0 from drift time spectra, auto-calibrates an RT function,
and produces a final resolution measurement of the chamber.

## Cloning this repository

Simply clone the repository with your preffered method (HTTPS, SSH, etc.).  i.e. run

```bash
git clone ssh://git@gitlab.cern.ch:7999/<YOUR CERN USERNAME>/smdt-reco.git
```

This package is standalone, so it does not run off of an AnalysisBase or Athena build.  The only dependencies are CMake, ROOT and Geant4, which are avaliable on cvmfs.

## Setting up and building the code

The code is compiled using cmake.  On a cvmfs mounted machine, simply source the setup script in the top level directory

```
cd smdt_reco
source setup.sh
```

This script will setup ROOT, CMake and Geant4, set environment variables, and create aliases to binaries that will be stored in your build area

Next, build the code

```
mkdir build
cd build
cmake ..
make
```

## Acquiring raw data

Raw data files are not tracked by git (for obvious reasons!).  They are hosted on /eos at /eos/user/k/kenelson/smdt-reco-files/raw.  They should be stored in a raw data file directory under the top level directory (on the same level as this README).  To download run

```
cd $SMDT_DIR
mkdir raw
cd raw
scp <YOUR_USERNAME>@lxplus.cern.ch:/eos/user/k/kenelson/smdt-reco-files/raw/<DESIRED_RAW_FILE> .
```

There are several raw data files on the order of a GB in this area.  At a minimum download the file Rt_BMG_6_1.dat, which is a txt file initialization of the r(t) function.

## Running the code

After building there be several executables in your build directory.  The setup script creates aliases for these.  For instance, running the command 


```
decodeRawData
```

from the top-level directory will create an ouput area with plots showing diagnostics of the raw data!

More documentation on running other macros is forthcoming.


## Further documentation

A dOxygen generated documentation is avaliable at http://cern.ch/kenelson