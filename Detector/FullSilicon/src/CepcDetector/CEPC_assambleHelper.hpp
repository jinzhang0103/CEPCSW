#pragma once

#include "DD4hep/DetFactoryHelper.h"
#include "Acts/Plugins/DD4hep/ConvertDD4hepMaterial.hpp"
#include "CEPC_service.hpp"
#include "CEPC_layouthelper.hpp"
using namespace std;
using namespace dd4hep;

static std::pair<Assembly, DetElement>
assembleTrapezoidalModule(Detector& lcdd,SensitiveDetector& sens,const xml_comp_t& x_module){
    // THe module envelop volume
    Assembly moduleAssembly("module");
    // Visualization
    moduleAssembly.setVisAttributes(lcdd, x_module.visStr());
    
    // the module detector element
    DetElement moduleElement("ModuleElementTemplate", 0);
 
    //Place the components inside the module 
    unsigned int compNum   = 0; 
    unsigned int sensorNum = 0; 
    for (xml_coll_t comp(x_module, _U(module_component)); comp;++comp, ++compNum) { 
        xml_comp_t x_comp = comp;
        
        //create the component volume
        string compName = _toString((int)compNum,"component%d") + x_comp.materialStr();
           
        Trapezoid trapShape(0.5 * x_comp.x2(),
                            0.5 * x_comp.x1(),
                            0.5 * x_comp.thickness(),
                            0.5 * x_comp.thickness(),
                            0.5 * x_comp.length());
        std::cout<<"the module shape : "<<x_comp.x1()<<" "<<x_comp.x2()<<" "<<x_comp.length()<<std::endl; 
        Volume componentVolume(compName, trapShape, lcdd.material(x_comp.materialStr()));
        componentVolume.setVisAttributes(lcdd,x_comp.visStr());
     
        //place the component
        double stereoAlpha     = x_comp.alpha();
        PlacedVolume placedComponent = moduleAssembly.placeVolume(componentVolume,Transform3D(RotationY(stereoAlpha)*RotationX(0.5*M_PI), Position(x_comp.x_offset(), x_comp.y_offset(), x_comp.z_offset())));
        if (x_comp.hasChild(_Unicode(service))) {
            std::cout<<"------------------adding the service-------------------"<<std::endl;
            decorations_materials(lcdd, componentVolume, x_comp.child(_Unicode(service))); 
          // later to be specified  decorations_materials(lcdd,componentVOlume)
        }
        // deal with the sensitive sensor
        if (x_comp.isSensitive()) {
            componentVolume.setSensitiveDetector(sens); 
            placedComponent.addPhysVolID("sensor", sensorNum++);

            // Create the sensor element and place it  
            string     sensorName = _toString((int)sensorNum, "sensor%d");
            DetElement sensorElement(moduleElement, sensorName, sensorNum); 
            sensorElement.setPlacement(placedComponent);

            // Add the sensor extension
            Acts::ActsExtension* sensorExtension = new Acts::ActsExtension();
            sensorExtension->addType("sensor", "detector");
            sensorExtension->addType("axes", "definitions", "XZY"); 

            sensorElement.addExtension<Acts::ActsExtension>(sensorExtension);   
        }
    }
    if (x_module.hasChild(_Unicode(service))) {
        decorations_materials(lcdd, moduleAssembly, x_module.child(_Unicode(service)));
    }
    return std::pair<Assembly, DetElement>(moduleAssembly, moduleElement); 
}

