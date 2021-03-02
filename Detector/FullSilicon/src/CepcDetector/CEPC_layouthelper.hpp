#pragma once

#include "DD4hep/DetFactoryHelper.h"

using namespace std;
using namespace dd4hep;

class layerconfigure {
    public:
        // information get from layer 
        double m_r;
        double m_half_z;
        double m_y_gap;
        double m_phi_gap;
        double m_layers_gap; // only > 0 if is double layer

        // information get from module
        double m_module_x_length_1;
        double m_module_x_length_2;
        double m_module_y_length;

        // the calculated results
        int m_Number_modules_z;
        double m_ymin;
        double m_step_z;
        double m_Number_staves_phi;
        double m_deltaphi;
        double m_phi_tilt;

        layerconfigure();
        layerconfigure(const xml_comp_t x_layer,const xml_comp_t x_module);
        void reset();
        ~layerconfigure();
    
};

layerconfigure::layerconfigure(){
    layerconfigure::reset();
}

layerconfigure::~layerconfigure(){
}

layerconfigure::layerconfigure(const xml_comp_t x_layer,const xml_comp_t x_module){
    m_r = x_layer.r();
    m_half_z = x_layer.z();
    m_y_gap = x_layer.gap(); // gap of modules along z
    m_phi_gap = x_layer.attr<double>(_Unicode(phi_gap));// gap of ideal phi
    m_phi_tilt = x_layer.attr<double>(_Unicode(phi_tilt)); 
    m_layers_gap = x_layer.attr<double>(_Unicode(layers_gap));// gap of double layers

    for (xml_coll_t comp(x_module, _U(module_component));comp; ++comp){
        xml_comp_t x_comp=comp;
        if (x_comp.nameStr()=="Sensor"){ // assume that the sensor is the largest
            m_module_x_length_1 = x_comp.x1()*0.5;
            m_module_x_length_2 = x_comp.x2()*0.5;
            m_module_y_length = x_comp.length();
            break;
        }
    }
    // now calculating the numbers
    m_Number_modules_z = floor(((m_half_z*2.0)+m_y_gap)/(m_module_y_length+m_y_gap));
    m_step_z = m_module_y_length+m_y_gap;
    m_deltaphi=2.*atan(m_module_x_length_2/(m_r));
    m_deltaphi-=m_phi_gap;
    m_Number_staves_phi = floor(2. * M_PI / m_deltaphi);
    m_deltaphi = 2. * M_PI / m_Number_staves_phi;
    m_ymin=m_half_z-0.5*m_module_y_length;
    if (m_phi_tilt == 0){m_phi_tilt=0.9*M_PI/m_Number_staves_phi;}
   
   
}
void layerconfigure::reset(){
    m_r=-99.0;
    m_half_z=-99.0;
    m_y_gap=-99.0;
    m_layers_gap=-99.0; // only > 0 if is double layer

    m_module_x_length_1=-99.0;
    m_module_x_length_2=-99.0;
    m_module_y_length=-99.0;

    m_Number_modules_z=-99.0;
    m_step_z=-99.0;
    m_ymin=-99.0;
    m_Number_staves_phi=-99.0;
    m_deltaphi=-99.0;
    m_phi_tilt=-99.0;
}

class ringconfigure {
    public:
        // information get from the x_ring
        double m_r_min;
        double m_r_max;
        double m_r;
        double m_z_offset;
        double m_phi_start;
        double m_phi_end;
        double m_in_layer_gap;
        double m_layers_gap; // if double layers
        double m_phi_gap; //  gap of ideal module 
  
        // information get from module
        double m_module_x_length_1;
        double m_module_x_length_2;
        double m_module_y_length;

        // the calculated results
        double m_Number_in_phi;
        double m_deltaphi;

        ringconfigure();
        ringconfigure(const xml_comp_t x_ring,const xml_comp_t x_module);
        void reset();
        ~ringconfigure();
};
ringconfigure::ringconfigure(){
    ringconfigure::reset();
}

ringconfigure::~ringconfigure(){
}

ringconfigure::ringconfigure(const xml_comp_t x_ring,const xml_comp_t x_module){
    m_r_min=x_ring.inner_r();
    m_r_max=x_ring.outer_r();
    m_r = x_ring.r();
    m_z_offset=x_ring.z_offset();
    m_phi_start=x_ring.attr<double>(_Unicode(phi_start));
    m_phi_end=x_ring.attr<double>(_Unicode(phi_end));
    m_in_layer_gap=x_ring.attr<double>(_Unicode(in_layer_gap));
    m_layers_gap=x_ring.attr<double>(_Unicode(layers_gap));
    m_phi_gap=x_ring.attr<double>(_Unicode(phi_gap));

    for (xml_coll_t comp(x_module, _U(module_component));comp; ++comp){
        xml_comp_t x_comp=comp;
        if (x_comp.nameStr()=="Sensor"){ // assume that the sensor is the largest
            m_module_x_length_1 = x_comp.x1() * 0.5;
            m_module_x_length_2 = x_comp.x2() * 0.5;
            m_module_y_length = x_comp.length();
            break;
        }
    }
    // now calculating 
    m_deltaphi=2.*atan(0.5*(m_module_x_length_1+m_module_x_length_2)/(m_r));
	std::cout<<"m_deltaphi "<<m_deltaphi<<std::endl;
    m_deltaphi-=m_phi_gap;
	std::cout<<"m_deltaphi- "<<m_deltaphi<<std::endl;
    m_Number_in_phi = floor(2. * M_PI / m_deltaphi);
	std::cout<<"m_Number_in_phi- "<<m_Number_in_phi<<std::endl;
    m_deltaphi = 2. * M_PI / m_Number_in_phi;
   
}

void ringconfigure::reset(){
    m_r_min=-99.0;
    m_r_max=-99.0;
    m_r=-99.0;
    m_z_offset=-99.0;
    m_phi_start=-99.0;
    m_phi_end=-99.0;
    m_phi_gap=-99.0;
    m_in_layer_gap=-99.0;
    m_layers_gap=-99.0; // only > 0 if is double layer

    m_module_x_length_1=-99.0;
    m_module_x_length_2=-99.0;
    m_module_y_length=-99.0;

    m_Number_in_phi=-99.0;
    m_deltaphi=-99.0;
}
