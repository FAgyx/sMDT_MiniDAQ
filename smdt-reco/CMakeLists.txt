# CMakeLists.txt for smdt package
cmake_minimum_required(VERSION 2.8.10)
project(smdt)


#
# Include the ROOT library
#

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net)

message(${ROOT_INCLUDE_DIRS})
include_directories(${ROOT_INCLUDE_DIRS})

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})


#
# Include the GEANT4 library with QT visualisation and GDML geometry importing
#
set(GEANT4_USE_QT TRUE)
set(GEANT4_USE_GDML ON)
set(XERCESC_ROOT_DIR /cvmfs/geant4.cern.ch/externals/XercesC/3.1.1p2/x86_64-slc6-gcc62-opt)
find_package(Geant4 REQUIRED qt gdml)
include_directories(${Geant4_INCLUDE_DIRS})
include(${Geant4_USE_FILE})
message(${Geant4_INCLUDE_DIRS})

#
# Define which files are headers and source
#

file(GLOB MuonReco_HEADERS ${PROJECT_SOURCE_DIR}/MuonReco/*.h)
file(GLOB MuonReco_SOURCES ${PROJECT_SOURCE_DIR}/src/reco/*.cxx)
file(GLOB MuonSim_HEADERS  ${PROJECT_SOURCE_DIR}/MuonSim/*.h)
file(GLOB MuonSim_SOURCES  ${PROJECT_SOURCE_DIR}/src/sim/*.cxx)

set(recoObjs_HEADERS
MuonReco/RecoObject.h
MuonReco/Cluster.h
MuonReco/Event.h
MuonReco/EventID.h
MuonReco/Geometry.h
MuonReco/Hit.h
MuonReco/Observable.h
MuonReco/Signal.h
MuonReco/Track.h
)

set(recoObjs_SOURCES
src/reco/Cluster.cxx
src/reco/Event.cxx
src/reco/EventID.cxx
src/reco/Geometry.cxx
src/reco/Hit.cxx
src/reco/Observable.cxx
src/reco/Signal.cxx
src/reco/Track.cxx
)


#
# Link this library to ROOT
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


ROOT_GENERATE_DICTIONARY(sMDT_CINT ${recoObjs_HEADERS} LINKDEF src/reco/LinkDef.h)

add_library(MuonReco SHARED ${MuonReco_SOURCES} sMDT_CINT.cxx)

add_library(MuonSim SHARED ${MuonSim_SOURCES} MuonReco)


#
# Create executables for muon reconstruction
#

add_executable(decodeRawData run/DecodeRawData.cxx)
target_link_libraries(decodeRawData MuonReco ${ROOT_LIBRARIES})

add_executable(doT0Fit run/DoT0Fit.cxx)
target_link_libraries(doT0Fit MuonReco ${ROOT_LIBRARIES})

add_executable(runEventDisplay run/RunEventDisplay.cxx)
target_link_libraries(runEventDisplay MuonReco ${ROOT_LIBRARIES})

add_executable(autoCalibration run/AutoCalibration.cxx)
target_link_libraries(autoCalibration MuonReco ${ROOT_LIBRARIES})

add_executable(resolution run/Resolution.cxx)
target_link_libraries(resolution MuonReco ${ROOT_LIBRARIES})

add_executable(twoRTAutoCalibration run/TwoRTAutoCalibration.cxx)
target_link_libraries(twoRTAutoCalibration MuonReco ${ROOT_LIBRARIES})

add_executable(twoRTResolution run/TwoRTResolution.cxx)
target_link_libraries(twoRTResolution MuonReco ${ROOT_LIBRARIES})

#
# create executables for muon simulation
#
if($ENV{USE_VISUALISATION})
  add_executable(showChamberGeometry run/ShowChamberGeometry.cxx)
  target_link_libraries(showChamberGeometry MuonSim MuonReco ${ROOT_LIBRARIES} ${Geant4_LIBRARIES})  
  target_compile_options(showChamberGeometry PRIVATE -Wno-shadow)

  add_executable(runBeam run/RunBeam.cxx)
  target_link_libraries(runBeam MuonSim MuonReco ${ROOT_LIBRARIES} ${Geant4_LIBRARIES})
  target_compile_options(runBeam PRIVATE -Wno-shadow)
endif($ENV{USE_VISUALISATION})

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow")
