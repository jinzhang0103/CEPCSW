#include "Acts/Plugins/DD4hep/ActsExtension.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepMaterial.hpp"
#include "DD4hep/DetFactoryHelper.h"
#include "ACTFW/DD4hepDetector/DD4hepDetectorHelper.hpp"

using namespace std;
using namespace dd4hep;

/**
 Constructor for a cylindrical barrel volume, possibly containing layers and the
 layers possibly containing modules.
 */

static Ref_t
create_element(Detector& lcdd, xml_h xml, SensitiveDetector sens)
{
    xml_det_t x_det    = xml;
    string    det_name = x_det.nameStr();
    // Make DetElement
    DetElement cylinderVolume(det_name, x_det.id());
  
    // add Extension to Detlement for the RecoGeometry
    Acts::ActsExtension* detvolume = new Acts::ActsExtension();
	detvolume->addType("barrel", "detector");

    cylinderVolume.addExtension<Acts::ActsExtension>(detvolume);
  
    // make Volume
    dd4hep::xml::Dimension x_det_dim(x_det.dimensions());
    Tube   tube_shape(x_det_dim.rmin(), x_det_dim.rmax(), x_det_dim.dz());
    Volume tube_vol(det_name, tube_shape,lcdd.vacuum()); 
    double Length_TPC_=x_det_dim.dz();
    double Thickness_TPC_=x_det_dim.rmax()-x_det_dim.rmin();
    tube_vol.setVisAttributes(lcdd, x_det_dim.visStr());
  
    // go trough possible layers
    double layer_n_total;
    int layer_number_=0;
    for (xml_coll_t j(xml, _U(layer)); j; ++j) {
        xml_comp_t x_layer = j;
        double     TPC_rmin = x_layer.rmin();
        double     TPC_rmax = x_layer.rmax();
        string     layer_name_temp=x_layer.nameStr();
        double     TPC_layer_number=x_layer.number(); // get how many TPC layers you wanted to add 
        std::cout<<layer_name_temp<<" : "<<TPC_layer_number<<std::endl;  
        double     TPC_layer_thickness  = (TPC_rmax-TPC_rmin)/(TPC_layer_number+1); // the thickness of the layer
        std::cout<<"thickness :"<<TPC_layer_thickness<<std::endl;
        int        has_thin_layer = x_layer.attr<int>(_Unicode(has_thin_layer)); // whether you wanted to add a thin layer instead of modules
  
  
        double l_rmin=TPC_rmin;
        double l_rmax=0.0; 
        string layer_name = layer_name_temp;
        Volume layer_vol(layer_name,Tube(TPC_rmin, TPC_rmax, Length_TPC_),lcdd.material(x_layer.materialStr()));
        DetElement lay_det(cylinderVolume, layer_name, layer_number_);
        layer_number_++;
        layer_vol.setVisAttributes(lcdd, x_layer.visStr());
        double module_num=0.0;
        int exter=1;
        for (int layer_v=0;layer_v<TPC_layer_number;layer_v++){
  
            // Create Volume for Layer
            //string layer_name = layer_name_temp+_toString((int)layer_v,"_layer_%d");
  
            //l_rmin +=TPC_layer_thickness;
		  std::cout<<"TPC rmin max "<<TPC_rmin<<","<<TPC_rmax<<std::endl;
            l_rmin =TPC_rmin+(layer_v + 1) *TPC_layer_thickness + 1;
            //std::cout<<"creating layer : "<<layer_v<<" at "<< l_rmin<<std::endl;
             
            l_rmax = l_rmin+0.01;
  
			if (has_thin_layer==1){
			  // if no module defined, and you want to add a cylinder layer of thickness 0.01*mm at same radius
			  //
			  //set the name of the thin layer
			  std::cout<<"layer_thin : "<<layer_v<<" at "<<l_rmin<<","<<l_rmax<< " length: "<<Length_TPC_-0.1<<std::endl;
			  string layer_thin_name = layer_name+ _toString((int)layer_v,"_sub_%d");
			  // create the volumen at l_rmin and fill the volume with vacuum
			  Volume layer_thin_vol(layer_thin_name,Tube(l_rmin,l_rmax,Length_TPC_-0.1),lcdd.vacuum()); // unit cm
			  // set vis
			  xml_comp_t thin_layer_conf = x_layer.child(_Unicode(thin_layer_conf));
			  layer_thin_vol.setVisAttributes(lcdd, thin_layer_conf.visStr());
			  std::cout<<"conf "<<std::endl;
			  //layer_thin_vol.setVisAttributes(lcdd, x_layer.visStr());
			  // creats Detector element
			  //DetElement layer_thin_det(cylinderVolume, layer_thin_name, layer_v);
			  DetElement layer_thin_det(lay_det, layer_thin_name, layer_v);
			  std::cout<<"layer_thin_det create "<<l_rmin<<std::endl;
			  // set the volume to sensitive
			  layer_thin_vol.setSensitiveDetector(sens);
			  // config and add actsextension to the layer_thin_det
			  Acts::ActsExtension* layer_thinExtension = new Acts::ActsExtension();
			  layer_thinExtension->addType("active cylinder", "layer");
			  layer_thin_det.addExtension<Acts::ActsExtension>(layer_thinExtension);
			  // place the volume default: no rotation, no shift
			  PlacedVolume placeLayer_thin = layer_vol.placeVolume(layer_thin_vol);
			  placeLayer_thin.addPhysVolID("sub_layer",layer_v);
			  //placeLayer_thin.addPhysVolID("module",layer_v);
			  layer_thin_det.setPlacement(placeLayer_thin);
			  layer_n_total++;
			}
		}

		Acts::ActsExtension* detlayer = new Acts::ActsExtension();
		detlayer->addType("active cylinder", "layer");
		lay_det.addExtension<Acts::ActsExtension>(detlayer);
		// Place layer volume
		PlacedVolume placedLayer = tube_vol.placeVolume(layer_vol);
		placedLayer.addPhysVolID("layer", layer_n_total);
		// Assign layer DetElement to layer volume
		lay_det.setPlacement(placedLayer);
		layer_n_total++;
	}

	// Place Volume
	Volume       mother_vol = lcdd.pickMotherVolume(cylinderVolume);
	PlacedVolume placedTube = mother_vol.placeVolume(tube_vol);
	placedTube.addPhysVolID("system", cylinderVolume.id());
	cylinderVolume.setPlacement(placedTube);
	return cylinderVolume;
}
DECLARE_DETELEMENT(cepc_TPC_barrel_cylinder, create_element)
