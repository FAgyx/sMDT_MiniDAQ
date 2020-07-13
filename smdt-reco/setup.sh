# set up ROOT

export SMDT_DIR=`pwd`
export ALRB_rootVersion=6.10.06-x86_64-slc6-gcc62-opt
LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
export ATLAS_LOCAL_ROOT_BASE=$LOCAL_ROOT_BASE
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
source ${ATLAS_LOCAL_ROOT_BASE}/packageSetups/atlasLocalROOTSetup.sh --rootVersion ${ALRB_rootVersion}


# set up CMAKE

lsetup cmake
lsetup "gcc gcc620_x86_64_slc6"


# set up GEANT4

source /cvmfs/sft.cern.ch/lcg/contrib/gcc/6.2.0/x86_64-slc6-gcc62-opt/setup.sh                  # set up compiler
source /cvmfs/geant4.cern.ch/geant4/10.1.p03/x86_64-slc6-gcc62-opt/CMake-setup.sh               # set up environment for Geant4
#source /cvmfs/sft.cern.ch/lcg/releases/XercesC/3.1.3-b3bf1/x86_64-slc6-gcc62-opt/XercesC-env.sh # set up GDML reader
export CXX=`which g++`                                                                          # tell CMake about compiler used 
export CC=`which gcc`
export G4INC='/cvmfs/geant4.cern.ch/geant4/10.1.p03/x86_64-slc6-gcc62-opt/include/Geant4'
export USE_VISUALISATION=1

# define useful aliases for running binaries from the top level directory

alias decodeRawData='./build/decodeRawData'
alias doT0Fit='./build/doT0Fit'
alias runEventDisplay='./build/runEventDisplay'
alias autoCalibration='./build/autoCalibration'
alias twoRTAutoCalibration='./build/twoRTAutoCalibration'
alias resolution='./build/resolution'
alias twoRTResolution='./build/twoRTResolution'

# binaries for simulation
alias showChamberGeometry='./build/showChamberGeometry'
alias runBeam='./build/runBeam'
