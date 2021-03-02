#ifndef DD4hepPropagationExample_h
#define DD4hepPropagationExample_h

#include <GaudiKernel/Algorithm.h>
#include <Gaudi/Property.h>
#include "DetInterface/IGeomSvc.h"
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "GaudiKernel/NTuple.h"

#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include <Acts/Propagator/StraightLineStepper.hpp>
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/ActionList.hpp"
#include "Acts/Propagator/DenseEnvironmentExtension.hpp"
#include "Acts/Propagator/StandardAborters.hpp"

namespace dd4hep {
    class Detector;
}

// The second algorithm shows how to invoke the services.

class DD4hepPropagationExample: public Algorithm {
public:
    DD4hepPropagationExample(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

private:
    SmartIF<IGeomSvc> m_geomSvc;
    //dd4hep::Detector* m_dd4hep;
    dd4hep::DetElement m_dd4hep;
    static void dummySort(std::vector<dd4hep::DetElement>& det);
    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
    void BuildTrackingGeo();
    std::vector<Acts::detail::Step> propagation();
    double m_envR;
    double m_envZ;
    double m_layerThickness;
    double m_logLevel;

    /// tuples
    NTuple::Tuple*  m_tuple_id;
    NTuple::Item<int> m_evt;
    NTuple::Item<long> m_stepNum;
    NTuple::Array<double> m_volumeID;
    NTuple::Array<double> m_boundaryID;
    NTuple::Array<double> m_layerID;
    NTuple::Array<double> m_approachID;
    NTuple::Array<double> m_sensitiveID;
    NTuple::Array<double> m_x;
    NTuple::Array<double> m_y;
    NTuple::Array<double> m_z;
    NTuple::Array<double> m_dx;
    NTuple::Array<double> m_dy;
    NTuple::Array<double> m_dz;
    NTuple::Array<double> m_step_type;
    NTuple::Array<double> m_step_acc;
    NTuple::Array<double> m_step_act;
    NTuple::Array<double> m_step_abt;
    NTuple::Array<double> m_step_usr;

};


#endif
