//  $Id$
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
#include "lisp/parser.hpp"
#include "audio/music_information.hpp"

STKConfig* stk_config=0;
float STKConfig::UNDEFINED = -99.9f;

//-----------------------------------------------------------------------------
/** Loads the stk configuration file. After loading it checks if all necessary
 *  values are actually defined, otherwise an error message is printed and STK
 *  is aborted.
 *  /param filename Name of the configuration file to load.
 */
void STKConfig::load(const std::string &filename)
{
    const lisp::Lisp* root = 0;

    try
    {
        lisp::Parser parser;
        root = parser.parse(filename);

        const lisp::Lisp* const LISP = root->getLisp("config");
        if(!LISP)
        {
            std::ostringstream msg;
            msg<<"No 'config' node found in '"<<filename<<"'.";
            throw std::runtime_error(msg.str());
        }
        getAllData(LISP);
    }
    catch(std::exception& err)
    {
        fprintf(stderr, "Error while parsing KartProperties '%s':\n",
                filename.c_str());
        fprintf(stderr, "%s", err.what());
        fprintf(stderr, "\n");
    }
    delete root;

    // Check that all necessary values are indeed set
    // -----------------------------------------------

#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                         \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n",    \
                strA,filename.c_str());exit(-1);                       \
    }

    if(m_scores.size()==0 || (int)m_scores.size()!=m_max_karts)
    {
        fprintf(stderr,"Not or not enough scores defined in stk_config");
        exit(-1);
    }
    if(m_leader_intervals.size()==0)
    {
        fprintf(stderr,"No follow leader interval(s) defined in stk_config");
        exit(-1);
    }
    if(m_menu_background.size()==0)
    {
        fprintf(stderr,"No menu background defined in stk_config");
        exit(-1);
    }
    if(m_mainmenu_background.size()==0)
    {
        fprintf(stderr,"No mainmenu background defined in stk_config");
        exit(-1);
    }
    CHECK_NEG(m_max_karts,                 "max-karts"                  );
    CHECK_NEG(m_grid_order,                "grid-order"                 );
    CHECK_NEG(m_parachute_friction,        "parachute-friction"         );
    CHECK_NEG(m_parachute_done_fraction,   "parachute-done-fraction"    );
    CHECK_NEG(m_parachute_time,            "parachute-time"             );
    CHECK_NEG(m_parachute_time_other,      "parachute-time-other"       );
    CHECK_NEG(m_bomb_time,                 "bomb-time"                  );
    CHECK_NEG(m_bomb_time_increase,        "bomb-time-increase"         );
    CHECK_NEG(m_anvil_time,                "anvil-time"                 );
    CHECK_NEG(m_anvil_weight,              "anvil-weight"               );
    CHECK_NEG(m_zipper_time,               "zipper-time"                );
    CHECK_NEG(m_zipper_force,              "zipper-force"               );
    CHECK_NEG(m_zipper_speed_gain,         "zipper-speed-gain"          );
    CHECK_NEG(m_zipper_max_speed_fraction, "zipper-max-speed-fraction"  );
    CHECK_NEG(m_final_camera_time,         "final-camera-time"          );
    CHECK_NEG(m_explosion_impulse,         "explosion-impulse"          );
    CHECK_NEG(m_explosion_impulse_objects, "explosion-impulse-objects"  );
    CHECK_NEG(m_max_history,               "max-history"                );
    CHECK_NEG(m_max_skidmarks,             "max-skidmarks"              );
    CHECK_NEG(m_min_kart_version,          "min-kart-version"           );
    CHECK_NEG(m_max_kart_version,          "max-kart-version"           );
    CHECK_NEG(m_min_track_version,         "min-track-version"          );
    CHECK_NEG(m_max_track_version,         "max-track-version"          );
    CHECK_NEG(m_skid_fadeout_time,         "skid-fadeout-time"          );
    CHECK_NEG(m_slowdown_factor,           "slowdown-factor"            );
    CHECK_NEG(m_near_ground,               "near-ground"                );
    CHECK_NEG(m_delay_finish_time,         "delay-finish-time"          );
    CHECK_NEG(m_music_credit_time,         "music-credit-time"          );
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
        m_anvil_time           = m_zipper_time               =
        m_zipper_force         = m_zipper_speed_gain         =
        m_zipper_max_speed_fraction =
        m_explosion_impulse    = m_explosion_impulse_objects =
        m_music_credit_time    = m_slowdown_factor           =
        m_delay_finish_time    = m_skid_fadeout_time         =
        m_final_camera_time    = m_near_ground               = UNDEFINED;
    m_max_karts                = -100;
    m_grid_order               = -100;
    m_max_history              = -100;
    m_max_skidmarks            = -100;
    m_min_kart_version         = -100;
    m_max_kart_version         = -100;
    m_min_track_version        = -100;
    m_max_track_version        = -100;
    m_title_music              = NULL;
    m_enable_networking        = true;
    m_scores.clear();
    m_leader_intervals.clear();
}   // init_defaults

