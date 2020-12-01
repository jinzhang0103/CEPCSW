#include "GenfitFitter.h"
#include "GenfitTrack.h"
#include "GenfitMsg.h"
#include "GenfitField.h"
#include "GenfitMaterialInterface.h"

//Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ISvcLocator.h"

//External
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Detector.h"

//genfit
#include <Track.h>
#include <Exception.h>
#include <FieldManager.h>
#include <TGeoMaterialInterface.h>
#include <TGeoManager.h>
#include <MaterialEffects.h>
#include <MeasuredStateOnPlane.h>
#include <KalmanFitter.h>
#include <KalmanFitterRefTrack.h>
#include <StateOnPlane.h>
#include <DAF.h>
#include <AbsKalmanFitter.h>
#include <KalmanFitterInfo.h>

//ROOT
#include <TVector3.h>
#include <TGeoManager.h>

//STL
#include <iostream>
#include <string>


GenfitFitter::~GenfitFitter(){
    delete m_absKalman;
}

GenfitFitter::GenfitFitter(const char* type, const char* name):
    m_absKalman(nullptr)
    ,m_genfitField(nullptr)
    ,m_geoMaterial(nullptr)
    ,m_fitterType(type)
    ,m_name(name)
    ,m_minIterations(4)
    ,m_maxIterations(10)
    ,m_deltaPval(1e-3)
    ,m_relChi2Change(0.2)
    ,m_blowUpFactor(1e3)
    ,m_resetOffDiagonals(true)
    ,m_blowUpMaxVal(1.e6)
    ,m_multipleMeasurementHandling(genfit::unweightedClosestToPredictionWire)
    ,m_maxFailedHits(-1)
    ,m_deltaWeight(1e-3)
    ,m_annealingBetaStart(100)
    ,m_annealingBetaStop(0.01)
    ,m_annealingNSteps(0.01)
    ,m_noEffects(false)
    ,m_energyLossBetheBloch(true)
    ,m_noiseBetheBloch(true)
    ,m_noiseCoulomb(true)
    ,m_energyLossBrems(false)
    ,m_noiseBrems(false)
    ,m_ignoreBoundariesBetweenEqualMaterials(true)
    ,m_mscModelName("GEANE")
    ,m_debug(0)
    ,m_hist(0)
{
    /// Initialize genfit fitter
    init();
}

void GenfitFitter::setField(const GenfitField* field)
{
    if(nullptr==m_genfitField) m_genfitField=field;
}

/// Set geometry for material, use geometry from IOADatabase
void GenfitFitter::setGeoMaterial(const dd4hep::Detector* dd4hepGeo)
{
    if(nullptr==m_geoMaterial){
        m_geoMaterial=GenfitMaterialInterface::getInstance(dd4hepGeo);
    }
}

/// initialize genfit fitter
int GenfitFitter::init(bool deleteOldFitter)
{
    if(deleteOldFitter && m_absKalman) delete m_absKalman;

    GenfitMsg::get()<<MSG::DEBUG<<"Initialize GenfitFitter with "
        <<m_fitterType<<endmsg;

    if (m_fitterType=="DAFRef") {
        m_absKalman = new genfit::DAF(true,getDeltaPval(),
                getConvergenceDeltaWeight());
    }
    else if (m_fitterType=="DAF") {
        m_absKalman = new genfit::DAF(false,getDeltaPval(),
                getConvergenceDeltaWeight());
    }
    else if (m_fitterType=="KalmanFitter") {
        m_absKalman = new genfit::KalmanFitter(getMaxIterations());
    }
    else if (m_fitterType=="KalmanFitterRefTrack") {
        m_absKalman = new genfit::KalmanFitterRefTrack(getMaxIterations());
    }
    else {
        m_absKalman = nullptr;
        GenfitMsg::get()<< MSG::DEBUG<<"Fitter type is invalid:"
            <<m_fitterType<<endmsg;
        return -1;
    }
    GenfitMsg::get()<<MSG::DEBUG<<"Fitter type is "<<m_fitterType<<endmsg;
std::cout<<__FILE__<<"   "<<__LINE__<<m_absKalman<<std::endl;
    m_absKalman->setDebugLvl(m_debug);
std::cout<<__FILE__<<"   "<<__LINE__<<std::endl;

    return 0;
}

