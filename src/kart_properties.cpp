//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
#include <stdexcept>
#include <plib/ssg.h>
#include "material_manager.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "loader.hpp"
#include "file_manager.hpp"
#include "string_utils.hpp"
#include "kart_properties.hpp"
#include "stk_config.hpp"
#include "translation.hpp"
#include "ssg_help.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

// This constructor would be a bit more useful, nicer, if we could call
// init_defaults() and load from here. Unfortunately, this object is used
// as a base class for STKConfig, which has to overwrite
// init_defaults() and getAllData(). But during the call of this constructor,
// the STKConfig object does not (yet) exist, so the overwriting
// functions do NOT get called, only the virtual functions here would be
// called. Therefore, a two step initialisation is necessary: the constructor
// doing not much, but then in load the overwriting functions can be used.
KartProperties::KartProperties() : m_icon_material(0), m_model(0)
{}   // KartProperties

//-----------------------------------------------------------------------------
void KartProperties::load(const std::string filename, const std::string node,
                          bool dont_load_models, bool dont_load_materials)
{

    init_defaults();

    const lisp::Lisp* root = 0;
    m_ident = StringUtils::basename(StringUtils::without_extension(filename));

    try
    {
        lisp::Parser parser;
        root = parser.parse(filename);

        const lisp::Lisp* const LISP = root->getLisp(node);
        if(!LISP)
        {
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), "No '%s' node found.", node.c_str());
            throw std::runtime_error(msg);
        }
        getAllData(LISP);
    }
    catch(std::exception& err)
    {
        fprintf(stderr, "Error while parsing KartProperties '%s':\n", 
                filename.c_str());
        fprintf(stderr, err.what());
        fprintf(stderr, "\n");
    }
    delete root;

    if(!dont_load_materials)
    {
        // Load material
        std::string materials_file = file_manager->getKartFile("materials.dat",getIdent());
        file_manager->pushModelSearchPath(file_manager->getKartFile("", getIdent()));
        file_manager->pushTextureSearchPath(file_manager->getKartFile("", getIdent()));

        // addShared makes sure that these textures/material infos stay in memory
        material_manager->addSharedMaterial(materials_file);
        m_icon_material = material_manager->getMaterial(m_icon_file);
    }
    // Load model, except when called as part of --list-karts
    if(m_model_file.length()>0 && !dont_load_models)
    {
        m_model = loader->load(m_model_file, CB_KART, false);
        if(!m_model)
        {
            fprintf(stderr, "Can't find kart model '%s'.\n",m_model_file.c_str());
            file_manager->popTextureSearchPath();
            file_manager->popModelSearchPath();
            return;
        }
        ssgStripify(m_model);
        float x_min, x_max, y_min, y_max, z_min, z_max;
        MinMax(m_model, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
        if(getName()=="Hexley" || getName()=="Wilber")
        {
            // These kart models are too small, so we get problems with stability. 
            // Till we find either better (bigger) models or improve their physics 
            // parameters to become playable, we just adjust the size of their 
            // physical models to be the same as the tuxkart model
            x_min=-0.473799f;
            x_max= 0.486361f;
            y_min=-0.772244f;
            y_max= 0.739075f;
            z_min= 0.002806f;
            z_max= 0.701095f;
        }
        m_kart_width  = x_max - x_min;
        m_kart_length = y_max - y_min;
        m_kart_height = z_max - z_min;
        if(m_kart_length<1.2) m_kart_length=1.5f;
        m_model->ref();
    }  // if
    if(!dont_load_materials)
    {
        file_manager->popTextureSearchPath();
        file_manager->popModelSearchPath();
    }

}   // load

//-----------------------------------------------------------------------------
KartProperties::~KartProperties()
{
    ssgDeRefDelete(m_model);
}   // ~KartProperties

