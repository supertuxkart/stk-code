//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "config/stk_config.hpp"

#include <stdexcept>
#include <stdio.h>
#include <sstream>

#include "audio/music_information.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "karts/kart_properties.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

STKConfig* stk_config=0;
float STKConfig::UNDEFINED = -99.9f;

//-----------------------------------------------------------------------------
/** Constructor, which only initialises the object. The actual work is done
 *  by calling load().
 */
STKConfig::STKConfig()
{
    m_has_been_loaded         = false;
    m_title_music             = NULL;
    m_default_music           = NULL;
    m_race_win_music          = NULL;
    m_race_neutral_music      = NULL;
    m_race_lose_music         = NULL;
    m_gp_win_music            = NULL;
    m_gp_lose_music           = NULL;
    m_unlock_music            = NULL;
    m_default_kart_properties = new KartProperties();
}   // STKConfig
//-----------------------------------------------------------------------------
STKConfig::~STKConfig()
{
    if (m_title_music)
        delete m_title_music;

    if (m_default_music)
        delete m_default_music;

    if (m_race_win_music)
        delete m_race_win_music;

    if (m_race_neutral_music)
        delete m_race_neutral_music;

    if (m_race_lose_music)
        delete m_race_lose_music;

    if (m_gp_win_music)
        delete m_gp_win_music;

    if (m_gp_lose_music)
        delete m_gp_lose_music;

    if (m_unlock_music)
        delete m_unlock_music;

    if (m_default_kart_properties)
        delete m_default_kart_properties;

    for(std::map<std::string, KartProperties*>::iterator it = m_kart_properties.begin();
            it != m_kart_properties.end(); ++it)
    {
        if (it->second)
            delete it->second;
    }
}   // ~STKConfig

//-----------------------------------------------------------------------------
/** Loads the stk configuration file. After loading it checks if all necessary
 *  values are actually defined, otherwise an error message is printed and STK
 *  is aborted.
 *  /param filename Name of the configuration file to load.
 */
