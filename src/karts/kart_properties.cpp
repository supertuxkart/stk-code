//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#include "addons/addon.hpp"
#include "config/stk_config.hpp"
#include "config/player_manager.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_model.hpp"
#include "karts/skidding_properties.hpp"
#include "modes/world.hpp"
#include "io/xml_node.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <stdexcept>
#include <string>


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
    m_shadow_scale    = 1.0f;
    m_shadow_x_offset = 0.0f;
    m_shadow_z_offset = 0.0f;

    m_groups.clear();
    m_custom_sfx_id.resize(SFXManager::NUM_CUSTOMS);

    // Set all other values to undefined, so that it can later be tested
    // if everything is defined properly.
    m_mass = m_brake_factor = m_brake_time_increase =
        m_time_reset_steer = m_nitro_consumption = m_nitro_engine_force =
        m_nitro_small_container = m_nitro_big_container = m_nitro_max =
        m_nitro_max_speed_increase = m_nitro_duration = m_nitro_fade_out_time =
        m_suspension_stiffness = m_wheel_damping_relaxation = m_wheel_base =
        m_wheel_damping_compression = m_friction_slip = m_roll_influence =
        m_wheel_radius = m_chassis_linear_damping = m_max_suspension_force =
        m_chassis_angular_damping = m_suspension_rest =
        m_max_speed_reverse_ratio = m_rescue_vert_offset =
        m_collision_terrain_impulse = m_collision_impulse = m_restitution =
        m_collision_impulse_time = m_suspension_travel_cm =
        m_track_connection_accel = m_rubber_band_max_length =
        m_rubber_band_force = m_rubber_band_duration =
        m_rubber_band_speed_increase = m_rubber_band_fade_out_time =
        m_zipper_time = m_zipper_force = m_zipper_speed_gain =
        m_zipper_max_speed_increase = m_zipper_fade_out_time =
        m_slipstream_length = m_slipstream_width = m_slipstream_collect_time =
        m_slipstream_use_time = m_slipstream_add_power =
        m_slipstream_min_speed = m_slipstream_max_speed_increase =
        m_slipstream_duration = m_slipstream_fade_out_time =
        m_camera_distance = m_camera_forward_up_angle =
        m_camera_backward_up_angle = m_explosion_invulnerability_time =
        m_rescue_time = m_rescue_height = m_explosion_time =
        m_explosion_radius = m_max_lean = m_lean_speed =
        m_swatter_distance2 = m_swatter_duration = m_squash_slowdown =
        m_squash_duration = m_downward_impulse_factor =
        m_bubblegum_fade_in_time = m_bubblegum_speed_fraction =
        m_bubblegum_time = m_bubblegum_torque = m_jump_animation_time =
        m_smooth_flying_impulse = m_physical_wheel_position = 
        m_graphical_y_offset =
            UNDEFINED;

    m_engine_power.resize(RaceManager::DIFFICULTY_COUNT, UNDEFINED);
    m_max_speed.resize(RaceManager::DIFFICULTY_COUNT, UNDEFINED);
    m_plunger_in_face_duration.resize(RaceManager::DIFFICULTY_COUNT,
                                      UNDEFINED);

    m_terrain_impulse_type   = IMPULSE_NONE;
    m_gravity_center_shift   = Vec3(UNDEFINED);
    m_bevel_factor           = Vec3(UNDEFINED);
    m_exp_spring_response    = false;
    m_version                = 0;
    m_color                  = video::SColor(255, 0, 0, 0);
    m_shape                  = 32;  // close enough to a circle.
    m_engine_sfx_type        = "engine_small";
    m_kart_model             = NULL;
    m_has_rand_wheels        = false;
    m_nitro_min_consumption  = 0.53f;
    // The default constructor for stk_config uses filename=""
    if (filename != "")
    {
        m_skidding_properties = NULL;
        for(unsigned int i=0; i<RaceManager::DIFFICULTY_COUNT; i++)
            m_ai_properties[i]= NULL;
        load(filename, "kart");
    }
    else
    {
        m_skidding_properties = new SkiddingProperties();
        for(unsigned int i=0; i<RaceManager::DIFFICULTY_COUNT; i++)
            m_ai_properties[i]= new AIProperties((RaceManager::Difficulty)i);
    }
}   // KartProperties

