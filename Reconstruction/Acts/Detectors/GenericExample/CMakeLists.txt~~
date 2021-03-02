add_library(
  ACTFWGenericDetector SHARED
  src/BuildGenericDetector.cpp
  src/GenericDetector.cpp
  src/GenericDetectorElement.cpp)
target_include_directories(
  ACTFWGenericDetector
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(
  ACTFWGenericDetector
  PUBLIC ActsCore ActsIdentificationPlugin ActsDigitizationPlugin)
target_link_libraries(
  ACTFWGenericDetector
  PUBLIC ACTFramework ACTFWDetectorsCommon)

install(
  TARGETS ACTFWGenericDetector
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