void STKConfig::load(const std::string &filename)
{
    // Avoid loading the default config file if a user-specific
    // config file has already been loaded.
    if(m_has_been_loaded) return;
    m_has_been_loaded = true;

    init_defaults();

    XMLNode *root = 0;
    try
    {
        root = new XMLNode(filename);
        if(!root || root->getName()!="config")
        {
            if(root) delete root;
            std::ostringstream msg;
            msg << "Couldn't load config '" << filename << "': no config node.";
            throw std::runtime_error(msg.str());
        }
        getAllData(root);
    }

    catch(std::exception& err)
    {
        Log::error("StkConfig", "FATAL ERROR while reading '%s':", filename.c_str());
        Log::fatal("StkConfig", "    %s", err.what());
    }
    delete root;

    // Check that all necessary values are indeed set
    // -----------------------------------------------

#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                   \
        Log::fatal("StkConfig", "Missing default value for '%s' in '%s'.",    \
                   strA,filename.c_str());              \
    }

    if(m_score_increase.size()==0)
    {
        Log::fatal("StkConfig", "Not or not enough scores defined in stk_config");
    }
    if(m_leader_intervals.size()==0)
    {
        Log::fatal("StkConfig", "No follow leader interval(s) defined in stk_config");
    }

    if(m_switch_items.size()!=Item::ITEM_LAST-Item::ITEM_FIRST+1)
    {
        Log::fatal("StkConfig", "Wrong number of item switches defined in stk_config");
    }

    if (m_client_port == 0 || m_server_port == 0 || m_server_discovery_port == 0 ||
        m_client_port == m_server_port || m_client_port == m_server_discovery_port ||
        m_server_port == m_server_discovery_port)
    {
        Log::fatal("StkConfig", "Invalid default port values.");
    }
    CHECK_NEG(m_max_karts,                 "<karts max=..."             );
    CHECK_NEG(m_item_switch_ticks,         "item switch-time"           );
    CHECK_NEG(m_bubblegum_counter,         "bubblegum disappear counter");
    CHECK_NEG(m_explosion_impulse_objects, "explosion-impulse-objects"  );
    CHECK_NEG(m_max_skidmarks,             "max-skidmarks"              );
    CHECK_NEG(m_min_kart_version,          "<kart-version min...>"      );
    CHECK_NEG(m_max_kart_version,          "<kart-version max=...>"     );
    CHECK_NEG(m_min_track_version,         "min-track-version"          );
    CHECK_NEG(m_max_track_version,         "max-track-version"          );
    CHECK_NEG(m_min_server_version,        "min-server-version"         );
    CHECK_NEG(m_max_server_version,        "max-server-version"         );
    CHECK_NEG(m_skid_fadeout_time,         "skid-fadeout-time"          );
    CHECK_NEG(m_near_ground,               "near-ground"                );
    CHECK_NEG(m_delay_finish_time,         "delay-finish-time"          );
    CHECK_NEG(m_music_credit_time,         "music-credit-time"          );
    CHECK_NEG(m_leader_time_per_kart,      "leader time-per-kart"       );
    CHECK_NEG(m_penalty_ticks,             "penalty-time"               );
    CHECK_NEG(m_max_display_news,          "max-display-news"           );
    CHECK_NEG(m_replay_max_frames,         "replay max-frames"          );
    CHECK_NEG(m_replay_delta_steering,     "replay delta-steering"      );
    CHECK_NEG(m_replay_delta_speed,        "replay delta-speed     "    );
    CHECK_NEG(m_replay_dt,                 "replay delta-t"             );
    CHECK_NEG(m_smooth_angle_limit,        "physics smooth-angle-limit" );
    CHECK_NEG(m_default_track_friction,    "physics default-track-friction");
    CHECK_NEG(m_physics_fps,               "physics fps"                );
    CHECK_NEG(m_no_explosive_items_timeout,"powerup no-explosive-items-timeout"    );
    CHECK_NEG(m_max_moveable_objects,      "network max-moveable-objects");
    CHECK_NEG(m_network_steering_reduction,"network steering-reduction" );
    CHECK_NEG(m_default_moveable_friction, "physics default-moveable-friction");
    CHECK_NEG(m_solver_iterations,         "physics: solver-iterations"       );
    CHECK_NEG(m_solver_split_impulse_thresh,"physics: solver-split-impulse-threshold");
    CHECK_NEG(m_snb_min_adjust_length, "network smoothing: min-adjust-length");
    CHECK_NEG(m_snb_max_adjust_length, "network smoothing: max-adjust-length");
    CHECK_NEG(m_snb_min_adjust_speed, "network smoothing: min-adjust-speed");
    CHECK_NEG(m_snb_max_adjust_time, "network smoothing: max-adjust-time");
    CHECK_NEG(m_snb_adjust_length_threshold, "network smoothing: adjust-length-threshold");
    CHECK_NEG(m_bonusbox_item_return_ticks, "bonus box return time");
    CHECK_NEG(m_nitro_item_return_ticks, "nitro return time");
    CHECK_NEG(m_banana_item_return_ticks, "banana return time");
    CHECK_NEG(m_bubblegum_item_return_ticks, "bubble gum return time");

    // Square distance to make distance checks cheaper (no sqrt)
    m_default_kart_properties->checkAllSet(filename);
}   // load

// -----------------------------------------------------------------------------
/** Init all values with invalid defaults, which are tested later. This
 * guarantees that all parameters will indeed be initialised, and helps
 * finding typos.
 */
