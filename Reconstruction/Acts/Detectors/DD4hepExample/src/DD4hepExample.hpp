#ifndef DD4hepExample_h
#define DD4hepExample_h

#include <GaudiKernel/Algorithm.h>
#include <Gaudi/Property.h>
#include "DetInterface/IGeomSvc.h"
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Utilities/Logger.hpp"

namespace dd4hep {
    class Detector;
}

// The second algorithm shows how to invoke the services.

class DD4hepExample: public Algorithm {
public:
    DD4hepExample(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

private:
    SmartIF<IGeomSvc> m_geomSvc;
    //dd4hep::Detector* m_dd4hep;
    dd4hep::DetElement m_dd4hep;
    static void dummySort(std::vector<dd4hep::DetElement>& det);
    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
    void propagation();
    double m_envR;
    double m_envZ;
    double m_layerThickness;
    double m_logLevel;

};


#endif
