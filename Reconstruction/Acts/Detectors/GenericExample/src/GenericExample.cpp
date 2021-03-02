#include "GenericExample.hpp"
#include "BuildGenericDetector.hpp"
#include "GenericDetectorElement.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Utilities/Logger.hpp"
#include <Acts/Material/IMaterialDecorator.hpp>

DECLARE_COMPONENT(GenericExample)

GenericExample::GenericExample(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator) {

}

StatusCode
GenericExample::initialize() {
    StatusCode sc;

    info() << "Retrieving the FirstSvc... " << endmsg;


    return sc;
}

StatusCode
GenericExample::execute() {
    StatusCode sc;

  // --------------------------------------------------------------------------------
    DetectorElement::ContextType nominalContext;

  auto buildLevel = 3;
  // set geometry building logging level                         
  Acts::Logging::Level surfaceLogLevel
	  = Acts::Logging::Level(0);
  Acts::Logging::Level layerLogLevel                             
	  = Acts::Logging::Level(0);
  Acts::Logging::Level volumeLogLevel                            
	  = Acts::Logging::Level(0);

 bool buildProto = "proto";

  std::shared_ptr<const Acts::IMaterialDecorator> mdecorator = nullptr;

 using TrackingGeometryPtr = std::shared_ptr<const Acts::TrackingGeometry>;
  /// Return the generic detector
  TrackingGeometryPtr gGeometry
	  = Generic::buildDetector<DetectorElement>(nominalContext,
			  detectorStore,
			  buildLevel,
			  std::move(mdecorator),
			  buildProto,
			  surfaceLogLevel,
			  layerLogLevel,
			  volumeLogLevel);
  std::cout<<"finish building "<<std::endl;


  return sc;
}

StatusCode
GenericExample::finalize() {
	StatusCode sc;
	return sc;
}


