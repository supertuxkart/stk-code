//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "kart_properties.hpp"

#include <iostream>
#include <stdexcept>
#include <plib/ssg.h>
#include "material_manager.hpp"
#include "loader.hpp"
#include "file_manager.hpp"
#include "stk_config.hpp"
#include "translation.hpp"
#include "user_config.hpp"
#include "karts/kart_model.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "utils/ssg_help.hpp"
#include "utils/string_utils.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

float KartProperties::UNDEFINED = -99.9f;

/** The constructor initialises all values with invalid values. It can later
 *  then be checked (for STKConfig) that all values are indeed defined.
 *  Otherwise the defaults are taken from STKConfig (and since they are all
 *  defined, it is guaranteed that each kart has well defined physics values.
 */
KartProperties::KartProperties() : m_icon_material(0)
{
    m_name          = "Tux";
    m_ident         = "tux";
    m_icon_file     = "tuxicon.png";
    m_shadow_file   = "tuxkartshadow.png";
    m_groups.clear();

    // Set all other values to undefined, so that it can later be tested
    // if everything is defined properly.
    m_mass = m_min_speed_turn = m_angle_at_min = 
        m_max_speed_turn = m_angle_at_max = m_brake_factor =
        m_engine_power[0] = m_engine_power[1] = m_engine_power[2] = 
        m_max_speed[0] = m_max_speed[1] = m_max_speed[2] = 
        m_time_full_steer = m_time_full_steer_ai = m_nitro_power_boost =
        m_suspension_stiffness = m_wheel_damping_relaxation = m_wheel_base =
        m_wheel_damping_compression = m_friction_slip = m_roll_influence = 
        m_wheel_radius = m_chassis_linear_damping = 
        m_chassis_angular_damping = m_suspension_rest = 
        m_max_speed_reverse_ratio = m_jump_velocity = m_upright_tolerance = 
        m_upright_max_force = m_suspension_travel_cm = 
        m_track_connection_accel = m_min_speed_turn = m_angle_at_min = 
        m_max_speed_turn = m_angle_at_max =
        m_rubber_band_max_length = m_rubber_band_force = 
        m_rubber_band_duration = m_time_till_max_skid =
        m_skid_decrease = m_skid_increase = m_skid_visual = m_skid_max =
        m_camera_max_accel = m_camera_max_brake = 
        m_camera_distance    = UNDEFINED;
    m_gravity_center_shift   = Vec3(UNDEFINED);
    m_has_skidmarks          = true;
    m_version                = 0;
    m_color.setValue(1.0f, 0.0f, 0.0f);
    m_engine_sfx_type = SFXManager::SOUND_ENGINE_SMALL;
}   // KartProperties

//-----------------------------------------------------------------------------
/** Destructor, dereferences the kart model. */
KartProperties::~KartProperties()
{
}   // ~KartProperties

//-----------------------------------------------------------------------------
/** Loads the kart properties from a file.
 *  \param filename Filename to load.
 *  \param node Name of the lisp node to load the data from 
 *              (default: tuxkart-kart)
 *  \param dont_load_models If set does not load the actual kart models, used
 *              when only printing kart information to stdout.
 */
void KartProperties::load(const std::string &filename, const std::string &node,
                          bool dont_load_models)
{

   // Get the default values from STKConfig:
   *this = stk_config->getDefaultKartProperties();

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

    // Set a default group (that has to happen after init_default and load)
    if(m_groups.size()==0)
        m_groups.push_back("standard");


    // Load material
    std::string materials_file = file_manager->getKartFile("materials.dat",getIdent());
    file_manager->pushModelSearchPath(file_manager->getKartFile("", getIdent()));
    file_manager->pushTextureSearchPath(file_manager->getKartFile("", getIdent()));

    // addShared makes sure that these textures/material infos stay in memory
    material_manager->addSharedMaterial(materials_file);
    m_icon_material = material_manager->getMaterial(m_icon_file);

    // Load model, except when called as part of --list-karts
    if(!dont_load_models)
    {
        // Only load the model if the .kart file has the appropriate version,
        // otherwise warnings are printed.
        if(m_version>=1)
            m_kart_model.loadModels();
        if(m_gravity_center_shift.getX()==UNDEFINED)
        {
            m_gravity_center_shift.setX(0);
            m_gravity_center_shift.setY(0);
            // Default: center at the very bottom of the kart.
            m_gravity_center_shift.setZ(m_kart_model.getHeight()*0.5f);
        }
        m_kart_model.setDefaultPhysicsPosition(m_gravity_center_shift,
                                               m_wheel_radius);
        m_wheel_base = fabsf( m_kart_model.getWheelPhysicsPosition(0).getY()
                             -m_kart_model.getWheelPhysicsPosition(2).getY());
        m_angle_at_min = asinf(m_wheel_base/m_min_radius);
        m_angle_at_max = asinf(m_wheel_base/m_max_radius);
        if(m_max_speed_turn == m_min_speed_turn)
            m_speed_angle_increase = 0.0;
        else
            m_speed_angle_increase = (m_angle_at_min   - m_angle_at_max)
                                   / (m_max_speed_turn - m_min_speed_turn);


        // Useful when tweaking kart parameters
        if(user_config->m_print_kart_sizes)
            printf("%s:\twidth: %f\tlength: %f\theight: %f\n",getIdent().c_str(), 
            m_kart_model.getWidth(), m_kart_model.getLength(),
            m_kart_model.getHeight());

    }  // if

    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath();

}   // load

