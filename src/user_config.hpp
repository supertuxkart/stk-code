// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//  Modelled after Supertux's configfile.h
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

#ifndef HEADER_USERCONFIG_HPP
#define HEADER_USERCONFIG_HPP

#define PLAYERS 4

/* The following config versions are currently used:
   0: the 0.2 release config file, without config-verison number
      (so that defaults to 0)
   1: Removed singleWindowMenu, newKeyboardStyle, oldStatusDisplay,
      added config-version number
      Version 1 can read version 0 without any problems, so 
      SUPPORTED_CONFIG_VERSION is 0.
   2: Changed to SDL keyboard bindings
   3: Added username
*/
#define CURRENT_CONFIG_VERSION   4
#define SUPPORTED_CONFIG_VERSION 3

#include <string>
#include "player.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"

#define CONFIGDIR ".supertuxkart"

/*class for managing general tuxkart configuration data*/
class UserConfig
{
private:
    std::string filename;

    void        setFilename      ();
    int         CheckAndCreateDir();
    std::string getConfigDir     ();

    void readInput(const lisp::Lisp* &r,
                   const char *node,
                   KartActions action,
                   Player& player);

    void writeInput(lisp::Writer &writer,
                    const char *node,
                    KartActions action,
                    Player& player);
public:
    bool        m_keyboard_debug;
    bool        m_fullscreen;
    bool        m_no_start_screen;
    bool        m_sfx;
    bool        m_music;
    bool        m_smoke;
    bool        m_display_fps;
    int         m_profile;         // Positive number: time in seconds, neg: # laps
                                   // 0 if no profiling. Never saved in config file!
    std::string m_herring_style;
    std::string m_username;
    bool        m_disable_magnet;   // true if a magnet can be dis- and enabled
    bool        m_replay_history;
    bool        m_use_kph;
    bool        m_improved_physics;
    int         m_width;
    int         m_height;
    int         m_karts;
    Player      m_player[PLAYERS];

    UserConfig();
    UserConfig(const std::string& filename);
    ~UserConfig();
    void setDefaults();
    void loadConfig();
    void loadConfig(const std::string& filename);
    void saveConfig();
    void saveConfig(const std::string& filename);
    std::string getInputAsString(int player_index, KartActions control);
};


extern UserConfig *user_config;

#endif

/*EOF*/
