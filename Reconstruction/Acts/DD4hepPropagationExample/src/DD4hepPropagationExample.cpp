#include "DD4hepPropagationExample.hpp"
#include "Acts/Utilities/Logger.hpp"
#include <Acts/Material/IMaterialDecorator.hpp>
#include "Acts/Geometry/CylinderVolumeBuilder.hpp"
#include "Acts/Geometry/CylinderVolumeHelper.hpp"
#include "Acts/Geometry/LayerArrayCreator.hpp"                                                                                  
#include "Acts/Geometry/SurfaceArrayCreator.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingVolumeArrayCreator.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Utilities/BinningType.hpp"

#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include <Acts/MagneticField/ConstantBField.hpp>
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Algebra.hpp" 
#include <Acts/Utilities/Helpers.hpp>


#include "TRandom.h"
#include <iostream>

DECLARE_COMPONENT(DD4hepPropagationExample)

DD4hepPropagationExample::DD4hepPropagationExample(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator) {
	declareProperty("envelopeR", m_envR);
	declareProperty("envelopeZ", m_envZ);
	declareProperty("layerThickness", m_layerThickness);
	declareProperty("loglevel", m_logLevel);
}

StatusCode
DD4hepPropagationExample::initialize() {

	StatusCode sc;

    //  Book N-tuple 1
    NTuplePtr nt1( ntupleSvc(), "MyTuples/Propagation" );
    if ( nt1 ) {
        m_tuple_id = nt1;
    } else {
        m_tuple_id = ntupleSvc()->book( "MyTuples/Propagation", CLID_ColumnWiseTuple, "Row-wise N-Tuple example" );
        if ( m_tuple_id ) {
            m_tuple_id->addItem("stepNum",m_stepNum,0,1000).ignore();
	    m_tuple_id->addItem("volume_id",    m_stepNum, m_volumeID).ignore();
	    m_tuple_id->addItem("boundary_id",  m_stepNum, m_boundaryID).ignore();
	    m_tuple_id->addItem("layer_id",     m_stepNum, m_layerID).ignore();
	    m_tuple_id->addItem("approach_id",  m_stepNum, m_approachID).ignore();
	    m_tuple_id->addItem("sensitive_id", m_stepNum, m_sensitiveID).ignore();
	    m_tuple_id->addItem("g_x", m_stepNum, m_x).ignore();
	    m_tuple_id->addItem("g_y", m_stepNum, m_y).ignore();
	    m_tuple_id->addItem("g_z", m_stepNum, m_z).ignore();
	    m_tuple_id->addItem("d_x", m_stepNum, m_dx).ignore();
	    m_tuple_id->addItem("d_y", m_stepNum, m_dy).ignore();
	    m_tuple_id->addItem("d_z", m_stepNum, m_dz).ignore();
	    m_tuple_id->addItem("type", m_stepNum, m_step_type).ignore();
	    m_tuple_id->addItem("step_acc", m_stepNum, m_step_acc).ignore();
	    m_tuple_id->addItem("step_act", m_stepNum, m_step_act).ignore();
	    m_tuple_id->addItem("step_abt", m_stepNum, m_step_abt).ignore();
	    m_tuple_id->addItem("step_usr", m_stepNum, m_step_usr).ignore();
        } else { // did not manage to book the N tuple....
            error() << "    Cannot book N-tuple:" << long( m_tuple_id ) << endmsg;
            return StatusCode::FAILURE;
        }
    }

	///Get GeomSvc
	m_geomSvc=Gaudi::svcLocator()->service("GeomSvc");
	if (nullptr==m_geomSvc) {
		std::cout<<"Failed to find GeomSvc"<<std::endl;
		return StatusCode::FAILURE;
	}
	///Get Detector
	m_dd4hep = m_geomSvc->lcdd()->world();

	info() << "Retrieving the FirstSvc... " << endmsg;

	BuildTrackingGeo();

    return StatusCode::SUCCESS;
}