//-----------------------------------------------------------------------------
/** Destructor, dereferences the kart model. */
KartProperties::~KartProperties()
{
    delete m_kart_model;
    if(m_skidding_properties)
        delete m_skidding_properties;
    for(unsigned int i=0; i<RaceManager::DIFFICULTY_COUNT; i++)
        if(m_ai_properties[i])
            delete m_ai_properties[i];
}   // ~KartProperties

//-----------------------------------------------------------------------------
/** Copies this KartProperties to another one. Importnat: if you add any
 *  pointers to kart_properties, the data structure they are pointing to
 *  need to be copied here explicitely!
 *  \param source The source kart properties from which to copy this objects'
 *         values.
 */
void KartProperties::copyFrom(const KartProperties *source)
{
    *this = *source;

    // After the memcpy any pointers will be shared.
    // So all pointer variables need to be separately allocated and assigned.
    m_skidding_properties = new SkiddingProperties();
    assert(m_skidding_properties);
    *m_skidding_properties = *source->m_skidding_properties;

    for(unsigned int i=0; i<RaceManager::DIFFICULTY_COUNT; i++)
    {
        m_ai_properties[i] = new AIProperties((RaceManager::Difficulty)i);
        assert(m_ai_properties);
        *m_ai_properties[i] = *source->m_ai_properties[i];
    }
}   // copyFrom

//-----------------------------------------------------------------------------
/** Loads the kart properties from a file.
 *  \param filename Filename to load.
 *  \param node Name of the xml node to load the data from
 */
void KartProperties::load(const std::string &filename, const std::string &node)
{
    // Get the default values from STKConfig. This will also allocate any
    // pointers used in KartProperties

    const XMLNode* root = new XMLNode(filename);
    std::string kart_type;
    if (root->get("type", &kart_type))
        copyFrom(&stk_config->getKartProperties(kart_type));
    else
        copyFrom(&stk_config->getDefaultKartProperties());

    // m_kart_model must be initialised after assigning the default
    // values from stk_config (otherwise all kart_properties will
    // share the same KartModel
    m_kart_model  = new KartModel(/*is_master*/true);

    m_root  = StringUtils::getPath(filename)+"/";
    m_ident = StringUtils::getBasename(StringUtils::getPath(filename));
    // If this is an addon kart, add "addon_" to the identifier - just in
    // case that an addon kart has the same directory name (and therefore
    // identifier) as an included kart.
    if(Addon::isAddon(filename))
        m_ident = Addon::createAddonId(m_ident);
    try
    {
        if(!root || root->getName()!="kart")
        {
            std::ostringstream msg;
            msg << "Couldn't load kart properties '" << filename <<
                "': no kart node.";

            delete m_kart_model;
            throw std::runtime_error(msg.str());
        }
        getAllData(root);
    }
    catch(std::exception& err)
    {
        Log::error("[KartProperties]", "Error while parsing KartProperties '%s':",
                   filename.c_str());
        Log::error("[KartProperties]", "%s", err.what());
    }
    if(root) delete root;

    // Set a default group (that has to happen after init_default and load)
    if(m_groups.size()==0)
        m_groups.push_back(DEFAULT_GROUP_NAME);


    // Load material
    std::string materials_file = m_root+"materials.xml";
    file_manager->pushModelSearchPath  (m_root);
    file_manager->pushTextureSearchPath(m_root);

    irr_driver->setTextureErrorMessage("Error while loading kart '%s':",
                                       m_name);

    // addShared makes sure that these textures/material infos stay in memory
    material_manager->addSharedMaterial(materials_file);

    m_icon_file = m_root+m_icon_file;

    // Make permanent is important, since otherwise icons can get deleted
    // (e.g. when freeing temp. materials from a track, the last icon
    //  would get deleted, too.
    m_icon_material = material_manager->getMaterial(m_icon_file,
                                                    /*is_full_path*/true,
                                                    /*make_permanent*/true,
                                                    /*complain_if_not_found*/true,
                                                    /*strip_path*/false);
    if(m_minimap_icon_file!="")
        m_minimap_icon = irr_driver->getTexture(m_root+m_minimap_icon_file);
    else
        m_minimap_icon = NULL;

    if (m_minimap_icon == NULL)
    {
        m_minimap_icon = getUnicolorTexture(m_color);
    }

    // Only load the model if the .kart file has the appropriate version,
    // otherwise warnings are printed.
    if (m_version >= 1)
    {
        const bool success = m_kart_model->loadModels(*this);
        if (!success)
        {
            delete m_kart_model;
            file_manager->popTextureSearchPath();
            file_manager->popModelSearchPath();
            throw std::runtime_error("Cannot load kart models");
        }
    }

    if(m_gravity_center_shift.getX()==UNDEFINED)
    {
        m_gravity_center_shift.setX(0);
        // Default: center at the very bottom of the kart.
        m_gravity_center_shift.setY(m_kart_model->getHeight()*0.5f);
        m_gravity_center_shift.setZ(0);
    }

    // In older STK versions the physical wheels where moved 'wheel_radius'
    // into the physical body (i.e. 'hypothetical' wheel shape would not
    // poke out of the physical shape). In order to make the karts a bit more
    // stable, the physical wheel position (i.e. location of raycast) were
    // moved to be on the corner of the shape. In order to retain the same
    // steering behaviour, the wheel base (which in turn determines the
    // turn angle at certain speeds) is shortened by 2*wheel_radius
    m_wheel_base = fabsf(m_kart_model->getLength() - 2*m_wheel_radius);

    // Now convert the turn radius into turn angle:
    for(unsigned int i=0; i<m_turn_angle_at_speed.size(); i++)
    {
        m_turn_angle_at_speed.setY( i,
                            sin(m_wheel_base/m_turn_angle_at_speed.getY(i)) );
    }

    m_shadow_texture = irr_driver->getTexture(m_shadow_file);

    irr_driver->unsetTextureErrorMessage();
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath();

}   // load

