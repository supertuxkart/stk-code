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

#include "karts/kart_properties.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_model.hpp"
#include "modes/world.hpp"
#include "io/xml_node.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

float KartProperties::UNDEFINED = -99.9f;

/** The constructor initialises all values with invalid values. It can later
 *  then be checked (for STKConfig) that all values are indeed defined.
 *  Otherwise the defaults are taken from STKConfig (and since they are all
 *  defined, it is guaranteed that each kart has well defined physics values).
 */
KartProperties::KartProperties(const std::string &filename)
{
    m_icon_material = NULL;
    m_minimap_icon  = NULL;
    m_name          = "NONAME";
    m_ident         = "NONAME";
    m_icon_file     = "";
    m_shadow_file   = "";

    m_groups.clear();
    m_custom_sfx_id.resize(SFXManager::NUM_CUSTOMS);

    // Set all other values to undefined, so that it can later be tested
    // if everything is defined properly.
    m_mass = m_min_speed_turn = m_angle_at_min =
        m_max_speed_turn = m_angle_at_max = m_brake_factor =
        m_engine_power[0] = m_engine_power[1] = m_engine_power[2] =
        m_max_speed[0] = m_max_speed[1] = m_max_speed[2] =
        m_time_full_steer = m_time_full_steer_ai =
        m_nitro_power_boost = m_nitro_consumption =
        m_nitro_small_container = m_nitro_big_container =
        m_suspension_stiffness = m_wheel_damping_relaxation = m_wheel_base =
        m_wheel_damping_compression = m_friction_slip = m_roll_influence =
        m_wheel_radius = m_chassis_linear_damping =
        m_chassis_angular_damping = m_suspension_rest =
        m_max_speed_reverse_ratio = m_jump_velocity =
        m_rescue_vert_offset = m_upright_tolerance = m_collision_side_impulse =
        m_upright_max_force = m_suspension_travel_cm =
        m_track_connection_accel = m_min_speed_turn = m_angle_at_min =
        m_max_speed_turn = m_angle_at_max =
        m_rubber_band_max_length = m_rubber_band_force =
        m_rubber_band_duration = m_time_till_max_skid =
        m_skid_decrease = m_skid_increase = m_skid_visual = m_skid_max =
        m_slipstream_length = m_slipstream_collect_time = 
        m_slipstream_use_time = m_slipstream_add_power =
        m_slipstream_min_speed = m_camera_distance = m_camera_up_angle =
        m_rescue_time = m_rescue_height = m_explosion_time =
        m_explosion_radius = m_ai_steering_variation = UNDEFINED;
    m_gravity_center_shift   = Vec3(UNDEFINED);
    m_has_skidmarks          = true;
    m_version                = 0;
    m_color                  = video::SColor(255, 0, 0, 0);
    m_shape                  = 32;  // close enough to a circle.
    m_engine_sfx_type        = "engine_small";
    m_kart_model             = NULL;
    // The default constructor for stk_config uses filename=""
    if (filename != "") load(filename, "kart");
}   // KartProperties

//-----------------------------------------------------------------------------
/** Destructor, dereferences the kart model. */
KartProperties::~KartProperties()
{
}   // ~KartProperties

//-----------------------------------------------------------------------------
/** Loads the kart properties from a file.
 *  \param filename Filename to load.
 *  \param node Name of the xml node to load the data from
 */