StatusCode
DD4hepPropagationExample::execute() {
	info() << "Start in the FirstSvc... " << endmsg;

	//propagation();

	std::vector<Acts::detail::Step> stepCollection = propagation();
	m_stepNum = stepCollection.size();
	// loop over the step vector of each test propagation in this
	int istep = 0;
  for (auto& step : stepCollection) {
	  // loop over single steps
      // the identification of the step
      Acts::GeometryIdentifier::Value volumeID = 0;
      Acts::GeometryIdentifier::Value boundaryID = 0;
      Acts::GeometryIdentifier::Value layerID = 0;
      Acts::GeometryIdentifier::Value approachID = 0;
      Acts::GeometryIdentifier::Value sensitiveID = 0;
      // get the identification from the surface first
      if (step.surface) {
        auto geoID = step.surface->geometryId();
        volumeID   = geoID.volume();
        boundaryID = geoID.boundary();
        layerID    = geoID.layer();
        approachID = geoID.approach();
        sensitiveID = geoID.sensitive();
      }
      // a current volume overwrites the surface tagged one
      if (step.volume) {
        volumeID = step.volume->geometryId().volume();
      }
      // now fill
      m_sensitiveID[istep]=sensitiveID;
      m_approachID[istep]=approachID;
      m_layerID[istep]=layerID;
      m_boundaryID[istep]=boundaryID;
      m_volumeID[istep]=volumeID;

      // kinematic information
      m_x[istep]=step.position.x();
      m_y[istep]=step.position.y();
      m_z[istep]=step.position.z();
      auto direction = step.momentum.normalized();
      m_dx[istep]=direction.x();
      m_dy[istep]=direction.y();
      m_dz[istep]=direction.z();

      double accuracy = step.stepSize.value(Acts::ConstrainedStep::accuracy);
      double actor = step.stepSize.value(Acts::ConstrainedStep::actor);
      double aborter = step.stepSize.value(Acts::ConstrainedStep::aborter);
      double user = step.stepSize.value(Acts::ConstrainedStep::user);
      double act2 = actor * actor;
      double acc2 = accuracy * accuracy;
      double abo2 = aborter * aborter;
      double usr2 = user * user;

      // todo - fold with direction
      if (act2 < acc2 && act2 < abo2 && act2 < usr2) {
        m_step_type[istep]=0;
      } else if (acc2 < abo2 && acc2 < usr2) {
        m_step_type[istep]=1;
      } else if (abo2 < usr2) {
        m_step_type[istep]=2;
      } else {
        m_step_type[istep]=3;
      }

      // step size information
      m_step_acc[istep]=accuracy;
      m_step_act[istep]=actor;
      m_step_abt[istep]=aborter;
      m_step_usr[istep]=user;
      istep++;
    }
	m_tuple_id->write();
	return StatusCode::SUCCESS;
}

StatusCode
DD4hepPropagationExample::finalize() {
	return StatusCode::SUCCESS;
}

	void
DD4hepPropagationExample::dummySort(std::vector<dd4hep::DetElement>& det)
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

std::vector<Acts::detail::Step> DD4hepPropagationExample::propagation(){

	using namespace Acts;
	using namespace Acts::UnitLiterals;
	double phi = gRandom->Uniform(-M_PI,M_PI);
	double eta= gRandom->Uniform(-4,4);
	double theta = 2 * atan(exp(-eta));
	double pT = gRandom->Uniform(1_GeV,10_GeV);
	double dcharge = 1;
	double p = pT / sin(theta);
	double q = dcharge;
	double time = 0;

	using BFieldType = ConstantBField;
	using EigenStepperType = EigenStepper<BFieldType>;
	using EigenPropagatorType = Propagator<EigenStepperType, Navigator>;
	using Covariance = BoundSymMatrix;
	using SteppingLogger = Acts::detail::SteppingLogger;

	using EndOfWorld = Acts::EndOfWorldReached;
	using ActionList = Acts::ActionList<SteppingLogger, MaterialInteractor>;
	using AbortList = Acts::AbortList<EndOfWorld>;
	using PropagatorOptions =
		Acts::DenseStepperPropagatorOptions<ActionList,AbortList>;

	// Get a Navigator
	Navigator navigator(m_trackingGeometry);

	BFieldType bField(0, 0, 2_T);
	EigenStepperType estepper(bField);
	EigenPropagatorType epropagator(std::move(estepper), std::move(navigator));

	Covariance cov;
	// take some major correlations (off-diagonals)
	cov << 10_mm, 0, 0.123, 0, 0.5, 0, 0, 10_mm, 0, 0.162, 0, 0, 0.123, 0, 0.1, 0,
	    0, 0, 0, 0.162, 0, 0.1, 0, 0, 0.5, 0, 0, 0, 1. / (10_GeV), 0, 0, 0, 0, 0,
	    0, 0;
	CurvilinearTrackParameters start(Vector4(0, 0, 0, time), phi, theta, p, q,cov);
	GeometryContext tgContext = GeometryContext();
	MagneticFieldContext mfContext = MagneticFieldContext();

	//PropagatorOptions<> options(tgContext, mfContext,getDummyLogger());
	PropagatorOptions options(tgContext, mfContext, getDummyLogger());
	///options.maxStepSize = 3_m;
	options.pathLimit = 250_cm;
	options.maxSteps= 10000;

	const auto& resultValue = epropagator.propagate(start, options).value();
	auto steppingResults =
		resultValue.template get<Acts::detail::SteppingLogger::result_type>();
	return steppingResults.steps;

}
void DD4hepPropagationExample::BuildTrackingGeo(){
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
					m_envR * Acts::UnitConstants::mm,
					m_envZ * Acts::UnitConstants::mm,
					m_layerThickness,
					sortDetectors,
					gctx,
					mdecorator));
}