//-----------------------------------------------------------------------------
void KartProperties::getAllData(const lisp::Lisp* lisp)
{
    lisp->get("version",                    m_version);
    // Only load the kart_model data if the .kart file has the appropriate
    if(m_version>=1)
        m_kart_model.loadInfo(lisp);
    lisp->get("name",                       m_name);
    lisp->get("icon-file",                  m_icon_file);
    lisp->get("shadow-file",                m_shadow_file);
    lisp->get("rgb",                        m_color);

    lisp->get("engine-power",               m_engine_power);
    lisp->get("time-full-steer",            m_time_full_steer);
    lisp->get("time-full-steer-ai",         m_time_full_steer_ai);
    lisp->get("brake-factor",               m_brake_factor);
    lisp->get("mass",                       m_mass);

    std::string sfx_type_string;
    lisp->get("engine-sound",                 sfx_type_string);
    if(sfx_type_string == "large")
    {
        m_engine_sfx_type = SFXManager::SOUND_ENGINE_LARGE;
    }
    else if(sfx_type_string == "small")
    {
        m_engine_sfx_type = SFXManager::SOUND_ENGINE_SMALL;
    }

    std::vector<float> v;
    if(lisp->getVector("max-speed-radius",      v))
    {
        if(v.size()!=2)
            printf("Incorrect max-speed-angle specifications for kart '%s'\n",
                   getIdent().c_str());
        else
        {
            m_max_speed_turn = v[0];
            m_max_radius     = v[1];
        }
    }
    v.clear();
    if(lisp->getVector("min-speed-radius",      v))
    {
        if(v.size()!=2)
            printf("Incorrect min-speed-angle specifications for kart '%s'\n",
                   getIdent().c_str());
        else
        {
            m_min_speed_turn = v[0];
            m_min_radius     = v[1];
        }
    }
                             
    lisp->get("nitro-power-boost",         m_nitro_power_boost       );

    //bullet physics data
    lisp->get("suspension-stiffness",      m_suspension_stiffness     );
    lisp->get("wheel-damping-relaxation",  m_wheel_damping_relaxation );
    lisp->get("wheel-damping-compression", m_wheel_damping_compression);
    lisp->get("friction-slip",             m_friction_slip            );
    lisp->get("roll-influence",            m_roll_influence           );
    lisp->get("wheel-radius",              m_wheel_radius             );
    lisp->get("chassis-linear-damping",    m_chassis_linear_damping   );
    lisp->get("chassis-angular-damping",   m_chassis_angular_damping  );
    lisp->get("max-speed-reverse-ratio",   m_max_speed_reverse_ratio  );
    lisp->get("max-speed",                 m_max_speed                );
    lisp->get("gravity-center-shift",      m_gravity_center_shift     );
    lisp->get("suspension-rest",           m_suspension_rest          );
    lisp->get("suspension-travel-cm",      m_suspension_travel_cm     );
    lisp->get("jump-velocity",             m_jump_velocity            );
    lisp->get("upright-tolerance",         m_upright_tolerance        );
    lisp->get("upright-max-force",         m_upright_max_force        );
    lisp->get("track-connection-accel",    m_track_connection_accel   );
    lisp->get("rubber-band-max-length",    m_rubber_band_max_length   );
    lisp->get("rubber-band-force",         m_rubber_band_force        );
    lisp->get("rubber-band-duration",      m_rubber_band_duration     );

    lisp->get("skid-increase",             m_skid_increase            );
    lisp->get("skid-decrease",             m_skid_decrease            );
    lisp->get("time-till-max-skid",        m_time_till_max_skid       );
    lisp->get("has-skidmarks",             m_has_skidmarks            );
    lisp->get("skid-max",                  m_skid_max                 );
    lisp->get("skid-visual",               m_skid_visual              );

    lisp->getVector("groups",              m_groups                   );

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
/** Checks if all necessary physics values are indeed defines. This helps
 *  finding bugs early, e.g. missing default in stk_config.dat file.
 *  \param filename File from which the data was read (only used to print
 *                  meaningful error messages).
 */
void KartProperties::checkAllSet(const std::string &filename)
{
    if(m_gear_switch_ratio.size()==0)
    {
        fprintf(stderr,"Missing default value for 'gear-switch-ratio' in '%s'.\n",
                filename.c_str());
        exit(-1);
    }
    if(m_gear_power_increase.size()==0)
    {
        fprintf(stderr,"Missing default value for 'gear-power-increase' in '%s'.\n",
                filename.c_str());
        exit(-1);
    }
    if(m_gear_switch_ratio.size()!=m_gear_power_increase.size())    {
        fprintf(stderr,"Number of entries for 'gear-switch-ratio' and 'gear-power-increase");
        fprintf(stderr,"in '%s' must be equal.\n", filename.c_str());
        exit(-1);
    }
#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                         \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n",    \
                strA,filename.c_str());exit(-1);                       \
    }

    CHECK_NEG(m_mass,                    "mass"                         );
    CHECK_NEG(m_engine_power[0],         "engine-power[0]"              );
    CHECK_NEG(m_engine_power[1],         "engine-power[1]"              );
    CHECK_NEG(m_engine_power[2],         "engine-power[2]"              );
    CHECK_NEG(m_min_speed_turn,          "min-speed-angle"              );
    CHECK_NEG(m_min_radius,              "min-speed-angle"              );
    CHECK_NEG(m_max_speed_turn,          "max-speed-angle"              );
    CHECK_NEG(m_max_radius,              "max-speed-angle"              );
    CHECK_NEG(m_brake_factor,            "brake-factor"                 );
    CHECK_NEG(m_time_full_steer,         "time-full-steer"              );
    CHECK_NEG(m_time_full_steer_ai,      "time-full-steer-ai"           );

    //bullet physics data
    CHECK_NEG(m_suspension_stiffness,      "suspension-stiffness"       );
    CHECK_NEG(m_wheel_damping_relaxation,  "wheel-damping-relaxation"   );
    CHECK_NEG(m_wheel_damping_compression, "wheel-damping-compression"  );
    CHECK_NEG(m_friction_slip,             "friction-slip"              );
    CHECK_NEG(m_roll_influence,            "roll-influence"             );
    CHECK_NEG(m_wheel_radius,              "wheel-radius"               );
    // Don't check m_wheel_base here, it is computed later!
    CHECK_NEG(m_chassis_linear_damping,    "chassis-linear-damping"     );
    CHECK_NEG(m_chassis_angular_damping,   "chassis-angular-damping"    );
    CHECK_NEG(m_max_speed[0],              "maximum-speed[0]"           );
    CHECK_NEG(m_max_speed[1],              "maximum-speed[1]"           );
    CHECK_NEG(m_max_speed[2],              "maximum-speed[2]"           );
    CHECK_NEG(m_max_speed_reverse_ratio,   "max-speed-reverse-ratio"    );
    CHECK_NEG(m_suspension_rest,           "suspension-rest"            );
    CHECK_NEG(m_suspension_travel_cm,      "suspension-travel-cm"       );
    CHECK_NEG(m_jump_velocity,             "jump-velocity"              );
    CHECK_NEG(m_upright_tolerance,         "upright-tolerance"          );
    CHECK_NEG(m_upright_max_force,         "upright-max-force"          );
    CHECK_NEG(m_track_connection_accel,    "track-connection-accel"     );
    CHECK_NEG(m_rubber_band_max_length,    "rubber-band-max-length"     );
    CHECK_NEG(m_rubber_band_force,         "rubber-band-force"          );
    CHECK_NEG(m_rubber_band_duration,      "rubber-band-duration"       );
    CHECK_NEG(m_skid_decrease,             "skid-decrease"              );
    CHECK_NEG(m_time_till_max_skid,        "time-till-max-skid"         );
    CHECK_NEG(m_skid_increase,             "skid-increase"              );
    CHECK_NEG(m_skid_max,                  "skid-max"                   );
    CHECK_NEG(m_skid_visual,               "skid-visual"                );

    CHECK_NEG(m_camera_max_accel,          "camera-max-accel"           );
    CHECK_NEG(m_camera_max_brake,          "camera-max-brake"           );
    CHECK_NEG(m_camera_distance,           "camera-distance"            );

}   // checkAllSet

// ----------------------------------------------------------------------------
float KartProperties::getMaxSteerAngle(float speed) const
{
    if(speed<=m_min_speed_turn) return m_angle_at_min;
    if(speed>=m_max_speed_turn) return m_angle_at_max;
    return m_angle_at_min - (speed-m_min_speed_turn)*m_speed_angle_increase;
}   // getMaxSteerAngle


/* EOF */
