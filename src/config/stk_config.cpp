//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "audio/music_information.hpp"

STKConfig* stk_config=0;
float STKConfig::UNDEFINED = -99.9f;

//-----------------------------------------------------------------------------
STKConfig::~STKConfig()
{
    if(m_title_music)
        delete m_title_music;
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
        fprintf(stderr, "FATAL ERROR while reading StkConfig '%s':\n", filename.c_str());
        fprintf(stderr, "    %s", err.what());
        fprintf(stderr, "\n");
        exit(1);
    }
    delete root;

    // Check that all necessary values are indeed set
    // -----------------------------------------------

#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                         \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n",    \
                strA,filename.c_str());exit(-1);                       \
    }

    if(m_score_increase.size()==0 || (int)m_score_increase.size()!=m_max_karts)
    {
        fprintf(stderr,"Not or not enough scores defined in stk_config");
        exit(-1);
    }
    if(m_leader_intervals.size()==0)
    {
        fprintf(stderr,"No follow leader interval(s) defined in stk_config");
        exit(-1);
    }
    
    if(m_switch_items.size()!=Item::ITEM_LAST-Item::ITEM_FIRST+1)
    {
        fprintf(stderr,"No item switches defined in stk_config");
        exit(-1);
    }

    CHECK_NEG(m_max_karts,                 "<karts max=..."             );
    CHECK_NEG(m_gp_order,                  "grand-prix order=..."       );
    CHECK_NEG(m_parachute_friction,        "parachute-friction"         );
    CHECK_NEG(m_parachute_done_fraction,   "parachute-done-fraction"    );
    CHECK_NEG(m_parachute_time,            "parachute-time"             );
    CHECK_NEG(m_parachute_time_other,      "parachute-time-other"       );
    CHECK_NEG(m_bomb_time,                 "bomb-time"                  );
    CHECK_NEG(m_bomb_time_increase,        "bomb-time-increase"         );
    CHECK_NEG(m_anvil_time,                "anvil-time"                 );
    CHECK_NEG(m_anvil_weight,              "anvil-weight"               );
    CHECK_NEG(m_item_switch_time,          "item-switch-time"           );
    CHECK_NEG(m_bubble_gum_counter,        "bubblegum disappear counter");
    CHECK_NEG(m_explosion_impulse_objects, "explosion-impulse-objects"  );
    CHECK_NEG(m_max_history,               "max-history"                );
    CHECK_NEG(m_max_skidmarks,             "max-skidmarks"              );
    CHECK_NEG(m_min_kart_version,          "<kart-version min...>"      );
    CHECK_NEG(m_max_kart_version,          "<kart-version max=...>"     );
    CHECK_NEG(m_min_track_version,         "min-track-version"          );
    CHECK_NEG(m_max_track_version,         "max-track-version"          );
    CHECK_NEG(m_skid_fadeout_time,         "skid-fadeout-time"          );
    CHECK_NEG(m_near_ground,               "near-ground"                );
    CHECK_NEG(m_delay_finish_time,         "delay-finish-time"          );
    CHECK_NEG(m_music_credit_time,         "music-credit-time"          );
    CHECK_NEG(m_leader_time_per_kart,      "leader time-per-kart"       );
    CHECK_NEG(m_penalty_time,              "penalty-time"               );
    CHECK_NEG(m_max_display_news,          "max-display-news"           );

    m_kart_properties.checkAllSet(filename);
}   // load

// -----------------------------------------------------------------------------
/** Init all values with invalid defaults, which are tested later. This
 * guarantees that all parameters will indeed be initialised, and helps
 * finding typos.
 */
