###############################################################################
# Package: Demon 
# Based on package: Acts
################################################################################

find_package(DD4hep COMPONENTS DDG4 DDParsers REQUIRED)
# find_package(DD4hep)
find_package(Geant4)
include(${Geant4_USE_FILE})

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${DD4hep_ROOT}/cmake )
include( DD4hep )

find_package(ROOT COMPONENTS MathCore GenVector Geom REQUIRED)

# install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/compact DESTINATION Detector/Demon)

gaudi_add_module(Demon
                 SOURCES src/DemonstratorBarrel_geo.cpp  
		  	 src/DemonstratorBeamPipe_geo.cpp
		 LINK ${DD4hep_COMPONENT_LIBRARIES}
		 ActsCore 
		 ActsPluginDD4hep
		 ROOT::Geom ROOT::GenVector 
)

target_include_directories(Demon PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/src/include
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(Demon)

install(TARGETS Demon
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
