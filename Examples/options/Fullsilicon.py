#!/usr/bin/env python

import os
print(os.environ["DD4HEP_LIBRARY_PATH"])
import sys

from Gaudi.Configuration import *

##############################################################################
# Random Number Svc
##############################################################################
from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_

# rndmengine = HepRndm__Engine_CLHEP__RanluxEngine_() # The default engine in Gaudi
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_() # The default engine in Geant4
rndmengine.SetSingleton = True
rndmengine.Seeds = [42]

##############################################################################
# Event Data Svc
##############################################################################
from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

##############################################################################
# Geometry Svc
##############################################################################

geometry_option = "cepc_FST2.xml"

if not os.getenv("FULLSILICONROOT"):
    print("Can't find the geometry. Please setup envvar FULLSILICONROOT." )
    sys.exit(-1)

geometry_path = os.path.join(os.getenv("FULLSILICONROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s"%geometry_path)
    sys.exit(-1)

from Configurables import GeomSvc
geosvc = GeomSvc("GeomSvc")
geosvc.compact = geometry_path

#from Configurables import DD4hepExample 
from Configurables import DD4hepPropagationExample

actsalg = DD4hepPropagationExample("DD4hepPropagationExample")
actsalg.envelopeR = 0.1
actsalg.envelopeZ = 0.1
actsalg.layerThickness= 10e-10
actsalg.loglevel = 3 

##############################################################################
## POD I/O
###############################################################################
#from Configurables import PodioOutput
#out = PodioOutput("outputalg")
#out.filename = "a.root"
#out.outputCommands = ["keep *"]

##############################################################################
# NTupleSvc
##############################################################################
from Configurables import NTupleSvc
ntsvc = NTupleSvc("NTupleSvc")
ntsvc.Output = ["MyTuples DATAFILE='result.root' OPT='NEW' TYP='ROOT'"]


# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [actsalg],
                EvtSel = 'NONE',
                EvtMax = 100000,
		ExtSvc = [rndmengine,geosvc,dsvc,ntsvc],
                HistogramPersistency = "ROOT",
                OutputLevel=ERROR
)