void STKConfig::init_defaults()
{
    m_bomb_time                  = m_bomb_time_increase          =
        m_explosion_impulse_objects = m_music_credit_time        =
        m_delay_finish_time      = m_skid_fadeout_time           =
        m_near_ground            = m_solver_split_impulse_thresh =
        m_smooth_angle_limit     = m_default_track_friction      =
        m_default_moveable_friction =    UNDEFINED;
    m_item_switch_ticks          = -100;
    m_penalty_ticks              = -100;
    m_physics_fps                = -100;
    m_bubblegum_counter          = -100;
    m_shield_restrict_weapons    = false;
    m_max_karts                  = -100;
    m_max_skidmarks              = -100;
    m_min_kart_version           = -100;
    m_max_kart_version           = -100;
    m_min_track_version          = -100;
    m_max_track_version          = -100;
    m_min_server_version         = -100;
    m_max_server_version         = -100;
    m_max_display_news           = -100;
    m_replay_max_frames          = -100;
    m_replay_delta_steering      = -100;
    m_replay_delta_speed         = -100;
    m_replay_dt                  = -100;
    m_donate_url                 = "";
    m_password_reset_url         = "";
    m_no_explosive_items_timeout = -100.0f;
    m_max_moveable_objects       = -100;
    m_solver_iterations          = -100;
    m_solver_set_flags           = 0;
    m_solver_reset_flags         = 0;
    m_network_steering_reduction = -100;
    m_title_music                = NULL;
    m_default_music              = NULL;
    m_race_win_music             = NULL;
    m_race_neutral_music         = NULL;
    m_race_lose_music            = NULL;
    m_gp_win_music               = NULL;
    m_gp_lose_music              = NULL;
    m_unlock_music               = NULL;
    m_solver_split_impulse       = false;
    m_smooth_normals             = false;
    m_same_powerup_mode          = POWERUP_MODE_ONLY_IF_SAME;
    m_ai_acceleration            = 1.0f;
    m_disable_steer_while_unskid = false;
    m_camera_follow_skid         = false;
    m_cutscene_fov               = 0.61f;
    m_max_skinning_bones         = 1024;
    m_tc_quality                 = 16;
    m_server_discovery_port      = 2757;
    m_client_port                = 2758;
    m_server_port                = 2759;
    m_snb_min_adjust_length = m_snb_max_adjust_length =
        m_snb_min_adjust_speed = m_snb_max_adjust_time =
        m_snb_adjust_length_threshold = UNDEFINED;

    m_bonusbox_item_return_ticks  = -100;
    m_nitro_item_return_ticks     = -100;
    m_banana_item_return_ticks    = -100;
    m_bubblegum_item_return_ticks = -100;

    m_score_increase.clear();
    m_leader_intervals.clear();
    m_switch_items.clear();
    m_normal_ttf.clear();
    m_digit_ttf.clear();
}   // init_defaults

//-----------------------------------------------------------------------------
/** Extracts the actual information from a xml file.
 *  \param xml Pointer to the xml data structure.
 */
