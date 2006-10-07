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

#ifndef TUXKART_CONFIG_H
#define TUXKART_CONFIG_H

#define PLAYERS 4

/* The following config versions are currently used:
   0: the 0.2 release config file, without config-verison number
      (so that defaults to 0)
   1: Removed singleWindowMenu, newKeyboardStyle, oldStatusDisplay,
      added config-version number
   Version 1 can read version 0 without any problems, so 
   SUPPORTED_CONFIG_VERSION is 0.
*/
#define CURRENT_CONFIG_VERSION   2
#define SUPPORTED_CONFIG_VERSION 2

#include <string>
#include "player.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"

#define CONFIGDIR ".supertuxkart"

/*class for managing general tuxkart configuration data*/
class Config {
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
    bool   fullscreen;
    bool   noStartScreen;
    bool   sfx;
    bool   music;
    bool   smoke;
    bool   displayFPS;
    int    profile;         // Positive number: time in seconds, neg: # laps
                            // 0 if no profiling. Never saved in config file!
    std::string herringStyle;
    bool   disableMagnet;   // true if a magnet can be dis- and enabled
    bool   replayHistory;
    bool   useKPH;
    bool   improvedPhysics;
    int    width;
    int    height;
    int    karts;
    Player player[PLAYERS];

    Config();
    Config(const std::string& filename);
    ~Config();
    void setDefaults();
    void loadConfig();
    void loadConfig(const std::string& filename);
    void saveConfig();
    void saveConfig(const std::string& filename);
    std::string getInputAsString(int player_index, KartActions control);
};


extern Config *config;

#endif

/*EOF*/