static std::pair<Volume, DetElement> 
assembleBarrelLayers(Detector& lcdd, SensitiveDetector& sens,const xml_comp_t& x_layer){
 
    // The Barrel layer volume 
    //Assembly OneLayerAssembly("One_BarrelLayer");
    Volume OneLayerAssembly(x_layer.nameStr(), Tube(x_layer.inner_r(),x_layer.outer_r(),x_layer.z()), lcdd.air());
    OneLayerAssembly.setVisAttributes(lcdd, x_layer.visStr());  
    // the layer detector element
    DetElement layerElement("BarrelLayerTemplate",0);

    // resolve the components of the layer
    // first checking if has module just in case 
    if (x_layer.hasChild(_Unicode(module))){

        xml_comp_t x_module = x_layer.child(_U(module));
        // get the volume and detector element of the module
        auto module  = assembleTrapezoidalModule(lcdd,sens,x_module);

        // the configuration of layer, like the module's phi, position, etc
        layerconfigure layer_conf(x_layer,x_module);

        // first assemble the staves
        Assembly staveAssembly("stave");
        // vis currently using same color as layer
        staveAssembly.setVisAttributes(lcdd, x_layer.visStr());
        // create the stave detector element
        DetElement staveElementTemplate("StaveElementTemplate", 0);
        
        int odd = 0;

        for (unsigned int moduleNum_in_stave = 0; moduleNum_in_stave < layer_conf.m_Number_modules_z; ++moduleNum_in_stave){ // first construct the staves

            odd=moduleNum_in_stave%2;
            PlacedVolume placedModule = staveAssembly.placeVolume(module.first, Position(0., -(layer_conf.m_ymin) + moduleNum_in_stave * layer_conf.m_step_z, 0.+ odd*x_layer.attr<double>(_Unicode(in_layer_gap)))); // axis 
            placedModule.addPhysVolID("module", moduleNum_in_stave); 

            string moduleName = _toString((int)moduleNum_in_stave,"module%d");
            // clone the detector element
            auto moduleElement = module.second.clone(moduleName, moduleNum_in_stave);
            moduleElement.setPlacement(placedModule);
            // Assign it as chile to the stave template
            staveElementTemplate.add(moduleElement);
            if (x_layer.hasChild(_Unicode(service))) {
                std::cout<<"-----------------layer service---------------"<<std::endl;
                decorations_materials(lcdd, staveAssembly, x_layer.child(_Unicode(service)));
            }
            
        }
        double phi_temp=0.0;
        double r = x_layer.r();
        double r_double_side = 0.0;
        
        int if_double_side = 0;
        if (layer_conf.m_layers_gap>0) {if_double_side = 1;r_double_side = r+layer_conf.m_layers_gap;}
        unsigned int stave_at_phi=0;
        for (int side = 0; side <= if_double_side; side ++){
 
            for (;stave_at_phi < layer_conf.m_Number_staves_phi*(side+1); ++stave_at_phi){
                string staveName = _toString((int)stave_at_phi,"stave%d");
                // get current phi 
                phi_temp = layer_conf.m_deltaphi / dd4hep::rad * stave_at_phi;
                // placement
                r = side==0?r:r_double_side;
                PlacedVolume placedStave = OneLayerAssembly.placeVolume(staveAssembly, Transform3D(RotationY(0.5 * M_PI)*RotationZ(0.5 * M_PI)*RotationY(phi_temp + layer_conf.m_phi_tilt),Position(r*cos(phi_temp),r*sin(phi_temp),0.)));
                //PlacedVolume placedStave = OneLayerAssembly.placeVolume(staveAssembly, Transform3D(RotationY(0.5 * M_PI)*RotationZ(0.5 * M_PI)*RotationY(phi_temp + layer_conf.m_phi_tilt+0.5 * M_PI),Position(r*cos(phi_temp),r*sin(phi_temp),0.)));
                placedStave.addPhysVolID("stave",stave_at_phi);
                // clone the detector element 
                DetElement staveElement = staveElementTemplate.clone(staveName, stave_at_phi);
                staveElement.setPlacement(placedStave);
                // add to the layer element
                layerElement.add(staveElement);
                
            }
        }
    }
    // ActsExtension for the layer
    Acts::ActsExtension* layerExtension = new Acts::ActsExtension();
    layerExtension->addType("active cylinder", "layer");
    //Acts::xml2CylinderProtoMaterial(x_layer, *layerExtension);
    for (xml_coll_t lmat(x_layer, _Unicode(layer_material)); lmat; ++lmat) {
      xml_comp_t x_layer_material = lmat;
      Acts::xmlToProtoSurfaceMaterial(
          x_layer_material, *layerExtension, "layer_material");
    }
    layerElement.addExtension<Acts::ActsExtension>(layerExtension);
 
    // return the layer assembly 
    return std::pair<Assembly, DetElement>(OneLayerAssembly,layerElement);
}