/// Fit a track from a candidate track
int GenfitFitter::processTrackWithRep(GenfitTrack* track,int repID,bool resort)
{
    GenfitMsg::get()<<MSG::DEBUG<< "In ProcessTrackWithRep rep "<<repID<<endmsg;
    if(getDebug()>2) print("");

    if(getDebug()>0){
        GenfitMsg::get()<<MSG::DEBUG<<"Print track seed "<<endmsg;
        track->getTrack()->getStateSeed().Print();
    }
    /// Do the fitting
    try{
        m_absKalman->processTrackWithRep(track->getTrack(), track->getRep(repID),
                resort);
    }catch(genfit::Exception& e){
        GenfitMsg::get()<<MSG::DEBUG<<"Genfit exception caught "<<endmsg;
        e.what();
        return false;
    }
    GenfitMsg::get()<<MSG::DEBUG<<"End of ProcessTrackWithRep"<<endmsg;
    return true;
} // End of ProcessTrackWithRep

/// Fit a track from a candidate track
int GenfitFitter::processTrack(GenfitTrack* track, bool resort)
{
    GenfitMsg::get()<<MSG::DEBUG<<"In ProcessTrack"<<endmsg;
    if(getDebug()>2) print("");

    /// Do the fitting
    try{
        m_absKalman->processTrack(track->getTrack(),resort);
    }catch(genfit::Exception& e){
        GenfitMsg::get()<<MSG::DEBUG<<"Genfit exception caught "<<endmsg;
        e.what();
        return false;
    }
    GenfitMsg::get()<<MSG::DEBUG<<"End of ProcessTrack"<<endmsg;
    return true;
} // End of ProcessTrack


void GenfitFitter::setFitterType(const char* val)
{
    std::string oldFitterType=m_fitterType;
    m_fitterType = val;
    GenfitMsg::get()<<MSG::DEBUG<<"Fitter type is "<<m_fitterType<<endmsg;
    init(oldFitterType==val);
}

/// Extrapolate track to the cloest point of approach(POCA) to the wire of hit,
/// return StateOnPlane of this POCA
/// inputs
///  pos,mom ... position & momentum at starting point, unit= [mm]&[GeV/c]
///              (converted to [cm]&[GeV/c] inside this function)
///  hit ... destination
/// outputs poca [mm] (converted from [cm] in this function) ,global
double GenfitFitter::extrapolateToHit( TVector3& poca, TVector3& pocaDir,
        TVector3& pocaOnWire, double& doca, const GenfitTrack* track,
        TVector3 pos, TVector3 mom,
        TVector3 end0,//one end of the hit wire
        TVector3 end1,//the orhter end of the hit wire
        int debug,
        int repID,
        bool stopAtBoundary,
        bool calcJacobianNoise)
{

    return track->extrapolateToHit(poca,pocaDir,pocaOnWire,doca,pos,mom,end0,
            end1,debug,repID,stopAtBoundary,calcJacobianNoise);
}//end of ExtrapolateToHit


/// Extrapolate the track to the cyliner at fixed raidus
/// position & momentum as starting point
/// position and momentum at global coordinate in dd4hepUnit
/// return trackLength in dd4hepUnit
    double
GenfitFitter::extrapolateToCylinder(TVector3& pos, TVector3& mom,
        GenfitTrack* track, double radius, const TVector3 linePoint,
        const TVector3 lineDirection, int hitID, int repID,
        bool stopAtBoundary, bool calcJacobianNoise)
{
    double trackLength(1e9*dd4hep::cm);
    if(!track->getFitStatus(repID)->isFitted()) return trackLength;
    try{
        // get track rep
        genfit::AbsTrackRep* rep = track->getRep(repID);
        if(nullptr == rep) {
            GenfitMsg::get()<<MSG::DEBUG<<"In ExtrapolateToCylinder rep is null"
                <<endmsg;
            return trackLength*dd4hep::cm;
        }

        // get track point with fitter info
        genfit::TrackPoint* tp =
            track->getTrack()->getPointWithFitterInfo(hitID,rep);
        if(nullptr == tp) {
            GenfitMsg::get()<<MSG::DEBUG<<
                "In ExtrapolateToCylinder TrackPoint is null"<<endmsg;
            return trackLength*dd4hep::cm;
        }

        // get fitted state on plane of this track point
        genfit::KalmanFittedStateOnPlane* state =
            static_cast<genfit::KalmanFitterInfo*>(
                    tp->getFitterInfo(rep))->getBackwardUpdate();

        if(nullptr == state){
            GenfitMsg::get()<<MSG::DEBUG<<"In ExtrapolateToCylinder "
                <<"no KalmanFittedStateOnPlane in backwardUpdate"<<endmsg;
            return trackLength*dd4hep::cm;
        }
        rep->setPosMom(*state, pos*(1/dd4hep::cm), mom*(1/dd4hep::GeV));

        /// extrapolate
        trackLength = rep->extrapolateToCylinder(*state,
                radius/dd4hep::cm, linePoint*(1/dd4hep::cm), lineDirection,
                stopAtBoundary, calcJacobianNoise);
        // get pos&mom at extrapolated point on the cylinder
        rep->getPosMom(*state,pos,mom);//FIXME exception exist
        pos = pos*dd4hep::cm;
        mom = mom*dd4hep::GeV;
    } catch(genfit::Exception& e){
        GenfitMsg::get() << MSG::ERROR
            <<"Exception in GenfitFitter::extrapolateToCylinder "
            << e.what()<<endmsg;
        trackLength = 1e9*dd4hep::cm;
    }
    return trackLength*dd4hep::cm;
}

