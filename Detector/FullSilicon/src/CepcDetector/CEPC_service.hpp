#pragma once

#include "DD4hep/DetFactoryHelper.h"

using namespace std;
using namespace dd4hep;

static Volume
relate_2_volumes(Detector& lcdd,const xml_comp_t& x_orig, const xml_comp_t& x_besub, string operation){
   
    Tube origVolum; 
    Tube besub_Volum; 
    if (x_orig.attr<string>(_Unicode(shape))=="tube") {
        Tube origVolum(x_orig.rmin(), x_orig.rmax(),x_orig.length());
    }else if (x_orig.attr<string>(_Unicode(shape))=="Trapezoid") {
        Trapezoid origVolum(x_orig.x1(),x_orig.x2(),x_orig.attr<int>(_Unicode(thick1)),x_orig.attr<int>(_Unicode(thick2)),x_orig.length());
    } else {
        std::cout<<"unknown orig shape ........."<<std::endl;}

    if (x_besub.attr<string>(_Unicode(shape))=="tube") {
        Tube besub_Volum(x_besub.rmin(), x_besub.rmax(),x_besub.length());
    }else if (x_besub.attr<string>(_Unicode(shape))=="Trapezoid") {
        Trapezoid besub_Volum(x_besub.x1(),x_besub.x2(),x_besub.attr<int>(_Unicode(thick1)),x_besub.attr<int>(_Unicode(thick2)),x_besub.length());
    } else {
        std::cout<<"unknown besub shape ........."<<std::endl;}
 
    Volume outputVolume;
    if (operation=="subtraction") {
        Volume outputVolume(x_orig.nameStr(),SubtractionSolid(origVolum,besub_Volum,Transform3D(RotationX(0.5 * M_PI))),lcdd.material(x_orig.materialStr()));
    }else if (operation=="union") {
        Volume outputVolume(x_orig.nameStr(),UnionSolid(origVolum,besub_Volum,Transform3D(RotationX(0.5 * M_PI))),lcdd.material(x_orig.materialStr()));
    }else if (operation=="intersection"){
        Volume outputVolume(x_orig.nameStr(),IntersectionSolid(origVolum,besub_Volum,Transform3D(RotationX(0.5 * M_PI))),lcdd.material(x_orig.materialStr()));
    }else {
        std::cout<<"unknown operation ........."<<std::endl;}

    return outputVolume;

}


template <typename volume_t>
void decorations_materials(Detector& lcdd, volume_t& upperVolume, const xml_comp_t& x_service){
//void decorations_materials(Detector& lcdd, volume_t& upperVolume, Transform3D& trans, Position& position, const xml_comp_t& x_service){

    if (x_service.hasChild(_Unicode(Addition))){
        for (xml_coll_t add_x(x_service,_Unicode(Addition));add_x;add_x++){
            xml_comp_t x_add = add_x;
 
            if (x_add.hasChild(_Unicode(Subtraction))){
                xml_comp_t x_sub = x_add.child((_Unicode(Subtraction)));
                Volume out_volume=relate_2_volumes(lcdd,x_add,x_sub,"subtraction");
                out_volume.setVisAttributes(lcdd, x_add.visStr());
                PlacedVolume place_decorations = upperVolume.placeVolume(out_volume,Transform3D(RotationX(x_add.attr<double>(_Unicode(x_rotation))*M_PI)*RotationY(x_add.attr<double>(_Unicode(y_rotation))*M_PI)* RotationZ(x_add.attr<double>(_Unicode(z_rotation))*M_PI) ,Position(x_add.x_offset(),x_add.y_offset(), x_add.z_offset())));
            }
            else if (x_add.hasChild(_Unicode(Union))){
                xml_comp_t x_sub = x_add.child((_Unicode(Union)));
                Volume out_volume=relate_2_volumes(lcdd,x_add,x_sub,"union");
                out_volume.setVisAttributes(lcdd, x_add.visStr());
                PlacedVolume place_decorations = upperVolume.placeVolume(out_volume,Transform3D(RotationX(x_add.attr<double>(_Unicode(x_rotation))*M_PI)*RotationY(x_add.attr<double>(_Unicode(y_rotation))*M_PI)* RotationZ(x_add.attr<double>(_Unicode(z_rotation))*M_PI) ,Position(x_add.x_offset(),x_add.y_offset(), x_add.z_offset())));
            }
            else if (x_add.hasChild(_Unicode(intersection))){
                xml_comp_t x_sub = x_add.child((_Unicode(intersection)));
                Volume out_volume=relate_2_volumes(lcdd,x_add,x_sub,"subtraction");
                out_volume.setVisAttributes(lcdd, x_add.visStr());
                PlacedVolume place_decorations = upperVolume.placeVolume(out_volume,Transform3D(RotationX(x_add.attr<double>(_Unicode(x_rotation))*M_PI)*RotationY(x_add.attr<double>(_Unicode(y_rotation))*M_PI)* RotationZ(x_add.attr<double>(_Unicode(z_rotation))*M_PI) ,Position(x_add.x_offset(),x_add.y_offset(), x_add.z_offset())));
            }
            else {
                //Tube origVolum;
                if (x_add.attr<string>(_Unicode(shape))=="tube"){
                    std::cout<<"------------------adding tube-----------------------"<<std::endl;
                    Tube origVolum(x_add.rmin(), x_add.rmax(),x_add.length());
                    Volume out_volume(x_add.nameStr(),origVolum,lcdd.material(x_add.materialStr()));
                    out_volume.setVisAttributes(lcdd, x_add.visStr());
                    std::cout<<"------------------place it-----------------------"<<std::endl;
                    PlacedVolume place_decorations = upperVolume.placeVolume(out_volume,Transform3D(RotationX(x_add.attr<double>(_Unicode(x_rotation))*M_PI)*RotationY(x_add.attr<double>(_Unicode(y_rotation))*M_PI)* RotationZ(x_add.attr<double>(_Unicode(z_rotation))*M_PI) ,Position(x_add.x_offset(),x_add.y_offset(), x_add.z_offset())));
                } else if (x_add.attr<string>(_Unicode(shape))=="Trapezoid"){
                    std::cout<<"------------------adding trapezoid-----------------------"<<std::endl;
                    Trapezoid origVolum(x_add.x1(),x_add.x2(),x_add.attr<int>(_Unicode(thick1)),x_add.attr<int>(_Unicode(thick2)),x_add.length());
                    Volume out_volume(x_add.nameStr(),origVolum,lcdd.material(x_add.materialStr()));
                    out_volume.setVisAttributes(lcdd, x_add.visStr());
                    std::cout<<"------------------place it-----------------------"<<std::endl;
                    PlacedVolume place_decorations = upperVolume.placeVolume(out_volume,Transform3D(RotationX(x_add.attr<double>(_Unicode(x_rotation))*M_PI)*RotationY(x_add.attr<double>(_Unicode(y_rotation))*M_PI)* RotationZ(x_add.attr<double>(_Unicode(z_rotation))*M_PI) ,Position(x_add.x_offset(),x_add.y_offset(), x_add.z_offset())));
                } else{
                    std::cout<<"unknown shape"<<std::endl;
                }
                 
            }
            // place the additation on upper volume
            //PlacedVolume place_decorations = upperVolume.placeVolume(out_volume,Position(x_add.x_offset(), x_add.y_offset(), x_add.z_offset()));
        }
    }
}

