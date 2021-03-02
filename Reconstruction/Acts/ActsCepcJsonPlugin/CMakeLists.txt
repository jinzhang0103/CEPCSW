file(GLOB_RECURSE src_files "src/*.cpp" "include/*.hpp") 

if (nlohmann_json_FOUND)
gaudi_add_module(ActsCepcJsonPluginAlg
        SOURCES ${src_files}
        LINK GearSvc
             Gaudi::GaudiAlgLib
             Gaudi::GaudiKernel
	     ActsCore
	     ActsPluginJson
	     nlohmann_json::nlohmann_json
)

target_include_directories(
  ActsCepcJsonPluginAlg
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(
  TARGETS ActsCepcJsonPluginAlg
  EXPORT  CEPCSWTargets 
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shilib
  COMPONENT dev
)
endif()