//-----------------------------------------------------------------------------
/** Actually reads in the data from the xml file.
 *  \param root Root of the xml tree.
 */
void KartProperties::getAllData(const XMLNode * root)
{
    root->get("version",           &m_version);

    root->get("name",              &m_name             );

    root->get("icon-file",         &m_icon_file        );

    root->get("minimap-icon-file", &m_minimap_icon_file);

    root->get("shadow-file",       &m_shadow_file      );
    Vec3 c;
    root->get("rgb",               &c                  );
    m_color.set(255, (int)(255*c.getX()), (int)(255*c.getY()), (int)(255*c.getZ()));

    root->get("groups",            &m_groups           );

    root->get("random-wheel-rot",  &m_has_rand_wheels  );

    root->get("shadow-scale",      &m_shadow_scale     );
    root->get("shadow-x-offset",   &m_shadow_x_offset  );
    root->get("shadow-z-offset",   &m_shadow_z_offset  );

    root->get("type",     &m_kart_type        );

    if(const XMLNode *dimensions_node = root->getNode("center"))
        dimensions_node->get("gravity-shift", &m_gravity_center_shift);

    if(const XMLNode *ai_node = root->getNode("ai"))
    {
        const XMLNode *easy = ai_node->getNode("easy");
        m_ai_properties[RaceManager::DIFFICULTY_EASY]->load(easy);
        const XMLNode *medium = ai_node->getNode("medium");
        m_ai_properties[RaceManager::DIFFICULTY_MEDIUM]->load(medium);
        const XMLNode *hard = ai_node->getNode("hard");
        m_ai_properties[RaceManager::DIFFICULTY_HARD]->load(hard);
        const XMLNode *best = ai_node->getNode("best");
        m_ai_properties[RaceManager::DIFFICULTY_BEST]->load(best);
    }

    if(const XMLNode *suspension_node = root->getNode("suspension"))
    {
        suspension_node->get("stiffness",            &m_suspension_stiffness);
        suspension_node->get("rest",                 &m_suspension_rest     );
        suspension_node->get("travel-cm",            &m_suspension_travel_cm);
        suspension_node->get("exp-spring-response",  &m_exp_spring_response );
        suspension_node->get("max-force",            &m_max_suspension_force);
    }

    if(const XMLNode *wheels_node = root->getNode("wheels"))
    {
        wheels_node->get("damping-relaxation",  &m_wheel_damping_relaxation );
        wheels_node->get("damping-compression", &m_wheel_damping_compression);
        wheels_node->get("radius",              &m_wheel_radius             );
    }

    if(const XMLNode *speed_weighted_objects_node = root->getNode("speed-weighted-objects"))
    {
        m_speed_weighted_object_properties.loadFromXMLNode(speed_weighted_objects_node);
    }

    if(const XMLNode *friction_node = root->getNode("friction"))
        friction_node->get("slip", &m_friction_slip);

    if(const XMLNode *stability_node = root->getNode("stability"))
    {
        stability_node->get("roll-influence",
                                                   &m_roll_influence         );
        stability_node->get("chassis-linear-damping",
                                                   &m_chassis_linear_damping );
        stability_node->get("chassis-angular-damping",
                                                   &m_chassis_angular_damping);
        stability_node->get("downward-impulse-factor",
                                                   &m_downward_impulse_factor);
        stability_node->get("track-connection-accel",
                                                   &m_track_connection_accel );
        stability_node->get("smooth-flying-impulse", &m_smooth_flying_impulse);
    }

    if(const XMLNode *collision_node = root->getNode("collision"))
    {
        collision_node->get("impulse",         &m_collision_impulse        );
        collision_node->get("impulse-time",    &m_collision_impulse_time   );
        collision_node->get("terrain-impulse", &m_collision_terrain_impulse);
        collision_node->get("restitution",     &m_restitution              );
        collision_node->get("bevel-factor",    &m_bevel_factor             );
        collision_node->get("physical-wheel-position",&m_physical_wheel_position);
        std::string s;
        collision_node->get("impulse-type",    &s                          );
        s = StringUtils::toLowerCase(s);
        if(s=="none")
            m_terrain_impulse_type = IMPULSE_NONE;
        else if(s=="normal")
            m_terrain_impulse_type = IMPULSE_NORMAL;
        else if(s=="driveline")
            m_terrain_impulse_type = IMPULSE_TO_DRIVELINE;
        else
        {
            Log::fatal("[KartProperties]",
                       "Missing or incorrect value for impulse-type: '%s'.",
                       s.c_str());
        }
    }

    //TODO: wheel front right and wheel front left is not loaded, yet is
    //TODO: listed as an attribute in the xml file after wheel-radius
    //TODO: same goes for their rear equivalents


    if(const XMLNode *jump_node= root->getNode("jump"))
    {
        jump_node->get("animation-time", &m_jump_animation_time);
    }

    if(const XMLNode *camera_node= root->getNode("camera"))
    {
        camera_node->get("distance", &m_camera_distance);
        camera_node->get("forward-up-angle", &m_camera_forward_up_angle);
        m_camera_forward_up_angle *= DEGREE_TO_RAD;
        camera_node->get("backward-up-angle", &m_camera_backward_up_angle);
        m_camera_backward_up_angle *= DEGREE_TO_RAD;
    }

    if(const XMLNode *sounds_node= root->getNode("sounds"))
    {
        std::string s;
        sounds_node->get("engine", &s);
        if      (s == "large") m_engine_sfx_type = "engine_large";
        else if (s == "small") m_engine_sfx_type = "engine_small";
        else
        {
            if (SFXManager::get()->soundExist(s))
            {
                m_engine_sfx_type = s;
            }
            else
            {
                Log::error("[KartProperties]",
                           "Kart '%s' has an invalid engine '%s'.",
                           m_name.c_str(), s.c_str());
                m_engine_sfx_type = "engine_small";
            }
        }

#ifdef WILL_BE_ENABLED_ONCE_DONE_PROPERLY
        // Load custom kart SFX files (TODO: enable back when it's implemented properly)
        for (int i = 0; i < SFXManager::NUM_CUSTOMS; i++)
        {
            std::string tempFile;
            // Get filename associated with each custom sfx tag in sfx config
            if (sounds_node->get(SFXManager::get()->getCustomTagName(i), tempFile))
            {
                // determine absolute filename
                // FIXME: will not work with add-on packs (is data dir the same)?
                tempFile = file_manager->getKartFile(tempFile, getIdent());

                // Create sfx in sfx manager and store id
                m_custom_sfx_id[i] = SFXManager::get()->addSingleSfx(tempFile, 1, 0.2f,1.0f);
            }
            else
            {
                // if there is no filename associated with a given tag
                m_custom_sfx_id[i] = -1;
            }   // if custom sound
        }   // for i<SFXManager::NUM_CUSTOMS
#endif
    }   // if sounds-node exist

    if(const XMLNode *nitro_node = root->getNode("nitro"))
    {
        nitro_node->get("consumption",          &m_nitro_consumption       );
        nitro_node->get("small-container",      &m_nitro_small_container   );
        nitro_node->get("big-container",        &m_nitro_big_container     );
        nitro_node->get("max-speed-increase",   &m_nitro_max_speed_increase);
        nitro_node->get("engine-force",         &m_nitro_engine_force      );
        nitro_node->get("duration",             &m_nitro_duration          );
        nitro_node->get("fade-out-time",        &m_nitro_fade_out_time     );
        nitro_node->get("max",                  &m_nitro_max               );
        nitro_node->get("min-consumption-time", &m_nitro_min_consumption   );
    }

    if(const XMLNode *bubble_node = root->getNode("bubblegum"))
    {
        bubble_node->get("time",           &m_bubblegum_time          );
        bubble_node->get("speed-fraction", &m_bubblegum_speed_fraction);
        bubble_node->get("fade-in-time",   &m_bubblegum_fade_in_time  );
        bubble_node->get("torque",         &m_bubblegum_torque        );
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
        explosion_node->get("invulnerability-time",
                        &m_explosion_invulnerability_time);
    }

    if(const XMLNode *skid_node = root->getNode("skid"))
    {
        m_skidding_properties->load(skid_node);
    }


    if(const XMLNode *slipstream_node = root->getNode("slipstream"))
    {
        slipstream_node->get("length",       &m_slipstream_length            );
        slipstream_node->get("width",        &m_slipstream_width             );
        slipstream_node->get("collect-time", &m_slipstream_collect_time      );
        slipstream_node->get("use-time",     &m_slipstream_use_time          );
        slipstream_node->get("add-power",    &m_slipstream_add_power         );
        slipstream_node->get("min-speed",    &m_slipstream_min_speed         );
        slipstream_node->get("max-speed-increase",
                                             &m_slipstream_max_speed_increase);
        slipstream_node->get("duration",     &m_slipstream_duration          );
        slipstream_node->get("fade-out-time",&m_slipstream_fade_out_time     );
    }

    if(const XMLNode *turn_node = root->getNode("turn"))
    {
        turn_node->get("time-full-steer",      &m_time_full_steer     );
        turn_node->get("time-reset-steer",     &m_time_reset_steer    );
        turn_node->get("turn-radius",          &m_turn_angle_at_speed );
        // For now store the turn radius in turn angle, the correct
        // value can only be determined later in ::load
    }

    if(const XMLNode *engine_node = root->getNode("engine"))
    {
        engine_node->get("brake-factor",            &m_brake_factor);
        engine_node->get("brake-time-increase",     &m_brake_time_increase);
        engine_node->get("max-speed-reverse-ratio", &m_max_speed_reverse_ratio);
        engine_node->get("power", &m_engine_power);
        if(m_engine_power.size()!=RaceManager::DIFFICULTY_COUNT)
        {
            Log::fatal("[KartProperties]",
                       "Incorrect engine-power specifications for kart '%s'",
                       getIdent().c_str());
        }
        engine_node->get("max-speed", &m_max_speed);
        if(m_max_speed.size()!=RaceManager::DIFFICULTY_COUNT)
        {
            Log::fatal("[KartProperties]",
                       "Incorrect max-speed specifications for kart '%s'",
                       getIdent().c_str());
        }
    }   // if getNode("engine")

    if(const XMLNode *gear_node = root->getNode("gear"))
    {
        gear_node->get("switch-ratio",   &m_gear_switch_ratio  );
        gear_node->get("power-increase", &m_gear_power_increase);
    }

    if(const XMLNode *mass_node = root->getNode("mass"))
        mass_node->get("value", &m_mass);

    if(const XMLNode *plunger_node= root->getNode("plunger"))
    {
        plunger_node->get("band-max-length",    &m_rubber_band_max_length    );
        plunger_node->get("band-force",         &m_rubber_band_force         );
        plunger_node->get("band-duration",      &m_rubber_band_duration      );
        plunger_node->get("band-speed-increase",&m_rubber_band_speed_increase);
        plunger_node->get("band-fade-out-time", &m_rubber_band_fade_out_time );
        plunger_node->get("in-face-time", &m_plunger_in_face_duration);
        if(m_plunger_in_face_duration.size()!=RaceManager::DIFFICULTY_COUNT)
        {
            Log::fatal("KartProperties",
                       "Invalid plunger in-face-time specification.");
        }
    }

    if(const XMLNode *zipper_node= root->getNode("zipper"))
    {
        zipper_node->get("time",               &m_zipper_time              );
        zipper_node->get("fade-out-time",      &m_zipper_fade_out_time     );
        zipper_node->get("force",              &m_zipper_force             );
        zipper_node->get("speed-gain",         &m_zipper_speed_gain        );
        zipper_node->get("max-speed-increase", &m_zipper_max_speed_increase);
    }

    if(const XMLNode *swatter_node= root->getNode("swatter"))
    {
        swatter_node->get("duration",        &m_swatter_duration      );
        swatter_node->get("squash-duration", &m_squash_duration       );
        swatter_node->get("squash-slowdown", &m_squash_slowdown       );
        if(swatter_node->get("distance",     &m_swatter_distance2) )
        {
            // Avoid squaring if distance is not defined, so that
            // distance2 remains UNDEFINED (which is a negative value)
            m_swatter_distance2 *= m_swatter_distance2;
        }
    }

    if(const XMLNode *lean_node= root->getNode("lean"))
    {
        lean_node->get("max",   &m_max_lean  );
        lean_node->get("speed", &m_lean_speed);
        m_max_lean   *= DEGREE_TO_RAD;
        m_lean_speed *= DEGREE_TO_RAD;
    }

    if(const XMLNode *startup_node= root->getNode("startup"))
    {
        startup_node->get("time", &m_startup_times);
        startup_node->get("boost", &m_startup_boost);
    }

    if(const XMLNode *graphics_node = root->getNode("graphics"))
    {
        graphics_node->get("y-offset", &m_graphical_y_offset);
    }

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
        Log::fatal("[KartProperties]",
                   "Missing default value for 'gear-switch-ratio' in '%s'.",
                   filename.c_str());
    }
    if(m_gear_power_increase.size()==0)
    {
        Log::fatal("[KartProperties]",
                   "Missing default value for 'gear-power-increase' in '%s'.",
                filename.c_str());
    }
    if(m_gear_switch_ratio.size()!=m_gear_power_increase.size())    {
        Log::error("KartProperties",
                   "Number of entries for 'gear-switch-ratio' and "
                   "'gear-power-increase");
        Log::fatal("KartProperties", "in '%s' must be equal.",
                    filename.c_str());
    }
    if(m_startup_boost.size()!=m_startup_times.size())
    {
        Log::error("[KartProperties]",
                 "Number of entried for 'startup times' and 'startup-boost");
        Log::fatal("KartProperties", "must be identical.");
    }
