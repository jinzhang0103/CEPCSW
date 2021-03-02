# Acts

if (Acts_FOUND)
gaudi_add_module(DD4hepPropagationExample
        SOURCES src/DD4hepPropagationExample.cpp
        LINK GearSvc
             Gaudi::GaudiAlgLib
             Gaudi::GaudiKernel
             ${GEAR_LIBRARIES}
             ${GSL_LIBRARIES}
             ${LCIO_LIBRARIES}
	     ${DD4hep_COMPONENT_LIBRARIES}
             DetSegmentation
             DetInterface
             DataHelperLib
             EDM4HEP::edm4hep
             EDM4HEP::edm4hepDict
	     ActsCore 
	     #ActsIdentificationPlugin
             ActsPluginDigitization
	     ActsPluginDD4hep
	     ROOT::Geom ROOT::GenVector 
	     k4FWCore::k4FWCore
	     ${podio_LIBRARIES} podio::podioRootIO
)

target_include_directories(DD4hepPropagationExample PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS DD4hepPropagationExample 
            EXPORT CEPCSWTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
            COMPONENT dev)
endif()