void STKConfig::init_defaults()
{
    m_anvil_weight             = m_parachute_friction        =
        m_parachute_time       = m_parachute_done_fraction   =
        m_parachute_time_other = m_anvil_speed_factor        =
        m_bomb_time            = m_bomb_time_increase        =
        m_anvil_time           = m_music_credit_time         =
        m_delay_finish_time    = m_skid_fadeout_time         =
        m_near_ground          = m_item_switch_time          = 
        m_penalty_time         = m_explosion_impulse_objects = UNDEFINED;
    m_bubble_gum_counter       = -100;
    m_max_karts                = -100;
    m_gp_order                 = -100;
    m_max_history              = -100;
    m_max_skidmarks            = -100;
    m_min_kart_version         = -100;
    m_max_kart_version         = -100;
    m_min_track_version        = -100;
    m_max_track_version        = -100;
    m_max_display_news         = -100;
    m_title_music              = NULL;
    m_enable_networking        = true;
    m_smooth_normals           = false;
    m_same_powerup_mode        = POWERUP_MODE_ONLY_IF_SAME;
    m_ai_acceleration          = 1.0f;
    
    m_score_increase.clear();
    m_leader_intervals.clear();
    m_switch_items.clear();
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

    if(const XMLNode *kart_node = root->getNode("karts"))
        kart_node->get("max-number", &m_max_karts);

    if(const XMLNode *gp_node = root->getNode("grand-prix"))
    {
        std::string order;
        gp_node->get("order", &order);
        m_gp_order = (order=="most-points-first");

        for(unsigned int i=0; i<gp_node->getNumNodes(); i++)
        {
            const XMLNode *pn=gp_node->getNode(i);
            int from=-1;
            pn->get("from", &from);
            int to=-1;
            pn->get("to", &to);
            if(to<0) to=m_max_karts;
            int points=-1;
            pn->get("points", &points);
            if(points<0 || from<0 || from>to||
               (int)m_score_increase.size()!=from-1)
            {
                fprintf(stderr, "Incorrect GP point specification:\n");
                fprintf(stderr, "from: %d  to: %d  points: %d\n", 
                        from, to, points);
                exit(-1);
            }
            for(int j=from; j<=to; j++)
                m_score_increase.push_back(points);
        }
    }

    if(const XMLNode *leader_node= root->getNode("follow-the-leader"))
    {
        leader_node->get("intervals",     &m_leader_intervals    );
        leader_node->get("time-per-kart", &m_leader_time_per_kart);
    }

    if (const XMLNode *physics_node= root->getNode("physics"))
    {
        physics_node->get("smooth-normals", &m_smooth_normals );
    }
    
    if (const XMLNode *startup_node= root->getNode("startup"))
    {
        startup_node->get("penalty", &m_penalty_time );
    }

    if (const XMLNode *news_node= root->getNode("news"))
    {
        news_node->get("max-display", &m_max_display_news);
    }

    if (const XMLNode *music_node = root->getNode("music"))
    {
        std::string title_music;
        music_node->get("title", &title_music);
        assert(title_music.size() > 0);
        
        m_title_music = MusicInformation::create(file_manager->getDataDir()
                                                 + "/music/" + title_music  );
        if(!m_title_music)
            fprintf(stderr, "Cannot load title music : %s\n", 
                    title_music.c_str());
    }

    if(const XMLNode *history_node = root->getNode("history"))
        history_node->get("max-frames", &m_max_history);

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

    
    if(const XMLNode *anvil_node= root->getNode("anvil"))
    {
        anvil_node->get("weight",       &m_anvil_weight      );
        anvil_node->get("speed-factor", &m_anvil_speed_factor);
        anvil_node->get("time",         &m_anvil_time        );
    }

    if(const XMLNode *parachute_node= root->getNode("parachute"))
    {
        parachute_node->get("friction",      &m_parachute_friction     );
        parachute_node->get("time",          &m_parachute_time         );
        parachute_node->get("time-other",    &m_parachute_time_other   );
        parachute_node->get("done-fraction", &m_parachute_done_fraction);
    }

    if(const XMLNode *bomb_node= root->getNode("bomb"))
    {
        bomb_node->get("time", &m_bomb_time);
        bomb_node->get("time-increase", &m_bomb_time_increase);
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
            printf("Invalid item mode '%s' - ignored.\n",
                    s.c_str());
        }
    }

    if(const XMLNode *switch_node= root->getNode("switch"))
    {
        switch_node->get("items", &m_switch_items    );
        switch_node->get("time",  &m_item_switch_time);
    }

    if(const XMLNode *bubble_gum_node= root->getNode("bubble-gum"))
    {
        bubble_gum_node->get("disappear-counter", &m_bubble_gum_counter);
    }

    if(const XMLNode *explosion_node= root->getNode("explosion"))
    {
        explosion_node->get("impulse-objects", &m_explosion_impulse_objects);
    }

    if(const XMLNode *ai_node = root->getNode("ai"))
    {
        ai_node->get("acceleration", &m_ai_acceleration);
    }
    
    if(const XMLNode *networking_node= root->getNode("networking"))
        networking_node->get("enable", &m_enable_networking);

    // Get the default KartProperties
    // ------------------------------
    const XMLNode *node = root -> getNode("general-kart-defaults");
    if(!node)
    {
        std::ostringstream msg;
        msg << "Couldn't load general-kart-defaults: no node.";
        throw std::runtime_error(msg.str());
    }
    m_kart_properties.getAllData(node);

}   // getAllData

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
    assert(num_karts <= m_max_karts);
    all_scores->resize(num_karts);
    (*all_scores)[num_karts-1] = 1;  // last position gets one point

    // Must be signed, in case that num_karts==1
    for(int i=num_karts-2; i>=0; i--)
    {
        (*all_scores)[i] = (*all_scores)[i+1] + m_score_increase[i];
    }
}   // getAllScores