static std::pair<Assembly, DetElement> 
assembleEndcapLayers(Detector& lcdd, SensitiveDetector& sens,const xml_comp_t& x_disk){

    // The EndCap layer volume 
    //Assembly OneDiskAssembly("One_EndCapLayer");
    Volume OneDiskAssembly(x_disk.nameStr(), Tube(x_disk.inner_r(),x_disk.outer_r(),x_disk.dz()), lcdd.air());
    // Vis
    OneDiskAssembly.setVisAttributes(lcdd, x_disk.visStr());  
    // the layer detector element
    DetElement DiskElement("EndCapDiskTemplate",0);

    // resolve the components of the disk
    // first checking if has module just in case 
    // for disk might have different moduels in different R 
    //
    // but only allow one module currently because cannot add type of xml_comp_t into pair<>
    //
    if (x_disk.hasChild(_Unicode(module))){
        //std::map<string, std::pair<Assembly, DetElement> > map_modules; 
        //std::map<int, xml_comp_t > map_modules_comp; 
        //int module_ids=0;
        //for (xml_coll_t module_x(x_disk,_U(module));module_x;++module_x,++module_ids){
        //    xml_comp_t x_module = module_x;
        //    map_modules[x_module.nameStr()]=assembleTrapezoidalModule(lcdd,sens,x_module);
            //map_modules_comp[module_ids]=x_module;
        // }

        xml_comp_t x_module = x_disk.child(_U(module));
        auto module=assembleTrapezoidalModule(lcdd,sens,x_module);

        // the configuration of layer, like the module's phi, position, etc
        
        size_t ringNum=0;
        for (xml_coll_t ring(x_disk,_U(ring));ring;++ring,++ringNum){
            // get the ring
            xml_comp_t x_ring = ring;
            // set name 
            string ringName = _toString((int)ringNum,"ring%d");
            // ring volume
            Assembly ringAssembly(ringName);
            // vis
            ringAssembly.setVisAttributes(lcdd, x_ring.visStr());
            // ring detector element
            DetElement ringElement(ringName, ringNum);
            // get the placement information of modules in this ring 
            //ringconfigure ring_conf(x_ring,map_modules_comp[x_ring.attr<int>(_Unicode(usingModuleid))]);
            ringconfigure ring_conf(x_ring,x_module);
            std::cout<<"----------------------ring "<<ringNum<<"----------------"<<std::endl; 
            double phi_temp=0.0;
            double z_temp_pre=0.0;
            double z_temp=0.0;
            int if_double_side = 0;
            
            // loop over phi to put the modules
            if (ring_conf.m_layers_gap>0) {if_double_side = 1;}
            unsigned int modNum=0;
            for (int side = 0; side <= if_double_side; side ++){
//			  std::cout<<"num "<<ring_conf.m_Number_in_phi*(side+1)<<std::endl;
///			  std::cout<<"num_in_phi "<<ring_conf.m_Number_in_phi<<",side+1 "<< side+1 <<std::endl;
                for (; modNum < ring_conf.m_Number_in_phi*(side+1); ++modNum){
                    // module name
                    string moduleName = _toString((int)modNum,"module%d");
                    // if it's odd
                    bool odd = bool(int(modNum-(ring_conf.m_Number_in_phi*side))%2);
                    // position
                    phi_temp = ring_conf.m_phi_start + modNum * ring_conf.m_deltaphi;
                    z_temp_pre = odd ? -(ring_conf.m_in_layer_gap):(ring_conf.m_in_layer_gap);
                    z_temp = side==0 ? z_temp_pre:(z_temp_pre+((z_temp_pre)/abs(z_temp_pre))*ring_conf.m_layers_gap);
                    Position trans(ring_conf.m_r * cos(phi_temp),ring_conf.m_r * sin(phi_temp),z_temp);
                    // place the module Box Volum, flip if necessary
                    double flip = ring_conf.m_z_offset < 0. ? M_PI : 0.;
                    if (ringNum != 0) { flip += M_PI; }
//					std::cout<<"mod num "<<modNum<<std::endl;
//					std::cout<<"ring "<<ringNum<<","<<trans<<std::endl;

                    PlacedVolume placedModule = ringAssembly.placeVolume(module.first,Transform3D(RotationZ(phi_temp + 1.5 * M_PI) * RotationY(flip), trans));
                    //PlacedVolume placedModule = ringAssembly.placeVolume(map_modules[x_ring.attr<string>(_Unicode(usingModule))].first,Transform3D(RotationZ(phi_temp + 1.5 * M_PI) * RotationY(flip), trans));
                    placedModule.addPhysVolID("module", modNum);
                    //clone 
                    auto moduleElement = module.second.clone(moduleName, modNum);
                    //auto moduleElement = map_modules[x_ring.attr<string>(_Unicode(usingModule))].second.clone(moduleName, modNum);
                    moduleElement.setPlacement(placedModule);
                    // assign it as child to the ring temolate
                    ringElement.add(moduleElement);
                }
              }
            // place the ring assembly into disk
            PlacedVolume placedRing = OneDiskAssembly.placeVolume(ringAssembly, Position(0., 0., x_ring.z_offset()));
            placedRing.addPhysVolID("ring", ringNum);
            ringElement.setPlacement(placedRing);
            DiskElement.add(ringElement);
        }

    }
    // ActsExtension for the layer 
    Acts::ActsExtension* diskExtension = new Acts::ActsExtension();
    diskExtension->addType("sensitive disk", "layer");
    //Acts::xml2DiscProtoMaterial(x_disk, *diskExtension);
    for (xml_coll_t lmat(x_disk, _Unicode(layer_material)); lmat; ++lmat) {
      xml_comp_t x_layer_material = lmat;
      Acts::xmlToProtoSurfaceMaterial(
          x_layer_material, *diskExtension, "layer_material");
    }
    DiskElement.addExtension<Acts::ActsExtension>(diskExtension);

    return std::pair<Assembly, DetElement>(OneDiskAssembly,DiskElement);
}