double GenfitFitter::extrapolateToPoint(TVector3& pos, TVector3& mom,
        const GenfitTrack* track,
        const TVector3& point,
        int repID,// same with pidType
        bool stopAtBoundary,
        bool calcJacobianNoise) const
{
    double trackLength(1e9*dd4hep::cm);
    if(!track->getFitStatus(repID)->isFitted()) return trackLength;
    try{
        // get track rep
        genfit::AbsTrackRep* rep = track->getRep(repID);
        if(nullptr == rep) {
            GenfitMsg::get()<<MSG::DEBUG<<"In ExtrapolateToPoint rep "
                <<repID<<" not exist!"<<endmsg;
            return trackLength*dd4hep::cm;
        }

        /// extrapolate to point
        //genfit::StateOnPlane state(*(&(track->getTrack()->getFittedState(0,rep))));

        // get track point with fitter info
        genfit::TrackPoint* tp =
            track->getTrack()->getPointWithFitterInfo(0,rep);
        if(nullptr == tp) {
            GenfitMsg::get()<<MSG::DEBUG<<
                "In ExtrapolateToPoint TrackPoint is null"<<endmsg;
            return trackLength*dd4hep::cm;
        }

        // get fitted state on plane of this track point
        genfit::KalmanFittedStateOnPlane* state =
            static_cast<genfit::KalmanFitterInfo*>(
                    tp->getFitterInfo(rep))->getBackwardUpdate();

        if(nullptr == state) {
            GenfitMsg::get()<<MSG::DEBUG<<
                "In ExtrapolateToPoint KalmanFittedStateOnPlane is null"<<endmsg;
            return trackLength*dd4hep::cm;
        }
        trackLength = rep->extrapolateToPoint(*state,
                point*(1/dd4hep::cm),stopAtBoundary, calcJacobianNoise);
        rep->getPosMom(*state,pos,mom);//FIXME exception exist
        pos = pos*dd4hep::cm;
        mom = mom*dd4hep::GeV;
    } catch(genfit::Exception& e){
        GenfitMsg::get() << MSG::ERROR
            <<"Exception in GenfitFitter::extrapolateToPoint"
            << e.what()<<endmsg;
        trackLength = 1e9*dd4hep::cm;
    }
    return trackLength*dd4hep::cm;
}//end of ExtrapolateToPoint

GenfitFitter& GenfitFitter::operator=(
        const GenfitFitter& r)
{
    m_fitterType          = r.m_fitterType;
    m_minIterations       = r.m_minIterations;
    m_maxIterations       = r.m_maxIterations;
    m_deltaPval           = r.m_deltaPval;
    m_relChi2Change       = r.m_relChi2Change;
    m_blowUpFactor        = r.m_blowUpFactor;
    m_resetOffDiagonals   = r.m_resetOffDiagonals;
    m_blowUpMaxVal        = r.m_blowUpMaxVal;
    m_multipleMeasurementHandling = r.m_multipleMeasurementHandling;
    m_maxFailedHits       = r.m_maxFailedHits;
    m_annealingNSteps     = r.m_annealingNSteps;
    m_deltaWeight         = r.m_deltaWeight;
    m_annealingBetaStart  = r.m_annealingBetaStart;
    m_annealingBetaStop   = r.m_annealingBetaStop;
    m_noEffects           = r.m_noEffects;
    m_energyLossBetheBloch= r.m_energyLossBetheBloch;
    m_noiseBetheBloch     = r.m_noiseBetheBloch;
    m_noiseCoulomb        = r.m_noiseCoulomb;
    m_energyLossBrems     = r.m_energyLossBrems;
    m_noiseBrems          = r.m_noiseBrems;
    m_ignoreBoundariesBetweenEqualMaterials
        = r.m_ignoreBoundariesBetweenEqualMaterials;
    m_mscModelName        = r.m_mscModelName;
    m_debug               = r.m_debug;
    m_hist                = r.m_hist;
    return *this;
}

