#include "DD4hepExample.hpp"
#include <Acts/Material/IMaterialDecorator.hpp>
#include "Acts/Geometry/CylinderVolumeBuilder.hpp"
#include "Acts/Geometry/CylinderVolumeHelper.hpp"
#include "Acts/Geometry/LayerArrayCreator.hpp"                                                                                  
#include "Acts/Geometry/SurfaceArrayCreator.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingVolumeArrayCreator.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"

#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include <Acts/MagneticField/ConstantBField.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/StraightLineStepper.hpp>
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Units.hpp"

#include <Acts/Plugins/Json/JsonGeometryConverter.hpp>
#include <Acts/Plugins/Json/JsonMaterialDecorator.hpp>
#include "JsonSurfacesWriter.hpp"

DECLARE_COMPONENT(DD4hepExample)

DD4hepExample::DD4hepExample(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator) {
	declareProperty("envelopeR", m_envR);
	declareProperty("envelopeZ", m_envZ);
	declareProperty("layerThickness", m_layerThickness);
	declareProperty("loglevel", m_logLevel);
}

StatusCode
DD4hepExample::initialize() {

	StatusCode sc;
	///Get GeomSvc
	m_geomSvc=Gaudi::svcLocator()->service("GeomSvc");
	if (nullptr==m_geomSvc) {
		std::cout<<"Failed to find GeomSvc"<<std::endl;
		return StatusCode::FAILURE;
	}
	///Get Detector
	m_dd4hep = m_geomSvc->lcdd()->world();

	info() << "Retrieving the FirstSvc... " << endmsg;


	return sc;
}

StatusCode
DD4hepExample::execute() {
	info() << "Start in the FirstSvc... " << endmsg;
	StatusCode sc;

	//Empty context
	Acts::GeometryContext gctx;

	//Config
	/// Log level for the geometry service.
	//Acts::Logging::Level logLevel = Acts::Logging::Level::VERBOSE;
	Acts::Logging::Level logLevel = Acts::Logging::Level(m_logLevel);
	/// Binningtype in phi
	Acts::BinningType bTypePhi = Acts::equidistant;
	/// Binningtype in r
	Acts::BinningType bTypeR = Acts::arbitrary;
	/// Binningtype in z
	Acts::BinningType bTypeZ = Acts::equidistant;
	//double envelopeR = 0.1 * Acts::units::_mm;
	//double envelopeZ = 0.1 * Acts::units::_mm;
	//double defaultLayerThickness = 10e-10;  
	//Material decorator
	std::shared_ptr<const Acts::IMaterialDecorator> mdecorator = nullptr;
	//Sort function
	std::function<void(std::vector<dd4hep::DetElement>& detectors)>
		sortDetectors = dummySort;

	// Set the tracking geometry
	m_trackingGeometry
		= std::move(Acts::convertDD4hepDetector(m_dd4hep,
					logLevel,
					bTypePhi,
					bTypeR,
					bTypeZ,
					m_envR,
					m_envZ,
					m_layerThickness,
					sortDetectors,
					gctx,
					mdecorator));

	//write Json
	JsonSurfacesWriter::Config sJsonWriterConfig;
	sJsonWriterConfig.trackingGeometry = m_trackingGeometry;
	sJsonWriterConfig.outputDir = "/besfs5/users/zhangjin/cepc/CEPCSW/output";
	sJsonWriterConfig.writePerEvent = true;
	auto sJsonWriter = std::make_shared<JsonSurfacesWriter>(
			sJsonWriterConfig, logLevel);
	// Write the tracking geometry object
	sJsonWriter->write();


	//propagation();
	return sc;
}

StatusCode
DD4hepExample::finalize() {
	StatusCode sc;
	return sc;
}

	void
DD4hepExample::dummySort(std::vector<dd4hep::DetElement>& det)
{
	std::vector<dd4hep::DetElement> tracker;
	for (auto& detElement : det) {                                                                                              
		tracker.push_back(detElement);
	}
	sort(tracker.begin(),
			tracker.end(),
			[](const dd4hep::DetElement& a, const dd4hep::DetElement& b) {
			return (a.id() < b.id());
			});
	det.clear();
	det = tracker;
}

/*
void DD4hepExample::propagation(){
	using namespace Acts;
	using namespace Acts::UnitLiterals;
	using BFieldType = ConstantBField;
	using EigenStepperType = EigenStepper<BFieldType>;
	using EigenPropagatorType = Propagator<EigenStepperType, Navigator>;
	using Covariance = BoundSymMatrix;

	// Get a Navigator
	Acts::Navigator navigator(m_trackingGeometry);

	BFieldType bField(0, 0, 2_T);
	EigenStepperType estepper(bField);
	EigenPropagatorType epropagator(std::move(estepper), std::move(navigator));

	double phi = 5;
	double theta = 10;
	double pT = 10;
	double dcharge = 1;

	double x = 0;
	double y = 0;
	double z = 0;
	double px = pT * cos(phi);
	double py = pT * sin(phi);
	double pz = pT / tan(theta);
	double q = dcharge;
	double time = 0;
	Vector3D pos(x, y, z);
	Vector3D mom(px, py, pz);
	CurvilinearParameters start(std::nullopt, pos, mom, q, time);
	GeometryContext tgContext = GeometryContext();
	MagneticFieldContext mfContext = MagneticFieldContext();

	PropagatorOptions<> options(tgContext, mfContext);
	options.maxStepSize = 10_cm;
	options.pathLimit = 25_cm;

	auto value = epropagator.propagate(start, options);
}
*/