void STKConfig::getAllData(const XMLNode * root)
{
    // Get the values which are not part of the default KartProperties
    // ---------------------------------------------------------------
    if(const XMLNode *kart_node = root->getNode("kart-version"))
    {
        kart_node->get("min", &m_min_kart_version);
        kart_node->get("max", &m_max_kart_version);
    }

    if(const XMLNode *node = root->getNode("track-version"))
    {
        node->get("min", &m_min_track_version);
        node->get("max", &m_max_track_version);
    }

    if(const XMLNode *node = root->getNode("server-version"))
    {
        node->get("min", &m_min_server_version);
        node->get("max", &m_max_server_version);
    }

    if(const XMLNode *kart_node = root->getNode("karts"))
        kart_node->get("max-number", &m_max_karts);

    if (const XMLNode *node = root->getNode("HardwareReportServer"))
    {
        node->get("url", &m_server_hardware_report);
    }

    if (const XMLNode *node = root->getNode("OnlineServer"))
    {
        node->get("url", &m_server_api);
        node->get("server-version", &m_server_api_version);
    }

    if (const XMLNode *node = root->getNode("AddonServer"))
    {
        node->get("url", &m_server_addons);
        node->get("allow-news-redirects", &m_allow_news_redirects);
    }

    if(const XMLNode *gp_node = root->getNode("grand-prix"))
    {
        for(unsigned int i=0; i<gp_node->getNumNodes(); i++)
        {
            const XMLNode *pn=gp_node->getNode(i);
            int points=-1;
            pn->get("points", &points);
            if(points<0)
            {
                Log::error("StkConfig", "Incorrect GP point specification:");
                Log::fatal("StkConfig", "points: %d",
                        points);
            }
            m_score_increase.push_back(points);
        }
        if (m_max_karts > int(gp_node->getNumNodes()))
        {
            Log::error("StkConfig", "Not enough grand-prix ranking nodes:");
            m_score_increase.resize(m_max_karts, 0);
        }
    }

    if(const XMLNode *leader_node= root->getNode("follow-the-leader"))
    {
        leader_node->get("intervals",     &m_leader_intervals    );
        leader_node->get("time-per-kart", &m_leader_time_per_kart);
    }

    if (const XMLNode *physics_node= root->getNode("physics"))
    {
        physics_node->get("smooth-normals",         &m_smooth_normals        );
        physics_node->get("smooth-angle-limit",     &m_smooth_angle_limit    );
        physics_node->get("default-track-friction", &m_default_track_friction);
        physics_node->get("default-moveable-friction",
                                                 &m_default_moveable_friction);
        physics_node->get("fps",                    &m_physics_fps           );
        physics_node->get("solver-iterations",      &m_solver_iterations     );
        physics_node->get("solver-split-impulse",   &m_solver_split_impulse  );
        physics_node->get("solver-split-impulse-threshold",
                                               &m_solver_split_impulse_thresh);
        std::vector<std::string> solver_modes;
        physics_node->get("solver-mode",            &solver_modes            );
        m_solver_set_flags=0, m_solver_reset_flags = 0;
        int *p;
        for (auto mode : solver_modes)
        {
            std::string s = mode;
            p = &m_solver_set_flags;
            if (s[0] == '-')
            {
                s.erase(s.begin());
                p = &m_solver_reset_flags;
            }
            s = StringUtils::toLowerCase(s);
            if      (s == "randmize_order"                         ) *p |=   1;
            else if (s == "friction_separate"                      ) *p |=   2;
            else if (s == "use_warmstarting"                       ) *p |=   4;
            else if (s == "use_friction_warmstarting"              ) *p |=   8;
            else if (s == "use_2_friction_directions"              ) *p |=  16;
            else if (s == "enable_friction_direction_caching"      ) *p |=  32;
            else if (s == "disable_velocity_dependent_friction_direction") *p |= 64;
            else if (s == "cache_friendly"                         ) *p |= 128;
            else if (s == "simd"                                   ) *p |= 256;
            else if (s == "cuda"                                   ) *p |= 512;
            else
            {
                Log::fatal("STK-Config",
                           "Unknown option '%s' for solver-mode - ignored.",
                           s.c_str());
            }
        }   // for mode in solver_modes

    }

    if (const XMLNode *startup_node= root->getNode("startup"))
    {
        float f;
        startup_node->get("penalty", &f);
        m_penalty_ticks = time2Ticks(f);
    }

    if (const XMLNode *news_node= root->getNode("news"))
    {
        news_node->get("max-display", &m_max_display_news);
    }

    if (const XMLNode *steer_node= root->getNode("steer"))
    {
        steer_node->get("disable-while-unskid", &m_disable_steer_while_unskid);
        steer_node->get("camera-follow-skid",   &m_camera_follow_skid        );
    }

    if (const XMLNode *camera = root->getNode("camera"))
    {
        camera->get("fov-1", &m_camera_fov[0]);
        camera->get("fov-2", &m_camera_fov[1]);
        camera->get("fov-3", &m_camera_fov[2]);
        camera->get("fov-4", &m_camera_fov[3]);

        for (unsigned int i = 4; i < MAX_PLAYER_COUNT; i++)
        {
            camera->get("fov-4", &m_camera_fov[i]);
        }
        camera->get("cutscene-fov", &m_cutscene_fov);
    }

    if (const XMLNode *music_node = root->getNode("music"))
    {
        music_node->get("title", &m_title_music_file);
        assert(m_title_music_file.size() > 0);
        m_title_music_file = file_manager->getAsset(FileManager::MUSIC, m_title_music_file);

        music_node->get("default", &m_default_music_file);
        assert(m_default_music_file.size() > 0);
        m_default_music_file = file_manager->getAsset(FileManager::MUSIC, m_default_music_file);

        music_node->get("race-win", &m_race_win_music_file);
        assert(m_race_win_music_file.size() > 0);
        m_race_win_music_file = file_manager->getAsset(FileManager::MUSIC, m_race_win_music_file);

        music_node->get("race-neutral", &m_race_neutral_music_file);
        assert(m_race_neutral_music_file.size() > 0);
        m_race_neutral_music_file = file_manager->getAsset(FileManager::MUSIC, m_race_neutral_music_file);

        music_node->get("race-lose", &m_race_lose_music_file);
        assert(m_race_lose_music_file.size() > 0);
        m_race_lose_music_file = file_manager->getAsset(FileManager::MUSIC, m_race_lose_music_file);

        music_node->get("gp-win", &m_gp_win_music_file);
        assert(m_gp_win_music_file.size() > 0);
        m_gp_win_music_file = file_manager->getAsset(FileManager::MUSIC, m_gp_win_music_file);

        music_node->get("gp-lose", &m_gp_lose_music_file);
        assert(m_gp_lose_music_file.size() > 0);
        m_gp_lose_music_file = file_manager->getAsset(FileManager::MUSIC, m_gp_lose_music_file);

        music_node->get("unlock", &m_unlock_music_file);
        assert(m_unlock_music_file.size() > 0);
        m_unlock_music_file = file_manager->getAsset(FileManager::MUSIC, m_unlock_music_file);
    }

    if(const XMLNode *skidmarks_node = root->getNode("skid-marks"))
    {
        skidmarks_node->get("max-number",   &m_max_skidmarks    );
        skidmarks_node->get("fadeout-time", &m_skid_fadeout_time);
    }

    if(const XMLNode *near_ground_node = root->getNode("near-ground"))
        near_ground_node->get("distance", &m_near_ground);

    if(const XMLNode *delay_finish_node= root->getNode("delay-finish"))
        delay_finish_node->get("time", &m_delay_finish_time);

    if(const XMLNode *credits_node= root->getNode("credits"))
        credits_node->get("music", &m_music_credit_time);


    if(const XMLNode *bomb_node= root->getNode("bomb"))
    {
        bomb_node->get("time", &m_bomb_time);
        bomb_node->get("time-increase", &m_bomb_time_increase);
    }

    if(const XMLNode *item_return_node= root->getNode("item-return-time"))
    {
        float f;
        if(item_return_node->get("bonusbox", &f))
            m_bonusbox_item_return_ticks = time2Ticks(f);
        if(item_return_node->get("nitro", &f))
            m_nitro_item_return_ticks = time2Ticks(f);
        if(item_return_node->get("banana", &f))
            m_banana_item_return_ticks = time2Ticks(f);
        if(item_return_node->get("bubblegum", &f))
            m_bubblegum_item_return_ticks = time2Ticks(f);
    }

    if(const XMLNode *powerup_node= root->getNode("powerup"))
    {
        std::string s;
        powerup_node->get("collect-mode", &s);
        if(s=="same")
            m_same_powerup_mode = POWERUP_MODE_SAME;
        else if(s=="new")
            m_same_powerup_mode = POWERUP_MODE_NEW;
        else if(s=="only-if-same")
            m_same_powerup_mode = POWERUP_MODE_ONLY_IF_SAME;
        else
        {
            Log::warn("StkConfig", "Invalid item mode '%s' - ignored.",
                    s.c_str());
        }
        powerup_node->get("no-explosive-items-timeout",
            &m_no_explosive_items_timeout);
    }

    if(const XMLNode *switch_node= root->getNode("switch"))
    {
        switch_node->get("items", &m_switch_items    );
        float f;
        if( switch_node->get("time",  &f) )
            m_item_switch_ticks = stk_config->time2Ticks(f);
    }

    if(const XMLNode *bubblegum_node= root->getNode("bubblegum"))
    {
        bubblegum_node->get("disappear-counter", &m_bubblegum_counter      );
        bubblegum_node->get("restrict-weapons",  &m_shield_restrict_weapons);
    }

    if(const XMLNode *explosion_node= root->getNode("explosion"))
    {
        explosion_node->get("impulse-objects", &m_explosion_impulse_objects);
    }

    if(const XMLNode *ai_node = root->getNode("ai"))
    {
        ai_node->get("acceleration", &m_ai_acceleration);
    }

    if (const XMLNode *networking_node = root->getNode("networking"))
    {
        networking_node->get("max-moveable-objects", &m_max_moveable_objects);
        networking_node->get("steering-reduction", &m_network_steering_reduction);
    }

    if(const XMLNode *replay_node = root->getNode("replay"))
    {
        replay_node->get("delta-steering", &m_replay_delta_steering);
        replay_node->get("delta-speed", &m_replay_delta_speed      );
        replay_node->get("delta-t",     &m_replay_dt               );
        replay_node->get("max-frames",    &m_replay_max_frames     );

    }

    if (const XMLNode *urls = root->getNode("urls"))
    {
        urls->get("donate", &m_donate_url);
        urls->get("password-reset", &m_password_reset_url);
        urls->get("assets-download", &m_assets_download_url);
    }

    if (const XMLNode *urls = root->getNode("stun"))
    {
        urls->get("ipv4", &m_stun_ipv4);
        urls->get("ipv6", &m_stun_ipv6);
    }

    if (const XMLNode *fonts_list = root->getNode("fonts-list"))
    {
        fonts_list->get("normal-ttf", &m_normal_ttf);
        fonts_list->get("digit-ttf",  &m_digit_ttf );
        fonts_list->get("color-emoji-ttf", &m_color_emoji_ttf);
    }

    if (const XMLNode *skinning = root->getNode("skinning"))
    {
        skinning->get("max-bones", &m_max_skinning_bones);
    }

    if (const XMLNode *tc = root->getNode("texture-compression"))
    {
        tc->get("quality", &m_tc_quality);
    }

    if (const XMLNode *np = root->getNode("network-ports"))
    {
        unsigned server_discovery_port = 0;
        unsigned client_port = 0;
        unsigned server_port = 0;
        np->get("server-discovery-port", &server_discovery_port);
        np->get("client-port", &client_port);
        np->get("server-port", &server_port);
        m_server_discovery_port = (uint16_t)server_discovery_port;
        m_client_port = (uint16_t)client_port;
        m_server_port = (uint16_t)server_port;
    }

    if (const XMLNode *ns = root->getNode("network-smoothing"))
    {
        ns->get("min-adjust-length", &m_snb_min_adjust_length);
        ns->get("max-adjust-length", &m_snb_max_adjust_length);
        ns->get("min-adjust-speed", &m_snb_min_adjust_speed);
        ns->get("max-adjust-time", &m_snb_max_adjust_time);
        ns->get("adjust-length-threshold", &m_snb_adjust_length_threshold);
    }

    if (const XMLNode* nc = root->getNode("network-capabilities"))
    {
        for (unsigned int i = 0; i < nc->getNumNodes(); i++)
        {
            const XMLNode* name = nc->getNode(i);
            std::string cap;
            name->get("name", &cap);
            if (!cap.empty())
                m_network_capabilities.insert(cap);
        }
    }

    // Get the default KartProperties
    // ------------------------------
    const XMLNode *node = root -> getNode("general-kart-defaults");
    if(!node)
    {
        std::ostringstream msg;
        msg << "Couldn't load general-kart-defaults: no node.";
        throw std::runtime_error(msg.str());
    }
    m_default_kart_properties->getAllData(node);
    const XMLNode *child_node = node->getNode("kart-type");

    for (unsigned int i = 0; i < child_node->getNumNodes(); ++i)
    {
        const XMLNode* type = child_node->getNode(i);
        m_kart_properties[type->getName()] = new KartProperties();
        m_kart_properties[type->getName()]->copyFrom(m_default_kart_properties);
        m_kart_properties[type->getName()]->getAllData(type);
    }
}   // getAllData

