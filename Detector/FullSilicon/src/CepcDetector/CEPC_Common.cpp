#include "Acts/Plugins/DD4hep/ActsExtension.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepMaterial.hpp"
#include "DD4hep/DetFactoryHelper.h"
#include "CEPC_service.hpp"
#include "CEPC_assambleHelper.hpp"

using namespace std;
using namespace dd4hep;

static Ref_t 
create_element(Detector& lcdd, xml_h xml, SensitiveDetector sens)
{

    xml_det_t x_det = xml;
    string det_name = x_det.nameStr();

    // Make DetElement 
    if (det_name.find("barrel")!=string::npos || det_name.find("Barrel")!=string::npos || det_name.find("Endcap")!=string::npos || det_name.find("endcap")!=string::npos){
        if (det_name.find("barrel")!=string::npos || det_name.find("Barrel")!=string::npos){
            
            // Make DetElement 
            DetElement barrelDetector(det_name,x_det.id());

            // Add Extension to DetElement for the RecoGeometry
            Acts::ActsExtension* barrelExtension = new Acts::ActsExtension();
            barrelExtension->addType("barrel", "detector");
            // Add the volume material if configured
            if (x_det.hasChild(_Unicode(boundary_material))) {
                xml_comp_t x_boundary_material = x_det.child(_Unicode(boundary_material));

                //// Inner / outer are cylinders
                //xml2ProtoMaterial(x_boundary_material,*barrelExtension,"boundary_material",{"inner", "outer"},{"binPhi", "binZ"});
                //// Negative / positive are discs
                //xml2ProtoMaterial(x_boundary_material,*barrelExtension,"boundary_material",{"negative", "positive"},{"binPhi", "binR"});

				xmlToProtoSurfaceMaterial(
					x_boundary_material, *barrelExtension, "boundary_material");
            }
            barrelDetector.addExtension<Acts::ActsExtension>(barrelExtension);
            
            // make volume
            dd4hep::xml::Dimension x_det_dim(x_det.dimensions());

            Tube barrelShape(x_det_dim.rmin(), x_det_dim.rmax(), x_det_dim.dz());
            Volume barrelVolume(det_name, barrelShape, lcdd.air());
            //
            //Assembly barrelVolume("Barrel_layers");
            barrelVolume.setVisAttributes(lcdd, x_det.visStr()); 
            
            if (x_det.hasChild(_Unicode(barrel_layer))){
                size_t layerNum=0;
                for (xml_coll_t layer_x(xml,_Unicode(barrel_layer));layer_x;++layer_x,++layerNum){
                    xml_comp_t x_layer = layer_x;
                    string barrel_layer_name = det_name + _toString((int)layerNum,"_layer_%d");
                    auto each_layer = assembleBarrelLayers(lcdd, sens, x_layer);

                    PlacedVolume placedLayer = barrelVolume.placeVolume(each_layer.first);
                    placedLayer.addPhysVolID("layer", layerNum); 

                    auto layer_Element = each_layer.second.clone(barrel_layer_name,layerNum);
                    layer_Element.setPlacement(placedLayer);
                    barrelDetector.add(layer_Element); 

                }
            }
            
            Volume       motherVolume = lcdd.pickMotherVolume(barrelDetector);
            Position     translation(0., 0.,0.);
            PlacedVolume placedBarrel=motherVolume.placeVolume(barrelVolume, translation);
            
            placedBarrel.addPhysVolID("system", barrelDetector.id()); 
            barrelDetector.setPlacement(placedBarrel); 
            std::cout<<"Finish constructing the barrel "<< det_name<<std::endl;

            return barrelDetector;

        }else if (det_name.find("Endcap")!=string::npos || det_name.find("endcap")!=string::npos){ 
            DetElement endcapDetector(det_name,x_det.id());
            // Add Extension to DetElement for the RecoGeometry
            Acts::ActsExtension* endcapExtension = new Acts::ActsExtension();
            endcapExtension->addType("endcap", "detector"); 
            endcapDetector.addExtension<Acts::ActsExtension>(endcapExtension);
            
            // make volume
            dd4hep::xml::Dimension x_det_dim(x_det.dimensions());

            //Assembly endcapVolume("Endcap_disks");
            //
            Tube endcapShape(x_det_dim.rmin(), x_det_dim.rmax(), x_det_dim.dz());
            Volume endcapVolume(det_name, endcapShape, lcdd.air());
            endcapVolume.setVisAttributes(lcdd, x_det.visStr()); 
            // place the volume            
            Volume       motherVolume = lcdd.pickMotherVolume(endcapDetector);
            Position     translation(0., 0., x_det_dim.z());
            PlacedVolume placedEndcap=motherVolume.placeVolume(endcapVolume, translation);

            if (x_det.hasChild(_Unicode(endcap_disk))){
                size_t diskNum=0;
                for (xml_coll_t disk_x(xml,_Unicode(endcap_disk));disk_x;++disk_x,++diskNum){
                    xml_comp_t x_disk = disk_x;

               //     string endcap_disk_name = det_name + _toString((int)diskNum,"_%d");
                    string endcap_disk_name = det_name + x_disk.nameStr();
                    auto each_disk = assembleEndcapLayers(lcdd, sens, x_disk);

                    PlacedVolume placedDisk = endcapVolume.placeVolume(each_disk.first, Position(0.,0.,x_disk.z()));
                    placedDisk.addPhysVolID("disk", diskNum); 

                    auto disk_Element = each_disk.second.clone(endcap_disk_name,diskNum);
                    disk_Element.setPlacement(placedDisk);
                    endcapDetector.add(disk_Element); 

                }
            }
            
            
            placedEndcap.addPhysVolID("system", endcapDetector.id()); 
            endcapDetector.setPlacement(placedEndcap); 
            std::cout<<"Finish constructing the endcap "<<det_name<<std::endl;
            return endcapDetector;
        }
    }

    DetElement e_Detector(det_name,x_det.id());
    std::cout<<"Double check the detector name to be sure contain: Barrel, barrel, Endcap or endcap"<<std::endl;
    return e_Detector;
    
}
DECLARE_DETELEMENT(cepc_common, create_element)