//-----------------------------------------------------------------------------
void KartProperties::getAllData(const lisp::Lisp* lisp)
{
    lisp->get("name",                    m_name);
    lisp->get("model-file",              m_model_file);
    lisp->get("icon-file",               m_icon_file);
    lisp->get("shadow-file",             m_shadow_file);
    lisp->get("rgb",                     m_color);

    lisp->get("wheel-base",              m_wheel_base);
    lisp->get("heightCOG",               m_height_cog);
    lisp->get("engine-power",            m_engine_power);
    lisp->get("time-full-steer",         m_time_full_steer);
    lisp->get("brake-factor",            m_brake_factor);
    lisp->get("mass",                    m_mass);
    lisp->get("max-steer-angle",         m_max_steer_angle);
    lisp->get("wheelie-max-speed-ratio", m_wheelie_max_speed_ratio );
    lisp->get("wheelie-max-pitch",       m_wheelie_max_pitch       );
    lisp->get("wheelie-pitch-rate",      m_wheelie_pitch_rate      );
    lisp->get("wheelie-restore-rate",    m_wheelie_restore_rate    );
    lisp->get("wheelie-speed-boost",     m_wheelie_speed_boost     );
    lisp->get("wheelie-lean-recovery",   m_wheelie_lean_recovery   );
    lisp->get("wheelie-step",            m_wheelie_step            );
    lisp->get("wheelie-balance-recovery",m_wheelie_balance_recovery);
    lisp->get("wheelie-power-boost",     m_wheelie_power_boost     );

    //bullet physics data
    lisp->get("suspension-stiffness",      m_suspension_stiffness     );
    lisp->get("wheel-damping-relaxation",  m_wheel_damping_relaxation );
    lisp->get("wheel-damping-compression", m_wheel_damping_compression);
    lisp->get("friction-slip",             m_friction_slip            );
    lisp->get("roll-influence",            m_roll_influence           );
    lisp->get("wheel-radius",              m_wheel_radius             );
    lisp->get("wheel-width",               m_wheel_width              );
    lisp->get("chassis-linear-damping",    m_chassis_linear_damping   );
    lisp->get("chassis-angular-damping",   m_chassis_angular_damping  );
    lisp->get("max-speed-reverse-ratio",   m_max_speed_reverse_ratio  );
    lisp->get("maximum-speed",             m_maximum_speed            );
    lisp->get("gravity-center-shift",      m_gravity_center_shift     );
    lisp->get("suspension-rest",           m_suspension_rest          );
    lisp->get("jump-velocity",             m_jump_velocity            );
    lisp->get("upright-tolerance",         m_upright_tolerance        );
    lisp->get("upright-max-force",         m_upright_max_force        );
    // getVector appends to existing vectors, so a new one must be used to load
    std::vector<float> temp;
    lisp->getVector("gear-switch-ratio",   temp);
    if(temp.size()>0) m_gear_switch_ratio = temp;
    temp.clear();
    lisp->getVector("gear-power-increase", temp);
    if(temp.size()>0) m_gear_power_increase = temp;
    
    // Camera
    lisp->get("camera-max-accel",             m_camera_max_accel);
    lisp->get("camera-max-brake",             m_camera_max_brake);
    lisp->get("camera-distance",              m_camera_distance );

}   // getAllData

//-----------------------------------------------------------------------------
void KartProperties::init_defaults()
{

    m_name          = "Tux";
    m_ident         = "tux";
    m_model_file    = "tuxkart.ac";
    m_icon_file     = "tuxicon.png";
    m_shadow_file   = "tuxkartshadow.png";

    m_color.setValue(1.0f, 0.0f, 0.0f);

    m_kart_width                = 1.0f;
    m_kart_length               = 1.5f;
    m_wheel_base                = stk_config->m_wheel_base;
    m_height_cog                = stk_config->m_height_cog;
    m_engine_power              = stk_config->m_engine_power;
    m_time_full_steer           = stk_config->m_time_full_steer;
    m_brake_factor              = stk_config->m_brake_factor;
    m_mass                      = stk_config->m_mass;
    m_max_steer_angle           = stk_config->m_max_steer_angle;
    m_wheelie_max_speed_ratio   = stk_config->m_wheelie_max_speed_ratio;
    m_wheelie_max_pitch         = stk_config->m_wheelie_max_pitch;
    m_wheelie_pitch_rate        = stk_config->m_wheelie_pitch_rate;
    m_wheelie_restore_rate      = stk_config->m_wheelie_restore_rate;
    m_wheelie_speed_boost       = stk_config->m_wheelie_speed_boost;
    m_wheelie_lean_recovery     = stk_config->m_wheelie_lean_recovery;
    m_wheelie_balance_recovery  = stk_config->m_wheelie_balance_recovery;
    m_wheelie_step              = stk_config->m_wheelie_step;
    m_wheelie_power_boost       = stk_config->m_wheelie_power_boost;

    //bullet physics data
    m_suspension_stiffness      = stk_config->m_suspension_stiffness;
    m_wheel_damping_relaxation  = stk_config->m_wheel_damping_relaxation;
    m_wheel_damping_compression = stk_config->m_wheel_damping_compression;
    m_friction_slip             = stk_config->m_friction_slip;
    m_roll_influence            = stk_config->m_roll_influence;
    m_wheel_radius              = stk_config->m_wheel_radius;
    m_wheel_width               = stk_config->m_wheel_width;
    m_chassis_linear_damping    = stk_config->m_chassis_linear_damping;
    m_chassis_angular_damping   = stk_config->m_chassis_angular_damping;
    m_maximum_speed             = stk_config->m_maximum_speed;
    m_max_speed_reverse_ratio   = stk_config->m_max_speed_reverse_ratio;
    m_gravity_center_shift      = stk_config->m_gravity_center_shift;
    m_suspension_rest           = stk_config->m_suspension_rest;
    m_jump_velocity             = stk_config->m_jump_velocity;
    m_gear_switch_ratio         = stk_config->m_gear_switch_ratio;
    m_gear_power_increase       = stk_config->m_gear_power_increase;
    m_upright_tolerance         = stk_config->getUprightTolerance();
    m_upright_max_force         = stk_config->getUprightMaxForce();
    m_camera_max_accel          = stk_config->getCameraMaxAccel();
    m_camera_max_brake          = stk_config->getCameraMaxBrake();
    m_camera_distance           = stk_config->getCameraDistance();
}   // init_defaults

/* EOF */
