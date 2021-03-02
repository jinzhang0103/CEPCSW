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
# Geometry Svc
##############################################################################

geometry_option = "Demonstrator.xml"

if not os.getenv("DEMONROOT"):
    print("Can't find the geometry. Please setup envvar DEMONROOT." )
    sys.exit(-1)

geometry_path = os.path.join(os.getenv("DEMONROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s"%geometry_path)
    sys.exit(-1)

from Configurables import GeomSvc
geosvc = GeomSvc("GeomSvc")
geosvc.compact = geometry_path

from Configurables import DD4hepExample 

actsalg = DD4hepExample("DD4hepPropagationAlg")
actsalg.loglevel = 3 
actsalg.envelopeR= 1 
actsalg.envelopeZ= 1 
actsalg.layerThickness= 10e-10

# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [actsalg],
                EvtSel = 'NONE',
                EvtMax = 1,
		ExtSvc = [rndmengine, geosvc],
)