// ----------------------------------------------------------------------------
/** Init the music files after downloading assets
 */
void STKConfig::initMusicFiles()
{
    m_title_music = MusicInformation::create(m_title_music_file);
    if (!m_title_music)
    {
        Log::error("StkConfig", "Cannot load title music: %s.",
            m_title_music_file.c_str());
    }

    m_default_music = MusicInformation::create(m_default_music_file);
    if (!m_default_music)
    {
        Log::error("StkConfig", "Cannot load default music: %s.",
            m_default_music_file.c_str());
    }

    m_race_win_music = MusicInformation::create(m_race_win_music_file);
    if (!m_race_win_music)
    {
        Log::error("StkConfig", "Cannot load race win music: %s.",
            m_race_win_music_file.c_str());
    }

    m_race_neutral_music = MusicInformation::create(m_race_neutral_music_file);
    if (!m_race_neutral_music)
    {
        Log::error("StkConfig", "Cannot load race neutral music: %s.",
            m_race_neutral_music_file.c_str());
    }

    m_race_lose_music = MusicInformation::create(m_race_lose_music_file);
    if (!m_race_lose_music)
    {
        Log::error("StkConfig", "Cannot load race lose music: %s.",
            m_race_lose_music_file.c_str());
    }

    m_gp_win_music = MusicInformation::create(m_gp_win_music_file);
    if (!m_gp_win_music)
    {
        Log::error("StkConfig", "Cannot load grand prix win music: %s.",
            m_gp_win_music_file.c_str());
    }

    m_gp_lose_music = MusicInformation::create(m_gp_lose_music_file);
    if (!m_gp_lose_music)
    {
        Log::error("StkConfig", "Cannot load grand prix lose music: %s.",
            m_gp_lose_music_file.c_str());
    }

    m_unlock_music = MusicInformation::create(m_unlock_music_file);
    if (!m_unlock_music)
    {
        Log::error("StkConfig", "Cannot load unlock music: %s.",
            m_unlock_music_file.c_str());
    }
}   // initMusicFiles

// ----------------------------------------------------------------------------
/** Defines the points for each position for a race with a given number
 *  of karts.
 *  \param all_scores A vector which will be resized to take num_karts
 *         elements and which on return contains the number of points
 *         for each position.
 * \param num_karts Number of karts.
 */
void  STKConfig::getAllScores(std::vector<int> *all_scores, int num_karts)
{
    std::vector<int> sorted_score_increase;

    if (num_karts == 0) return;

    assert(num_karts <= m_max_karts);
    all_scores->resize(num_karts);
    sorted_score_increase.resize(num_karts+1); //sorting function is [begin, end[

    //get increase data into sorted_score_increase
    for(int i=0; i<num_karts; i++)
    {
        sorted_score_increase[i] = m_score_increase[i];
    }

    std::sort (sorted_score_increase.begin(), sorted_score_increase.end());

    (*all_scores)[num_karts-1] = sorted_score_increase[0];  // last position score

    // Must be signed, in case that num_karts==1
    for(int i=num_karts-2; i>=0; i--)
    {
        (*all_scores)[i] = (*all_scores)[i+1] + sorted_score_increase[num_karts-i];
    }
}   // getAllScores