void KartProperties::load(const std::string &filename, const std::string &node)
{
    // Get the default values from STKConfig:
    *this = stk_config->getDefaultKartProperties();
    // m_kart_model must be initialised after assigning the default
    // values from stk_config (otherwise all kart_properties will
    // share the same KartModel
    m_kart_model  = new KartModel(/*is_master*/true);

    const XMLNode * root = 0;
    m_root  = StringUtils::getPath(filename);
    m_ident = StringUtils::getBasename(StringUtils::removeExtension(filename));
    m_ident = StringUtils::getBasename(StringUtils::getPath(filename));
    try
    {
        root = new XMLNode(filename);
        if(!root || root->getName()!="kart")
        {
            std::ostringstream msg;
            msg << "Couldn't load kart properties '" << filename <<
                "': no kart node.";
            throw std::runtime_error(msg.str());
        }
        getAllData(root);
    }
    catch(std::exception& err)
    {
        fprintf(stderr, "Error while parsing KartProperties '%s':\n",
                filename.c_str());
        fprintf(stderr, "%s\n", err.what());
    }
    if(root) delete root;

    // Set a default group (that has to happen after init_default and load)
    if(m_groups.size()==0)
        m_groups.push_back(DEFAULT_GROUP_NAME);


    // Load material
    std::string materials_file = file_manager->getKartFile("materials.xml",getIdent());
    file_manager->pushModelSearchPath(file_manager->getKartFile("", getIdent()));
    file_manager->pushTextureSearchPath(file_manager->getKartFile("", getIdent()));

    // addShared makes sure that these textures/material infos stay in memory
    material_manager->addSharedMaterial(materials_file);
    // Make permanent is important, since otherwise icons can get deleted
    // (e.g. when freeing temp. materials from a track, the last icon
    //  would get deleted, too.
    m_icon_material = material_manager->getMaterial(m_icon_file,
                                                    /*is_full+path*/false, 
                             
                                                    /*make_permanent*/true);
    if(m_minimap_icon_file!="")
        m_minimap_icon = irr_driver->getTexture(m_minimap_icon_file);
    else
        m_minimap_icon = NULL;

    // Only load the model if the .kart file has the appropriate version,
    // otherwise warnings are printed.
    if(m_version>=1)
        m_kart_model->loadModels(*this);
    if(m_gravity_center_shift.getX()==UNDEFINED)
    {
        m_gravity_center_shift.setX(0);
        // Default: center at the very bottom of the kart.
        m_gravity_center_shift.setY(m_kart_model->getHeight()*0.5f);
        m_gravity_center_shift.setZ(0);
    }
    m_kart_model->setDefaultPhysicsPosition(m_gravity_center_shift,
                                           m_wheel_radius           );
    m_wheel_base = fabsf( m_kart_model->getWheelPhysicsPosition(0).getZ()
                         -m_kart_model->getWheelPhysicsPosition(2).getZ());
    m_angle_at_min = asinf(m_wheel_base/m_min_radius);
    m_angle_at_max = asinf(m_wheel_base/m_max_radius);
    if(m_max_speed_turn == m_min_speed_turn)
        m_speed_angle_increase = 0.0;
    else
        m_speed_angle_increase = (m_angle_at_min   - m_angle_at_max)
                               / (m_max_speed_turn - m_min_speed_turn);


    m_shadow_texture = irr_driver->getTexture(m_shadow_file);
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath();

}   // load

//-----------------------------------------------------------------------------
/** Actually reads in the data from the xml file.
 *  \param root Root of the xml tree.
 */