//-----------------------------------------------------------------------------
const std::string &STKConfig::getMainMenuPicture(int n)
{
    if(n>=0 && n<(int)m_mainmenu_background.size())
        return m_mainmenu_background[n];
    else
        return m_mainmenu_background[0];
}   // getMainMenuPicture

//-----------------------------------------------------------------------------
const std::string &STKConfig::getBackgroundPicture(int n)
{
    if(n>=0 && n<(int)m_menu_background.size())
        return m_menu_background[n];
    else
        return m_menu_background[0];
}   // getBackgroundPicture

//-----------------------------------------------------------------------------
/** Extracts the actual information from a lisp file.
 *  \param lisp Pointer to the lisp data structure.
 */
void STKConfig::getAllData(const lisp::Lisp* lisp)
{

    // Get the values which are not part of the default KartProperties
    // ---------------------------------------------------------------
    lisp->get("anvil-weight",                 m_anvil_weight             );
    lisp->get("final-camera-time",            m_final_camera_time        );
    lisp->get("anvil-speed-factor",           m_anvil_speed_factor       );
    lisp->get("parachute-friction",           m_parachute_friction       );
    lisp->get("parachute-time",               m_parachute_time           );
    lisp->get("parachute-time-other",         m_parachute_time_other     );
    lisp->get("parachute-done-fraction",      m_parachute_done_fraction  );
    lisp->get("bomb-time",                    m_bomb_time                );
    lisp->get("bomb-time-increase",           m_bomb_time_increase       );
    lisp->getVector("leader-intervals",       m_leader_intervals         );
    lisp->get("anvil-time",                   m_anvil_time               );
    lisp->get("zipper-time",                  m_zipper_time              );
    lisp->get("zipper-force",                 m_zipper_force             );
    lisp->get("zipper-speed-gain",            m_zipper_speed_gain        );
    lisp->get("zipper-max-speed-fraction",    m_zipper_max_speed_fraction);
    lisp->get("explosion-impulse",            m_explosion_impulse        );
    lisp->get("explosion-impulse-objects",    m_explosion_impulse_objects);
    lisp->get("max-karts",                    m_max_karts                );
    lisp->get("grid-order",                   m_grid_order               );
    lisp->getVector("scores",                 m_scores                   );
    lisp->get("max-history",                  m_max_history              );
    lisp->get("max-skidmarks",                m_max_skidmarks            );
    lisp->get("min-kart-version",             m_min_kart_version         );
    lisp->get("max-kart-version",             m_max_kart_version         );
    lisp->get("min-track-version",            m_min_track_version        );
    lisp->get("max-track-version",            m_max_track_version        );
    lisp->get("skid-fadeout-time",            m_skid_fadeout_time        );
    lisp->get("slowdown-factor",              m_slowdown_factor          );
    lisp->get("near-ground",                  m_near_ground              );
    lisp->get("delay-finish-time",            m_delay_finish_time        );
    lisp->get("music-credit-time",            m_music_credit_time        );
    lisp->getVector("menu-background",        m_menu_background          );
    lisp->getVector("mainmenu-background",    m_mainmenu_background      );
    lisp->get("enable_networking",            m_enable_networking        );
    std::string title_music;
    lisp->get("title-music",                  title_music                );
    m_title_music = new MusicInformation(file_manager->getMusicFile(title_music));

    // Get the default KartProperties
    // ------------------------------
    m_kart_properties.getAllData(lisp->getLisp("kart-defaults"));

}   // getAllData