void GenfitFitter::print(const char* name)
{

    if(0==strlen(name)) name=m_name;
    //TODO print all fitting parameters
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" GenfitFitter Global Parameters:"<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" Fitter type          = " << m_fitterType<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" Fitting Parameters:"<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" MinIteration         = " << m_minIterations<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" MaxIteration         = " << m_maxIterations<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_deltaPval           = " << m_deltaPval<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" Material Effect Parameters:"<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_noEffects           = " << m_noEffects<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_energyLossBetheBloch= " << m_energyLossBetheBloch<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_noiseBetheBloch     = " << m_noiseBetheBloch<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_noiseCoulomb        = " << m_noiseCoulomb<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_energyLossBrems     = " << m_energyLossBrems<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_noiseBrems          = " << m_noiseBrems<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_ignoreBoundariesBetweenEqualMaterials= "
        << m_ignoreBoundariesBetweenEqualMaterials<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_mscModelName        = " << m_mscModelName<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_debug               = " << m_debug<<endmsg;
    GenfitMsg::get()<<MSG::DEBUG<<name
        <<" m_hist                 = " << m_hist<<endmsg;

    if(m_fitterType=="DAF"||m_fitterType=="DAFRef"){
        genfit::DAF* daf = getDAF();

        if(nullptr != daf){
            std::ostringstream sBetas;
            std::vector<double> betas = daf->getBetas();
            for (unsigned int i=0; i<betas.size(); ++i) {
                sBetas<<betas.at(i)<<",";
            }
            GenfitMsg::get()<<MSG::DEBUG<<" print "<<betas.size()<<
                " betas: %s "<<sBetas.str()<<endmsg;
            GenfitMsg::get()<<MSG::DEBUG<<m_name
                << " m_deltaWeight         = " << m_deltaWeight<<endmsg;
            GenfitMsg::get()<<MSG::DEBUG<<" DAF maxIterations= "
                << daf->getMaxIterations()<<endmsg;
            GenfitMsg::get()<<MSG::DEBUG<<" DAF minIterations= "
                << daf->getMinIterations()<<endmsg;
            GenfitMsg::get()<<MSG::DEBUG<<" DAF deltaPval= "
                << daf->getDeltaPval()<<endmsg;
        }
    }
    genfit::KalmanFitterRefTrack* ref = getKalRef();
    if(nullptr != ref){
        std::ostringstream sBetas;
        GenfitMsg::get()<<MSG::DEBUG<<" DAF maxIterations= "
            << ref->getMaxIterations()<<endmsg;
        GenfitMsg::get()<<MSG::DEBUG<<" DAF minIterations= "
            << ref->getMinIterations()<<endmsg;
        GenfitMsg::get()<<MSG::DEBUG<<" DAF deltaPval= "
            << ref->getDeltaPval()<<endmsg;
    }
}

///Setters of AbsKalmanFitter
void GenfitFitter::setMinIterations(unsigned int val)
{
    m_absKalman->setMinIterations(val);
    m_minIterations = val;
}

void GenfitFitter::setMaxIterations(unsigned int val)
{
    if(val<=4) {
        GenfitMsg::get()<<MSG::ERROR<<"Could NOT set MaxIteration<=4!"<<endmsg;
    }
    m_absKalman->setMaxIterations(val);
    m_maxIterations = val;
}

void GenfitFitter::setMaxIterationsBetas(double bStart,double bFinal,
        unsigned int val)
{
    m_absKalman->setMaxIterations(val);
    m_maxIterations = val;
    if(val<=4) {
        GenfitMsg::get()<<MSG::ERROR<<"Could NOT set MaxIteration<=4!"<<endmsg;
    }
    GenfitMsg::get()<<MSG::DEBUG<<"bStart "<<bStart<<" bFinal "<<bFinal
        <<" MaxIteration "<<val<<endmsg;
    // genfit version less than 2.2.0
    genfit::DAF* daf = dynamic_cast<genfit::DAF*> (m_absKalman);
    daf->setAnnealingScheme(bStart,bFinal,val);
}

void GenfitFitter::setDeltaPval(double val)
{
    m_absKalman->setDeltaPval(val);
    m_deltaPval = val;
}

void GenfitFitter::setRelChi2Change(double val)
{
    m_absKalman->setRelChi2Change(val);
    m_relChi2Change = val;
}

void GenfitFitter::setBlowUpFactor(double val)
{
    m_absKalman->setBlowUpFactor(val);
    m_blowUpFactor = val;
}

void GenfitFitter::setResetOffDiagonals(bool val)
{
    m_absKalman->setResetOffDiagonals(val);
    m_resetOffDiagonals = val;
}