void KartProperties::getAllData(const XMLNode * root)
{
    root->get("version", &m_version);
    
    std::string temp_name;
    root->get("name",              &temp_name          );
    m_name = _(temp_name.c_str());
    
    root->get("icon-file",         &m_icon_file        );
    
    root->get("minimap-icon-file", &m_minimap_icon_file);

    root->get("shadow-file",       &m_shadow_file      );
    Vec3 c;
    root->get("rgb",               &c                  );
    m_color.set(255, (int)(255*c.getX()), (int)(255*c.getY()), (int)(255*c.getZ()));

    root->get("groups",            &m_groups           );

    if(const XMLNode *dimensions_node = root->getNode("center"))
        dimensions_node->get("gravity-shift", &m_gravity_center_shift);

    if(const XMLNode *nitro_node = root->getNode("nitro"))
    {
        nitro_node->get("power-boost",     &m_nitro_power_boost    );
        nitro_node->get("consumption",     &m_nitro_consumption    );
        nitro_node->get("small-container", &m_nitro_small_container);
        nitro_node->get("big-container",   &m_nitro_big_container  );
    }

    if(const XMLNode *rescue_node = root->getNode("rescue"))
    {
        rescue_node->get("vert-offset", &m_rescue_vert_offset);
        rescue_node->get("time",        &m_rescue_time       );
        rescue_node->get("height",      &m_rescue_height     );
    }

    if(const XMLNode *explosion_node = root->getNode("explosion"))
    {
        explosion_node->get("time",   &m_explosion_time  );
        explosion_node->get("radius", &m_explosion_radius);
    }

    if(const XMLNode *ai_node = root->getNode("ai"))
    {
        ai_node->get("steering-variation",   &m_ai_steering_variation  );
    }
    if(const XMLNode *skid_node = root->getNode("skid"))
    {
        skid_node->get("increase",      &m_skid_increase     );
        skid_node->get("decrease",      &m_skid_decrease     );
        skid_node->get("max",           &m_skid_max          );
        skid_node->get("time-till-max", &m_time_till_max_skid);
        skid_node->get("visual",        &m_skid_visual       );
        skid_node->get("enable",        &m_has_skidmarks     );
    }

    if(const XMLNode *slipstream_node = root->getNode("slipstream"))
    {
        slipstream_node->get("length",       &m_slipstream_length      );
        slipstream_node->get("collect-time", &m_slipstream_collect_time);
        slipstream_node->get("use-time",     &m_slipstream_use_time    );
        slipstream_node->get("add-power",    &m_slipstream_add_power   );
        slipstream_node->get("min-speed",    &m_slipstream_min_speed   );
    }

    if(const XMLNode *turn_node = root->getNode("turn"))
    {
        turn_node->get("time-full-steer",      &m_time_full_steer   );
        turn_node->get("time-full-steer-ai",   &m_time_full_steer_ai);
        std::vector<float> v;
        if(turn_node->get("min-speed-radius", &v))
        {
            if(v.size()!=2)
                printf("Incorrect min-speed-radius specifications for kart '%s'\n",
                getIdent().c_str());
            else
            {
                m_min_speed_turn = v[0];
                m_min_radius     = v[1];
            }
        }

        v.clear();
        if(turn_node->get("max-speed-radius", &v))
        {
            if(v.size()!=2)
                printf("Incorrect max-speed-radius specifications for kart '%s'\n",
                getIdent().c_str());
            else
            {
                m_max_speed_turn = v[0];
                m_max_radius     = v[1];
            }
        }
    }   // if turn_node

    if(const XMLNode *engine_node = root->getNode("engine"))
    {
        engine_node->get("brake-factor", &m_brake_factor);
        engine_node->get("max-speed-reverse-ratio", &m_max_speed_reverse_ratio);
        std::vector<float> v;
        if( engine_node->get("power", &v))
        {
            if(v.size()!=3)
                printf("Incorrect engine-power specifications for kart '%s'\n",
                getIdent().c_str());
            else
            {
                m_engine_power[0] = v[0];
                m_engine_power[1] = v[1];
                m_engine_power[2] = v[2];
            }
        }   // if engine-power
        v.clear();
        if( engine_node->get("max-speed", &v))
        {
            if(v.size()!=3)
                printf("Incorrect max-speed specifications for kart '%s'\n",
                getIdent().c_str());
            else
            {
                m_max_speed[0] = v[0];
                m_max_speed[1] = v[1];
                m_max_speed[2] = v[2];
            }
        }   // if max-speed
    }   // if getNode("engine")

    if(const XMLNode *gear_node = root->getNode("gear"))
    {
        gear_node->get("switch-ratio",   &m_gear_switch_ratio  );
        gear_node->get("power-increase", &m_gear_power_increase);
    }

    if(const XMLNode *mass_node = root->getNode("mass"))
        mass_node->get("value", &m_mass);


    if(const XMLNode *suspension_node = root->getNode("suspension"))
    {
        suspension_node->get("stiffness", &m_suspension_stiffness);
        suspension_node->get("rest",      &m_suspension_rest     );
        suspension_node->get("travel-cm", &m_suspension_travel_cm);
    }

    if(const XMLNode *wheels_node = root->getNode("wheels"))
    {
        wheels_node->get("damping-relaxation",  &m_wheel_damping_relaxation );
        wheels_node->get("damping-compression", &m_wheel_damping_compression);
        wheels_node->get("radius",              &m_wheel_radius             );
    }

    if(const XMLNode *friction_node = root->getNode("friction"))
        friction_node->get("slip", &m_friction_slip);

    if(const XMLNode *stability_node = root->getNode("stability"))
    {
        stability_node->get("roll-influence",          &m_roll_influence);
        stability_node->get("chassis-linear-damping",  &m_chassis_linear_damping);
        stability_node->get("chassis-angular-damping", &m_chassis_angular_damping);
    }

    if(const XMLNode *upright_node = root->getNode("upright"))
    {
        upright_node->get("tolerance", &m_upright_tolerance);
        upright_node->get("max-force", &m_upright_max_force);
    }

    if(const XMLNode *track_connection_node = root->getNode("track-connection-accel"))
        track_connection_node->get("value", &m_track_connection_accel);

    if(const XMLNode *jump_node = root->getNode("jump"))
        jump_node->get("velocity", &m_jump_velocity);

    if(const XMLNode *collision_node = root->getNode("collision"))
        collision_node->get("side-impulse",  &m_collision_side_impulse);

    //TODO: wheel front right and wheel front left is not loaded, yet is listed as an attribute in the xml file after wheel-radius
    //TODO: same goes for their rear equivalents

    if(const XMLNode *rubber_band_node= root->getNode("rubber-band"))
    {
        rubber_band_node->get("max-length", &m_rubber_band_max_length);
        rubber_band_node->get("force",      &m_rubber_band_force     );
        rubber_band_node->get("duration",   &m_rubber_band_duration  );
    }

    if(const XMLNode *camera_node= root->getNode("camera"))
    {
        camera_node->get("distance", &m_camera_distance);
        camera_node->get("up-angle", &m_camera_up_angle);
        m_camera_up_angle *= DEGREE_TO_RAD;
    }

    if(const XMLNode *startup_node= root->getNode("startup"))
    {
        startup_node->get("time", &m_startup_times);
        startup_node->get("boost", &m_startup_boost);
    }

    if(const XMLNode *sounds_node= root->getNode("sounds"))
    {
        std::string s;
        sounds_node->get("engine", &s);
        if      (s == "large") m_engine_sfx_type = "engine_large";
        else if (s== "small")  m_engine_sfx_type = "engine_small";

#ifdef WILL_BE_ENABLED_ONCE_DONE_PROPERLY
        // Load custom kart SFX files (TODO: enable back when it's implemented properly)
        for (int i = 0; i < SFXManager::NUM_CUSTOMS; i++)
        {
            std::string tempFile;
            // Get filename associated with each custom sfx tag in sfx config
            if (sounds_node->get(sfx_manager->getCustomTagName(i), tempFile))
            {
                // determine absolute filename
                // FIXME: will not work with add-on packs (is data dir the same)?
                tempFile = file_manager->getKartFile(tempFile, getIdent());

                // Create sfx in sfx manager and store id
                m_custom_sfx_id[i] = sfx_manager->addSingleSfx(tempFile, 1, 0.2f,1.0f);
            }
            else
            {
                // if there is no filename associated with a given tag
                m_custom_sfx_id[i] = -1;
            }   // if custom sound
        }   // for i<SFXManager::NUM_CUSTOMS
#endif
    }   // if sounds-node exist

    if(m_kart_model)
        m_kart_model->loadInfo(*root);
}   // getAllData

