//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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
#include "karts/cached_characteristic.hpp"
#include "karts/combined_characteristic.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/skidding_properties.hpp"
#include "karts/xml_characteristic.hpp"
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

std::string KartProperties::getPerPlayerDifficultyAsString(PerPlayerDifficulty d)
{
    switch(d)
    {
    case PLAYER_DIFFICULTY_NORMAL:   return "normal";   break;
    case PLAYER_DIFFICULTY_HANDICAP: return "handicap"; break;
    default:  assert(false);
    }
    return "";
}

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
    m_wheel_base = m_friction_slip = m_collision_terrain_impulse =
        m_collision_impulse = m_restitution = m_collision_impulse_time =
        m_max_lean = m_lean_speed = m_physical_wheel_position = UNDEFINED;

    m_terrain_impulse_type       = IMPULSE_NONE;
    m_gravity_center_shift       = Vec3(UNDEFINED);
    m_bevel_factor               = Vec3(UNDEFINED);
    m_version                    = 0;
    m_color                      = video::SColor(255, 0, 0, 0);
    m_shape                      = 32;  // close enough to a circle.
    m_engine_sfx_type            = "engine_small";
    m_kart_model                 = NULL;
    m_nitro_min_consumption      = 0.53f;
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

    if (source->m_characteristic)
    {
        m_characteristic.reset(new XmlCharacteristic());
        *m_characteristic = *source->m_characteristic;
        // Combine the characteristics for this object. We can't copy it because
        // this object has other pointers (to m_characteristic).
        combineCharacteristics();
    }

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
    {
        // Handle the case that kart_type might be incorrect
        try
        {
            copyFrom(&stk_config->getKartProperties(kart_type));
        }
        catch (std::out_of_range)
        {
            copyFrom(&stk_config->getDefaultKartProperties());
        }   // try .. catch
    }
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
        m_characteristic.reset(new XmlCharacteristic(root));
        combineCharacteristics();
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
        // If the kart is 'too high', its height will be changed in
        // kart.cpp, the same adjustment needs to be made here.
        if (m_kart_model->getHeight() > m_kart_model->getLength()*0.6f)
            m_gravity_center_shift.setY(m_kart_model->getLength()*0.6f*0.5f);
        else
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
    // Wheel radius was always 0.25, and is now not used anymore, but in order
    // to keep existing steering behaviour, the same formula is still
    // used.
    m_wheel_base = fabsf(m_kart_model->getLength() - 2*0.25f);

    m_shadow_texture = irr_driver->getTexture(m_shadow_file);

    irr_driver->unsetTextureErrorMessage();
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath();

}   // load

//-----------------------------------------------------------------------------
void KartProperties::combineCharacteristics()
{
    m_combined_characteristic.reset(new CombinedCharacteristic());
    m_combined_characteristic->addCharacteristic(kart_properties_manager->
        getBaseCharacteristic());

    // Try to get the kart type
    const AbstractCharacteristic *characteristic = kart_properties_manager->
        getKartTypeCharacteristic(m_kart_type);
    if (!characteristic)
        Log::warn("KartProperties", "Can't find kart type '%s' for kart '%s'",
            m_kart_type.c_str(), m_name.c_str());
    else
        // Kart type found
        m_combined_characteristic->addCharacteristic(characteristic);

    m_combined_characteristic->addCharacteristic(m_characteristic.get());
}   // combineCharacteristics

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

    if(const XMLNode *speed_weighted_objects_node = root->getNode("speed-weighted-objects"))
    {
        m_speed_weighted_object_properties.loadFromXMLNode(speed_weighted_objects_node);
    }

    if(const XMLNode *friction_node = root->getNode("friction"))
        friction_node->get("slip", &m_friction_slip);

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

    if(const XMLNode *skid_node = root->getNode("skid"))
    {
        m_skidding_properties->load(skid_node);
    }


    if(const XMLNode *lean_node= root->getNode("lean"))
    {
        lean_node->get("max",   &m_max_lean  );
        lean_node->get("speed", &m_lean_speed);
        m_max_lean   *= DEGREE_TO_RAD;
        m_lean_speed *= DEGREE_TO_RAD;
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
#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                      \
        Log::fatal("[KartProperties]",                                \
                    "Missing default value for '%s' in '%s'.",    \
                    strA,filename.c_str());                \
    }

    CHECK_NEG(m_friction_slip,              "friction slip"                 );
    CHECK_NEG(m_collision_terrain_impulse,  "collision terrain-impulse"     );
    CHECK_NEG(m_collision_impulse,          "collision impulse"             );
    CHECK_NEG(m_collision_impulse_time,     "collision impulse-time"        );
    CHECK_NEG(m_restitution,                "collision restitution"         );
    CHECK_NEG(m_physical_wheel_position,    "collision physical-wheel-position");

    CHECK_NEG(m_max_lean,                   "lean max"                      );
    CHECK_NEG(m_lean_speed,                 "lean speed"                    );

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
const AbstractCharacteristic* KartProperties::getCharacteristic() const
{
    return m_characteristic.get();
}   // getCharacteristic

// ----------------------------------------------------------------------------
const AbstractCharacteristic* KartProperties::getCombinedCharacteristic() const
{
    return m_combined_characteristic.get();
}   // getCombinedCharacteristic

// ----------------------------------------------------------------------------
bool KartProperties::isInGroup(const std::string &group) const
{
    return std::find(m_groups.begin(), m_groups.end(), group) != m_groups.end();
}   // isInGroups

// ----------------------------------------------------------------------------
float KartProperties::getAvgPower() const
{
    float sum = 0;
    std::vector<float> gear_power_increase = m_combined_characteristic->getGearPowerIncrease();
    float power = m_combined_characteristic->getEnginePower();
    for (unsigned int i = 0; i < gear_power_increase.size(); ++i)
        sum += gear_power_increase[i] * power;
    return sum / gear_power_increase.size();
}   // getAvgPower