void GenfitFitter::setBlowUpMaxVal(double val)
{
    m_absKalman->setBlowUpMaxVal(val);
    m_blowUpMaxVal = val;
}

void GenfitFitter::setMultipleMeasurementHandling(
        genfit::eMultipleMeasurementHandling val)
{
    m_absKalman->setMultipleMeasurementHandling(val);
    m_multipleMeasurementHandling = val;
}

void GenfitFitter::setMaxFailedHits(int val)
{
    m_absKalman->setMaxFailedHits(val);
    m_maxFailedHits = val;
}

///DAF setters
void GenfitFitter::setConvergenceDeltaWeight(double val)
{
    genfit::DAF* daf = getDAF();
    if(nullptr != daf) daf->setConvergenceDeltaWeight(val);
    m_deltaWeight = val;
}
void GenfitFitter::setAnnealingScheme(
        double bStart, double bFinal, unsigned int nSteps)
{
    genfit::DAF* daf = getDAF();
    if(nullptr != daf) daf->setAnnealingScheme(bStart, bFinal, nSteps);
    m_annealingBetaStart = bStart;
    m_annealingBetaStop = bFinal;
    m_annealingNSteps = nSteps;
}

//TODO chi2Cuts?
///Material effects
void GenfitFitter::setNoEffects(bool val)
{
    genfit::MaterialEffects::getInstance()->setNoEffects();
    m_noEffects = val;
}

void GenfitFitter::setEnergyLossBetheBloch(bool val)
{
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(val);
    m_energyLossBetheBloch = val;
}

void GenfitFitter::setNoiseBetheBloch(bool val)
{
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(val);
    m_noiseBetheBloch = val;
}

void GenfitFitter::setNoiseCoulomb(bool val)
{
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(val);
    m_noiseCoulomb = val;
}

void GenfitFitter::setEnergyLossBrems(bool val)
{
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(val);
    m_energyLossBrems = val;
}

void GenfitFitter::setNoiseBrems(bool val)
{
    genfit::MaterialEffects::getInstance()->setNoiseBrems(val);
    m_noiseBrems = val;
}

void GenfitFitter::setIgnoreBoundariesBetweenEqualMaterials(bool val)
{
    genfit::MaterialEffects::getInstance()->
        ignoreBoundariesBetweenEqualMaterials(val);
    m_ignoreBoundariesBetweenEqualMaterials = val;
}

void GenfitFitter::setMscModelName(std::string val)
{
    genfit::MaterialEffects::getInstance()->setMscModel(val);
    m_mscModelName = val;
}

void GenfitFitter::setMaterialDebugLvl(unsigned int val)
{
    genfit::MaterialEffects::getInstance()->setDebugLvl(val);
}

///Set GenfitFitter parameters
void GenfitFitter::setDebug(unsigned int val)
{
    GenfitMsg::get()<<MSG::DEBUG<<"set fitter debugLvl "<<val<<endmsg;
    m_absKalman->setDebugLvl(val);
    if(nullptr!=getDAF()) getDAF()->setDebugLvl(val);
    if(val>10) genfit::MaterialEffects::getInstance()->setDebugLvl(val);
    m_debug = val;
}

void GenfitFitter::setHist(unsigned int val) {m_hist = val;}

genfit::DAF* GenfitFitter::getDAF()
{
    genfit::DAF* daf = nullptr;
    try{
        daf = dynamic_cast<genfit::DAF*> (m_absKalman);
    }catch(...){
        GenfitMsg::get()<< MSG::ERROR
            << "dynamic_cast m_rom AbsFitter to DAF m_ailed!"<<endmsg;
        return nullptr;
    }
    return daf;
}

genfit::KalmanFitterRefTrack* GenfitFitter::getKalRef()
{
    genfit::KalmanFitterRefTrack* ref=nullptr;
    try{
        ref = dynamic_cast<genfit::KalmanFitterRefTrack*> (m_absKalman);
    }catch(...){
        GenfitMsg::get()<< MSG::ERROR
            << "dynamic_cast m_rom AbsFitter to KalmanFitterRefTrack m_ailed!"
            <<endmsg;
    }
    return ref;
}

void GenfitFitter::initHist(std::string name)
{
    GenfitMsg::get()<<MSG::DEBUG<<"GenfitFitter::initHist "<<name<<endmsg;
    //getDAF()->initHist(name);
}

void GenfitFitter::writeHist()
{
    GenfitMsg::get()<<MSG::DEBUG<<"GenfitFitter::writeHist "<<endmsg;
    //getDAF()->writeHist();
}


