# Acts

if (Acts_FOUND)
gaudi_add_module(GenericExampleAlg
        SOURCES src/BuildGenericDetector.cpp
		src/GenericDetectorElement.cpp
		src/GenericExample.cpp
        LINK GearSvc
             Gaudi::GaudiAlgLib
             Gaudi::GaudiKernel
             ${GEAR_LIBRARIES}
             ${GSL_LIBRARIES}
             ${LCIO_LIBRARIES}
             DetSegmentation
             DetInterface
             DataHelperLib
             EDM4HEP::edm4hep
             EDM4HEP::edm4hepDict
	     ActsCore 
	     #ActsIdentificationPlugin
             ActsDigitizationPlugin
)

target_include_directories(GenericExampleAlg PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS GenericExampleAlg 
            EXPORT CEPCSWTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
            COMPONENT dev)
endif()
