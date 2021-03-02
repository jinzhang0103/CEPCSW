#ifndef GenericExample_h
#define GenericExample_h

#include <GaudiKernel/Algorithm.h>
#include <Gaudi/Property.h>
#include "GenericDetectorElement.hpp"


// The second algorithm shows how to invoke the services.

class GenericExample: public Algorithm {
public:
    GenericExample(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

    using DetectorElement    = Generic::GenericDetectorElement;
    using DetectorElementPtr = std::shared_ptr<DetectorElement>;
    using DetectorStore      = std::vector<std::vector<DetectorElementPtr>>;

    /// The Store of the detector elements (lifetime: job)
    DetectorStore detectorStore;


};


#endif
