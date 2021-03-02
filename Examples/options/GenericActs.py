#!/usr/bin/env python

from Gaudi.Configuration import *

from Configurables import GenericExample 

actsalg = GenericExample("GenericExampleAlg")

# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [actsalg],
                EvtSel = 'NONE',
                EvtMax = 10,
)
