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

#include "karts/player_difficulty.hpp"

#include "config/stk_config.hpp"
#include "io/xml_node.hpp"
#include "karts/skidding_properties.hpp"
#include "race/race_manager.hpp"
#include "modes/world.hpp"
#include "utils/log.hpp"

/**
 * The constructor initialises all values with default values.
 */
PlayerDifficulty::PlayerDifficulty(const std::string &filename)
{
    // Set all other values to undefined, so that it can later be tested
    // if everything is defined properly.
    m_mass = m_brake_factor = m_brake_time_increase = m_rescue_time =
        m_explosion_time = m_explosion_invulnerability_time = m_zipper_time =
        m_zipper_fade_out_time = m_zipper_force = m_zipper_speed_gain =
        m_zipper_max_speed_increase = m_rubber_band_max_length =
        m_rubber_band_force = m_rubber_band_duration =
        m_rubber_band_speed_increase = m_rubber_band_fade_out_time =
        m_nitro_consumption = m_nitro_max_speed_increase =
        m_nitro_engine_force = m_nitro_duration = m_nitro_fade_out_time =
        m_bubblegum_time = m_bubblegum_torque = m_bubblegum_speed_fraction =
        m_bubblegum_fade_in_time = m_swatter_duration = m_squash_duration =
        m_squash_slowdown = m_max_speed_reverse_ratio = m_slipstream_length =
        m_slipstream_width = m_slipstream_collect_time =
        m_slipstream_use_time = m_slipstream_add_power =
        m_slipstream_min_speed = m_slipstream_max_speed_increase =
        m_slipstream_duration = m_slipstream_fade_out_time = 1;

    m_startup_times.resize(RaceManager::DIFFICULTY_COUNT, 1);
    m_startup_boost.resize(RaceManager::DIFFICULTY_COUNT, 1);

    // The default constructor for stk_config uses filename=""
    if (filename != "")
        load(filename, "normal");
} // PlayerDifficulty

//-----------------------------------------------------------------------------
/** Destructor, dereferences the kart model. */
PlayerDifficulty::~PlayerDifficulty()
{
} // ~PlayerDifficulty

//-----------------------------------------------------------------------------
/**  */
std::string PlayerDifficulty::getIdent() const
{
    switch(m_difficulty)
    {
    case PLAYER_DIFFICULTY_NORMAL:   return "normal";   break;
    case PLAYER_DIFFICULTY_HANDICAP: return "handicap"; break;
    default:  assert(false);
    }
    return "";
}

//-----------------------------------------------------------------------------
/** Loads the difficulty properties from a file.
 *  \param filename Filename to load.
 *  \param node Name of the xml node to load the data from
 */
void PlayerDifficulty::load(const std::string &filename, const std::string &node)
{
    const XMLNode* root = new XMLNode(filename);
    getAllData(root->getNode(node));
    if(root)
        delete root;
}   // load

//-----------------------------------------------------------------------------
/** Actually reads in the data from the xml file.
 *  \param root Root of the xml tree.
 */
void PlayerDifficulty::getAllData(const XMLNode * root)
{
    if(const XMLNode *mass_node = root->getNode("mass"))
        mass_node->get("value", &m_mass);

    if(const XMLNode *engine_node = root->getNode("engine"))
    {
        engine_node->get("brake-factor",            &m_brake_factor);
        engine_node->get("brake-time-increase",     &m_brake_time_increase);
        engine_node->get("max-speed-reverse-ratio", &m_max_speed_reverse_ratio);
        engine_node->get("power",                   &m_engine_power);
        engine_node->get("max-speed",               &m_max_speed);
    }

    if(const XMLNode *nitro_node = root->getNode("nitro"))
    {
        nitro_node->get("consumption",        &m_nitro_consumption       );
        nitro_node->get("max-speed-increase", &m_nitro_max_speed_increase);
        nitro_node->get("engine-force",       &m_nitro_engine_force      );
        nitro_node->get("duration",           &m_nitro_duration          );
        nitro_node->get("fade-out-time",      &m_nitro_fade_out_time     );
    }

    if(const XMLNode *bubble_node = root->getNode("bubblegum"))
    {
        bubble_node->get("time",           &m_bubblegum_time          );
        bubble_node->get("speed-fraction", &m_bubblegum_speed_fraction);
        bubble_node->get("torque",         &m_bubblegum_torque        );
        bubble_node->get("fade-in-time",   &m_bubblegum_fade_in_time  );
    }

    if(const XMLNode *rescue_node = root->getNode("rescue"))
        rescue_node->get("time", &m_rescue_time);

    if(const XMLNode *explosion_node = root->getNode("explosion"))
    {
        explosion_node->get("time", &m_explosion_time);
        explosion_node->get("invulnerability-time",
                                    &m_explosion_invulnerability_time);
    }

    if(const XMLNode *slipstream_node = root->getNode("slipstream"))
    {
        slipstream_node->get("length",        &m_slipstream_length            );
        slipstream_node->get("width",         &m_slipstream_width             );
        slipstream_node->get("collect-time",  &m_slipstream_collect_time      );
        slipstream_node->get("use-time",      &m_slipstream_use_time          );
        slipstream_node->get("add-power",     &m_slipstream_add_power         );
        slipstream_node->get("min-speed",     &m_slipstream_min_speed         );
        slipstream_node->get("max-speed-increase",
                                              &m_slipstream_max_speed_increase);
        slipstream_node->get("duration",      &m_slipstream_duration          );
        slipstream_node->get("fade-out-time", &m_slipstream_fade_out_time     );
    }

    if(const XMLNode *plunger_node= root->getNode("plunger"))
    {
        plunger_node->get("band-max-length",    &m_rubber_band_max_length    );
        plunger_node->get("band-force",         &m_rubber_band_force         );
        plunger_node->get("band-duration",      &m_rubber_band_duration      );
        plunger_node->get("band-speed-increase",&m_rubber_band_speed_increase);
        plunger_node->get("band-fade-out-time", &m_rubber_band_fade_out_time );
        plunger_node->get("in-face-time", &m_plunger_in_face_duration);
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
    }

    if(const XMLNode *startup_node= root->getNode("startup"))
    {
        startup_node->get("time", &m_startup_times);
        startup_node->get("boost", &m_startup_boost);
    }
}   // getAllData

// ----------------------------------------------------------------------------
/** Called the first time a kart accelerates after 'ready-set-go'. It searches
 *  through m_startup_times to find the appropriate slot, and returns the
 *  speed-boost from the corresponding entry in m_startup_boost.
 *  If the kart started too slow (i.e. slower than the longest time in
 *  m_startup_times, it returns 0.
 */
float PlayerDifficulty::getStartupBoost() const
{
    float t = World::getWorld()->getTime();
    for(unsigned int i=0; i<m_startup_times.size(); i++)
    {
        if(t<=m_startup_times[i]) return m_startup_boost[i];
    }
    return 0;
}   // getStartupBoost