// ----------------------------------------------------------------------------
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
        fprintf(stderr,"Number of entries for 'gear-switch-ratio' and 'gear-power-increase\n");
        fprintf(stderr,"in '%s' must be equal.\n", filename.c_str());
        exit(-1);
    }
    if(m_startup_boost.size()!=m_startup_times.size())
    {
        fprintf(stderr, "Number of entried for 'startup times' and 'startup-boost\n");
        fprintf(stderr, "must be identical.\n");
        exit(-1);
    }
#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                         \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n",    \
                strA,filename.c_str());exit(-1);                       \
    }

    CHECK_NEG(m_mass,                      "mass"                          );
    CHECK_NEG(m_min_speed_turn,            "turn min-speed-angle"          );
    CHECK_NEG(m_min_radius,                "turn min-speed-angle"          );
    CHECK_NEG(m_max_speed_turn,            "turn max-speed-angle"          );
    CHECK_NEG(m_max_radius,                "turn max-speed-angle"          );
    CHECK_NEG(m_time_full_steer,           "turn time-full-steer"          );
    CHECK_NEG(m_time_full_steer_ai,        "turn time-full-steer-ai"       );
    CHECK_NEG(m_wheel_damping_relaxation,  "wheels damping-relaxation"     );
    CHECK_NEG(m_wheel_damping_compression, "wheels damping-compression"    );
    CHECK_NEG(m_wheel_radius,              "wheels radius"                 );
    CHECK_NEG(m_friction_slip,             "friction slip"                 );
    CHECK_NEG(m_roll_influence,            "stability roll-influence"      );
    CHECK_NEG(m_chassis_linear_damping,    "stability chassis-linear-damping");
    CHECK_NEG(m_chassis_angular_damping,   "stability chassis-angular-damping");
    CHECK_NEG(m_engine_power[0],           "engine power[0]"               );
    CHECK_NEG(m_engine_power[1],           "engine power[1]"               );
    CHECK_NEG(m_engine_power[2],           "engine power[2]"               );
    CHECK_NEG(m_max_speed[0],              "engine maximum-speed[0]"       );
    CHECK_NEG(m_max_speed[1],              "engine maximum-speed[1]"       );
    CHECK_NEG(m_max_speed[2],              "engine maximum-speed[2]"       );
    CHECK_NEG(m_max_speed_reverse_ratio,   "engine max-speed-reverse-ratio");
    CHECK_NEG(m_brake_factor,              "engine brake-factor"           );
    CHECK_NEG(m_suspension_stiffness,      "suspension stiffness"          );
    CHECK_NEG(m_suspension_rest,           "suspension rest"               );
    CHECK_NEG(m_suspension_travel_cm,      "suspension travel-cm"          );
    CHECK_NEG(m_collision_side_impulse,    "collision side-impulse"        );
    CHECK_NEG(m_jump_velocity,             "jump velocity"                 );
    CHECK_NEG(m_upright_tolerance,         "upright tolerance"             );
    CHECK_NEG(m_upright_max_force,         "upright max-force"             );
    CHECK_NEG(m_track_connection_accel,    "track-connection-accel"        );
    CHECK_NEG(m_rubber_band_max_length,    "rubber-band max-length"        );
    CHECK_NEG(m_rubber_band_force,         "rubber-band force"             );
    CHECK_NEG(m_rubber_band_duration,      "rubber-band duration"          );
    CHECK_NEG(m_skid_decrease,             "skid decrease"                 );
    CHECK_NEG(m_time_till_max_skid,        "skid time-till-max"            );
    CHECK_NEG(m_skid_increase,             "skid increase"                 );
    CHECK_NEG(m_skid_max,                  "skid max"                      );
    CHECK_NEG(m_skid_visual,               "skid visual"                   );
    CHECK_NEG(m_slipstream_length,         "slipstream length"             );
    CHECK_NEG(m_slipstream_collect_time,   "slipstream collect-time"       );
    CHECK_NEG(m_slipstream_use_time,       "slipstream use-time"           );
    CHECK_NEG(m_slipstream_add_power,      "slipstream add-power"          );
    CHECK_NEG(m_slipstream_min_speed,      "slipstream min-speed"          );
    CHECK_NEG(m_camera_distance,           "camera distance"               );
    CHECK_NEG(m_camera_up_angle,           "camera up-angle"               );
    CHECK_NEG(m_nitro_power_boost,         "nitro power-boost"             );
    CHECK_NEG(m_nitro_consumption,         "nitro consumption"             );
    CHECK_NEG(m_nitro_big_container,       "nitro big-container"           );
    CHECK_NEG(m_nitro_small_container,     "nitro small-container"         );
    CHECK_NEG(m_rescue_height,             "rescue height"                 );
    CHECK_NEG(m_rescue_time,               "rescue time"                   );
    CHECK_NEG(m_rescue_vert_offset,        "rescue vert-offset"            );
    CHECK_NEG(m_explosion_time,            "explosion time"                );
    CHECK_NEG(m_explosion_radius,          "explosion radius"              );
    CHECK_NEG(m_ai_steering_variation,     "ai steering-variation"         );

}   // checkAllSet

// ----------------------------------------------------------------------------
float KartProperties::getMaxSteerAngle(float speed) const
{
    if(speed<=m_min_speed_turn) return m_angle_at_min;
    if(speed>=m_max_speed_turn) return m_angle_at_max;
    return m_angle_at_min - (speed-m_min_speed_turn)*m_speed_angle_increase;
}   // getMaxSteerAngle

// ----------------------------------------------------------------------------
/** Called the first time a kart accelerates after 'ready-set-go'. It searches
 *  through m_startup_times to find the appropriate slot, and returns the
 *  speed-boost from the corresponding entry in m_startup_boost.
 *  If the kart started too slow (i.e. slower than the longest time in
 *  m_startup_times, it returns 0.
 */
float KartProperties::getStartupBoost() const
{
    float t = World::getWorld()->getTime();
    for(unsigned int i=0; i<m_startup_times.size(); i++)
    {
        if(t<=m_startup_times[i]) return m_startup_boost[i];
    }
    return 0;
}   // getStartupBoost
/* EOF */