#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                      \
        Log::fatal("[KartProperties]",                                \
                    "Missing default value for '%s' in '%s'.",    \
                    strA,filename.c_str());                \
    }

    CHECK_NEG(m_mass,                       "mass"                          );
    CHECK_NEG(m_time_reset_steer,           "turn time-reset-steer"         );
    CHECK_NEG(m_wheel_damping_relaxation,   "wheels damping-relaxation"     );
    CHECK_NEG(m_wheel_damping_compression,  "wheels damping-compression"    );
    CHECK_NEG(m_wheel_radius,               "wheels radius"                 );
    CHECK_NEG(m_friction_slip,              "friction slip"                 );
    CHECK_NEG(m_roll_influence,             "stability roll-influence"      );
    CHECK_NEG(m_chassis_linear_damping,  "stability chassis-linear-damping" );
    CHECK_NEG(m_chassis_angular_damping, "stability chassis-angular-damping");
    CHECK_NEG(m_downward_impulse_factor, "stability downward-impulse-factor");
    CHECK_NEG(m_track_connection_accel,  "stability track-connection-accel" );
    CHECK_NEG(m_smooth_flying_impulse,      "smooth-flying-impulse"         );
    CHECK_NEG(m_max_speed_reverse_ratio,    "engine max-speed-reverse-ratio");
    CHECK_NEG(m_brake_factor,               "engine brake-factor"           );
    CHECK_NEG(m_brake_time_increase,        "engine brake-time-increase"    );
    CHECK_NEG(m_suspension_stiffness,       "suspension stiffness"          );
    CHECK_NEG(m_suspension_rest,            "suspension rest"               );
    CHECK_NEG(m_suspension_travel_cm,       "suspension travel-cm"          );
    CHECK_NEG(m_max_suspension_force,       "suspension max-force"          );
    CHECK_NEG(m_collision_impulse,          "collision impulse"             );
    CHECK_NEG(m_collision_impulse_time,     "collision impulse-time"        );
    CHECK_NEG(m_restitution,                "collision restitution"         );
    CHECK_NEG(m_collision_terrain_impulse,  "collision terrain-impulse"     );
    CHECK_NEG(m_bevel_factor.getX(),        "collision bevel-factor"        );
    CHECK_NEG(m_bevel_factor.getY(),        "collision bevel-factor"        );
    CHECK_NEG(m_bevel_factor.getZ(),        "collision bevel-factor"        );
    CHECK_NEG(m_physical_wheel_position,    "collision physical-wheel-position");
    CHECK_NEG(m_rubber_band_max_length,     "plunger band-max-length"       );
    CHECK_NEG(m_rubber_band_force,          "plunger band-force"            );
    CHECK_NEG(m_rubber_band_duration,       "plunger band-duration"         );
    CHECK_NEG(m_rubber_band_speed_increase, "plunger band-speed-increase"   );
    CHECK_NEG(m_rubber_band_fade_out_time,  "plunger band-fade-out-time"    );
    CHECK_NEG(m_zipper_time,                "zipper-time"                   );
    CHECK_NEG(m_zipper_fade_out_time,       "zipper-fade-out-time"          );
    CHECK_NEG(m_zipper_force,               "zipper-force"                  );
    CHECK_NEG(m_zipper_speed_gain,          "zipper-speed-gain"             );
    CHECK_NEG(m_zipper_max_speed_increase,  "zipper-max-speed-increase"     );
    CHECK_NEG(m_slipstream_length,          "slipstream length"             );
    CHECK_NEG(m_slipstream_width,           "slipstream width"              );
    CHECK_NEG(m_slipstream_collect_time,    "slipstream collect-time"       );
    CHECK_NEG(m_slipstream_use_time,        "slipstream use-time"           );
    CHECK_NEG(m_slipstream_add_power,       "slipstream add-power"          );
    CHECK_NEG(m_slipstream_min_speed,       "slipstream min-speed"          );
    CHECK_NEG(m_slipstream_max_speed_increase,
                                            "slipstream max-speed-increase" );
    CHECK_NEG(m_slipstream_duration,        "slipstream duration"           );
    CHECK_NEG(m_slipstream_fade_out_time,   "slipstream fade-out-time"      );
    CHECK_NEG(m_camera_distance,            "camera distance"               );
    CHECK_NEG(m_camera_forward_up_angle,    "camera forward-up-angle"       );
    CHECK_NEG(m_camera_backward_up_angle,   "camera forward-up-angle"       );
    CHECK_NEG(m_nitro_consumption,          "nitro consumption"             );
    CHECK_NEG(m_nitro_big_container,        "nitro big-container"           );
    CHECK_NEG(m_nitro_small_container,      "nitro small-container"         );
    CHECK_NEG(m_nitro_max_speed_increase,   "nitro max-speed-increase"      );
    CHECK_NEG(m_nitro_engine_force,         "nitro engine-force"            );
    CHECK_NEG(m_nitro_duration,             "nitro duration"                );
    CHECK_NEG(m_nitro_fade_out_time,        "nitro fade-out-time"           );
    CHECK_NEG(m_nitro_max,                  "nitro max"                     );
    CHECK_NEG(m_bubblegum_time,             "bubblegum time"                );
    CHECK_NEG(m_bubblegum_speed_fraction,   "bubblegum speed-fraction"      );
    CHECK_NEG(m_bubblegum_fade_in_time  ,   "bubblegum fade-in-time"        );
    CHECK_NEG(m_bubblegum_torque,           "bubblegum  torque"             );

    CHECK_NEG(m_swatter_distance2,          "swatter distance"              );
    CHECK_NEG(m_swatter_duration,           "swatter duration"              );
    CHECK_NEG(m_squash_duration,            "swatter squash-duration"       );
    CHECK_NEG(m_squash_slowdown,            "swatter squash-slowdown"       );
    CHECK_NEG(m_max_lean,                   "lean max"                      );
    CHECK_NEG(m_lean_speed,                 "lean speed"                    );

    CHECK_NEG(m_rescue_height,              "rescue height"                 );
    CHECK_NEG(m_rescue_time,                "rescue time"                   );
    CHECK_NEG(m_rescue_vert_offset,         "rescue vert-offset"            );
    CHECK_NEG(m_explosion_time,             "explosion time"                );
    CHECK_NEG(m_explosion_invulnerability_time,
                                            "explosion invulnerability-time");
    CHECK_NEG(m_explosion_radius,           "explosion radius"              );
    CHECK_NEG(m_graphical_y_offset,         "graphics y-offset"             );
    for(unsigned int i=RaceManager::DIFFICULTY_FIRST;
        i<=RaceManager::DIFFICULTY_LAST; i++)
    {
        CHECK_NEG(m_max_speed[i], "engine maximum-speed[0]");
        CHECK_NEG(m_engine_power[i], "engine power" );
        CHECK_NEG(m_plunger_in_face_duration[i],"plunger in-face-time");
    }

    m_speed_weighted_object_properties.checkAllSet();

    m_skidding_properties->checkAllSet(filename);
    for(unsigned int i=0; i<RaceManager::DIFFICULTY_COUNT; i++)
        m_ai_properties[i]->checkAllSet(filename);
}   // checkAllSet

// ----------------------------------------------------------------------------
bool KartProperties::operator<(const KartProperties &other) const
{
    PlayerProfile *p = PlayerManager::getCurrentPlayer();
    bool this_is_locked = p->isLocked(getIdent());
    bool other_is_locked = p->isLocked(other.getIdent());
    if (this_is_locked == other_is_locked)
    {
        return getName() < other.getName();
    }
    else
        return other_is_locked;

    return true;
}  // operator<

// ----------------------------------------------------------------------------
bool KartProperties::isInGroup(const std::string &group) const
{
    return std::find(m_groups.begin(), m_groups.end(), group) != m_groups.end();
}   // isInGroups


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

// ----------------------------------------------------------------------------
const float KartProperties::getAvgPower() const
{
    float sum = 0.0;
    for (unsigned int i = 0; i < m_gear_power_increase.size(); ++i)
    {
        sum += m_gear_power_increase[i]*m_max_speed[0];
    }
    return sum/m_gear_power_increase.size();
}   // getAvgPower

/* EOF */
