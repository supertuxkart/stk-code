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
#include "preprocessor.hpp"
#include "string_utils.hpp"
#include "kart_properties.hpp"
#include "physics_parameters.hpp"

// This constructor would be a bit more useful, nicer, if we could call
// init_defaults() and load from here. Unfortunately, this object is used
// as a base class for PhysicsParameters, which has to overwrite
// init_defaults() and getAllData(). But during the call of this constructor,
// the PhysicsParameters object does not (yet) exist, so the overwriting
// functions do NOT get called, only the virtual functions here would be
// called. Therefore, a two step initialisation is necessary: the constructor
// doing not much, but then in load the overwriting functions can be used.
KartProperties::KartProperties() : m_icon_material(0), m_model(0)
{}   // KartProperties

//-----------------------------------------------------------------------------
void KartProperties::load(const std::string filename, const std::string node)
{

    init_defaults();

    const lisp::Lisp* root = 0;
    m_ident = StringUtils::basename(StringUtils::without_extension(filename));

    try
    {
        lisp::Parser parser;
        root = parser.parse(loader->getPath(filename));

        const lisp::Lisp* const LISP = root->getLisp(node);
        if(!LISP)
        {
            std::string s="No '";
            s+=node;
            s+="' node found";
            throw std::runtime_error(s);
        }
        getAllData(LISP);
    }
    catch(std::exception& err)
    {
        std::cout << "Error while parsing KartProperties '" << filename
        << ": " << err.what() << "\n";
    }
    delete root;

    // Load material
    m_icon_material = material_manager->getMaterial(m_icon_file);

    // Load model
    if(m_model_file.length()>0)
    {
        m_model = loader->load(m_model_file, CB_KART, false);
        ssgStripify(m_model);
        preProcessObj(m_model, 0);
        m_model->ref();
    }  // if

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
    lisp->get("red",                     m_color[0]);
    lisp->get("green",                   m_color[1]);
    lisp->get("blue",                    m_color[2]);

    lisp->get("wheel-base",              m_wheel_base);
    lisp->get("heightCOG",               m_height_cog);
    lisp->get("engine-power",            m_engine_power);
    lisp->get("time-full-steer",         m_time_full_steer);
    lisp->get("brake-factor",            m_brake_factor);
    lisp->get("roll-resistance",         m_roll_resistance);
    lisp->get("mass",                    m_mass);
    lisp->get("air-resistance",          m_air_resistance);
    lisp->get("tire-grip",               m_tire_grip);
    lisp->get("max-steer-angle",         m_max_steer_angle);
    lisp->get("corn-f",                  m_corn_f);
    lisp->get("corn-r",                  m_corn_r);
    lisp->get("inertia",                 m_inertia);
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
    lisp->get("maximum-velocity",          m_maximum_velocity         );

}   // getAllData

//-----------------------------------------------------------------------------
void KartProperties::init_defaults()
{

    m_name          = "Tux";
    m_ident         = "tux";
    m_model_file    = "tuxkart.ac";
    m_icon_file     = "tuxicon.png";
    m_shadow_file   = "tuxkartshadow.png";

    m_color[0] = 1.0f; m_color[1] = 0.0f; m_color[2] = 0.0f;

    m_wheel_base           = physicsParameters->m_wheel_base;
    m_height_cog           = physicsParameters->m_height_cog;
    m_engine_power         = physicsParameters->m_engine_power;
    m_time_full_steer      = physicsParameters->m_time_full_steer;
    m_brake_factor         = physicsParameters->m_brake_factor;
    m_roll_resistance      = physicsParameters->m_roll_resistance;
    m_mass                 = physicsParameters->m_mass;
    m_air_resistance       = physicsParameters->m_air_resistance;
    m_tire_grip            = physicsParameters->m_tire_grip;
    m_max_steer_angle      = physicsParameters->m_max_steer_angle;
    m_corn_f               = physicsParameters->m_corn_f;
    m_corn_r               = physicsParameters->m_corn_r;
    m_inertia              = physicsParameters->m_inertia;
    m_wheelie_max_speed_ratio = physicsParameters->m_wheelie_max_speed_ratio;
    m_wheelie_max_pitch       = physicsParameters->m_wheelie_max_pitch;
    m_wheelie_pitch_rate      = physicsParameters->m_wheelie_pitch_rate;
    m_wheelie_restore_rate    = physicsParameters->m_wheelie_restore_rate;
    m_wheelie_speed_boost     = physicsParameters->m_wheelie_speed_boost;
    m_wheelie_lean_recovery   = physicsParameters->m_wheelie_lean_recovery;
    m_wheelie_balance_recovery= physicsParameters->m_wheelie_balance_recovery;
    m_wheelie_step            = physicsParameters->m_wheelie_step;
    m_wheelie_power_boost     = physicsParameters->m_wheelie_power_boost;

    //bullet physics data
    m_suspension_stiffness      = physicsParameters->m_suspension_stiffness;
    m_wheel_damping_relaxation  = physicsParameters->m_wheel_damping_relaxation;
    m_wheel_damping_compression = physicsParameters->m_wheel_damping_compression;
    m_friction_slip             = physicsParameters->m_friction_slip;
    m_roll_influence            = physicsParameters->m_roll_influence;
    m_wheel_radius              = physicsParameters->m_wheel_radius;
    m_wheel_width               = physicsParameters->m_wheel_width;
    m_chassis_linear_damping    = physicsParameters->m_chassis_linear_damping;
    m_chassis_angular_damping   = physicsParameters->m_chassis_angular_damping;
    m_maximum_velocity          = physicsParameters->m_maximum_velocity;


}   // init_defaults

/* EOF */
