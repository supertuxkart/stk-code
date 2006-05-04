// $Id: Config.cxx,v 1.8 2005/09/30 16:42:15 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//  Modelled after Supertux's configfile.cpp
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
#include <stdio.h>
#include <stdexcept>

// for mkdir:
#ifndef WIN32
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

#include <plib/pw.h>
#include <plib/ul.h>

#include "Config.h"
#include "lisp/Lisp.h"
#include "lisp/Parser.h"
#include "lisp/Writer.h"

Config *config;

Config::Config() {
  setDefaults();
  loadConfig();
}   // Config

// -----------------------------------------------------------------------------
Config::Config(const std::string& filename) {
  setDefaults();
  loadConfig(filename);
}   // Config


// -----------------------------------------------------------------------------
Config::~Config() {
}

// -----------------------------------------------------------------------------
std::string Config::getConfigDir() {
  std::string dirname;
#ifdef WIN32
  // For now the old windows config way is used: store a config file
  // in the current directory (in other OS a special subdirectory is created)
  dirname=".";
#else
  if(getenv("HOME")!=NULL) {
    dirname = getenv("HOME");
  } else{
    dirname = ".";
  }
  dirname += "/";
  dirname += CONFIGDIR;
#endif
  return dirname;
}  // getConfigDir

// -----------------------------------------------------------------------------
/*set the config filename for this platform*/
void Config::setFilename() {
  filename = getConfigDir();
  filename += "/";
#ifdef WIN32
  filename += "tuxkart.cfg";
#else
  filename += "config";
#endif
}   // setFilename

// -----------------------------------------------------------------------------
/*load default values for options*/
void Config::setDefaults() {
  setFilename();
  fullscreen       = false;
  noStartScreen    = false;
  sound            = true;
  music            = true;
  smoke            = false;
  displayFPS       = false;
  singleWindowMenu = false;
  oldStatusDisplay = false;
  herringStyle     = "default";
  disableMagnet    = false;
  profile          = 0;
  useKPH           = false;
  replayHistory    = false;
  width            = 800;
  height           = 600;
  karts            = 4;
  player[0].setName("Player 1");
  player[1].setName("Player 2");
  player[2].setName("Player 3");
  player[3].setName("Player 4");

  player[0].useJoy = true;
  player[1].useJoy = false;
  player[2].useJoy = false;
  player[3].useJoy = false;
  /*player 1 default keyboard settings*/
  player[0].keys[KC_LEFT]    = PW_KEY_LEFT;
  player[0].keys[KC_RIGHT]   = PW_KEY_RIGHT;
  player[0].keys[KC_UP]      = PW_KEY_UP;
  player[0].keys[KC_DOWN]    = PW_KEY_DOWN;
  player[0].keys[KC_WHEELIE] = 'a';
  player[0].keys[KC_JUMP]    = 's';
  player[0].keys[KC_RESCUE]  = 'd';
  player[0].keys[KC_FIRE]    = 'f';
  /*player 1 default joystick settings*/
  player[0].buttons[KC_UP]      = 2;
  player[0].buttons[KC_DOWN]    = 1;
  player[0].buttons[KC_WHEELIE] = 0x20;
  player[0].buttons[KC_JUMP]    = 0x10;
  player[0].buttons[KC_RESCUE]  = 0x04;
  player[0].buttons[KC_FIRE]    = 0x08;
  /*player 2 default keyboard settings*/
  player[1].keys[KC_LEFT]    = 'j';
  player[1].keys[KC_RIGHT]   = 'l';
  player[1].keys[KC_UP]      = 'i';
  player[1].keys[KC_DOWN]    = 'k';
  player[1].keys[KC_WHEELIE] = 'q';
  player[1].keys[KC_JUMP]    = 'w';
  player[1].keys[KC_RESCUE]  = 'e';
  player[1].keys[KC_FIRE]    = 'r';
  /*player 2 default joystick settings*/
  player[1].buttons[KC_UP]      = 0;
  player[1].buttons[KC_DOWN]    = 1;
  player[1].buttons[KC_WHEELIE] = 2;
  player[1].buttons[KC_JUMP]    = 3;
  player[1].buttons[KC_RESCUE]  = 4;
  player[1].buttons[KC_FIRE]    = 5;

  /*player 3 default joystick settings*/
  player[2].buttons[KC_UP]      = 0;
  player[2].buttons[KC_DOWN]    = 1;
  player[2].buttons[KC_WHEELIE] = 2;
  player[2].buttons[KC_JUMP]    = 3;
  player[2].buttons[KC_RESCUE]  = 4;
  player[2].buttons[KC_FIRE]    = 5;
  /*player 4 default joystick settings*/
  player[3].buttons[KC_UP]      = 0;
  player[3].buttons[KC_DOWN]    = 1;
  player[3].buttons[KC_WHEELIE] = 2;
  player[3].buttons[KC_JUMP]    = 3;
  player[3].buttons[KC_RESCUE]  = 4;
  player[3].buttons[KC_FIRE]    = 5;
}   // setDefaults


// -----------------------------------------------------------------------------
/*load default configuration file for this platform*/
void Config::loadConfig() {
  loadConfig(filename);
}   // loadConfig

// -----------------------------------------------------------------------------
// Checks for existance of the tuxkart configuration directory. If the 
// directory does not exist, it will be created. Return values:
// 1: config dir exists
// 2: does not exist, but was created
// 0: does not exist, and could not be created.
int Config::CheckAndCreateDir() {
  std::string dirname = getConfigDir();
  ulDir*      u       = ulOpenDir(dirname.c_str());
  if(u) {  // OK, directory exists
    ulCloseDir(u);
    return 1;
  }
  // The directory does not exist, try to create it
  int bError;
#if defined(WIN32) && !defined(__CYGWIN__)
  bError = mkdir(dirname.c_str()      ) != 0;
#else
  bError = mkdir(dirname.c_str(), 0755) != 0;
#endif
  if(bError) {
    fprintf(stderr, "Couldn't create '%s', config files will not be saved.\n",
	    dirname.c_str());
    return 0;
  } else {
    printf("Config directory '%s' successfully created.\n",dirname.c_str());
    return 2;
  }

}   // CheckAndCreateDir

// -----------------------------------------------------------------------------
/*load configuration values from file*/
void Config::loadConfig(const std::string& filename) {
  std::string temp;
  const lisp::Lisp* root = 0;
  int i;
  int dirExist = CheckAndCreateDir();
  // Check if the config directory exists. If not, exit without an error
  // message, an appropriate message was printed by CheckAndCreateDir
  if (dirExist != 1) return;
  
  try {
    lisp::Parser parser;
    root = parser.parse(filename);

    const lisp::Lisp* lisp = root->getLisp("tuxkart-config");
    if(!lisp)
      throw std::runtime_error("No tuxkart-config node");

    /*get toggles*/
    lisp->get("fullscreen",       fullscreen);
    lisp->get("sound",            sound);
    lisp->get("nostartscreen",    noStartScreen);
    lisp->get("music",            music);
    lisp->get("smoke",            smoke);
    lisp->get("displayFPS",       displayFPS);
    lisp->get("singlewindowmenu", singleWindowMenu);
    lisp->get("oldStatusDisplay", oldStatusDisplay);
    lisp->get("herringStyle",     herringStyle);
    lisp->get("disableMagnet",    disableMagnet);
    lisp->get("useKPH",           useKPH);

    /*get resolution width/height*/
    lisp->get("width",            width);
    lisp->get("height",           height);

    /*get number of karts*/
    lisp->get("karts", karts);

    /*get player configurations*/
    for(i=0; i<PLAYERS; ++i) {
      temp = "player-";
      temp += i+'1';

      const lisp::Lisp* reader = lisp->getLisp(temp);
      if(!reader) {
        temp = "No " + temp + " node";
        throw std::runtime_error(temp);
      }
      reader->get("name",     player[i].name);

      /*get keyboard configuration*/
      reader->get("left",    player[i].keys[KC_LEFT]);
      reader->get("right",   player[i].keys[KC_RIGHT]);
      reader->get("up",      player[i].keys[KC_UP]);
      reader->get("down",    player[i].keys[KC_DOWN]);
      reader->get("wheelie", player[i].keys[KC_WHEELIE]);
      reader->get("jump",    player[i].keys[KC_JUMP]);
      reader->get("rescue",  player[i].keys[KC_RESCUE]);
      reader->get("fire",    player[i].keys[KC_FIRE]);

      /*get joystick configuration*/
      reader->get("joy-up",      player[i].buttons[KC_UP]);
      reader->get("joy-down",    player[i].buttons[KC_DOWN]);
      reader->get("joy-wheelie", player[i].buttons[KC_WHEELIE]);
      reader->get("joy-jump",    player[i].buttons[KC_JUMP]);
      reader->get("joy-rescue",  player[i].buttons[KC_RESCUE]);
      reader->get("joy-fire",    player[i].buttons[KC_FIRE]);
    }
  } catch(std::exception& e) {
    std::cout << "Error while parsing config '" << filename
              << "': " << e.what() << "\n";
  }
  delete root;
}   // loadConfig


// -----------------------------------------------------------------------------
/*call saveConfig w/ the default filename for this platform*/
void Config::saveConfig() {
  saveConfig(filename);
}   // saveConfig

// -----------------------------------------------------------------------------
/*write settings to config file*/
void Config::saveConfig(const std::string& filename) {
  std::string temp;
  int i;

  int dirExist = CheckAndCreateDir();
  // Check if the config directory exists (again, since it was already checked 
  // when reading the config file - this is done in case that the problem was
  // fixed while tuxkart is running). If the directory does not exist and
  // can not be created, an error message was already printed to stderr,
  // and we can exit here without any further messages.
  if (dirExist == 0) return;

  try {
    lisp::Writer writer(filename);

    writer.beginList("tuxkart-config");
    writer.writeComment("the following options can be set to #t or #f:");
    writer.write("fullscreen\t", fullscreen);
    writer.write("sound\t", sound);
    writer.write("music\t", music);
    writer.write("smoke\t", smoke);
    writer.write("displayFPS\t", displayFPS);
    writer.write("singleWindowMenu\t", singleWindowMenu);
    writer.write("oldStatusDisplay\t", oldStatusDisplay);
    writer.write("herringStyle\t", herringStyle);
    writer.write("disableMagnet\t", disableMagnet);
    writer.write("useKPH\t", useKPH);

    writer.writeComment("screen resolution");
    writer.write("width\t", width);
    writer.write("height\t", height);

    writer.writeComment("number of karts. -1 means use all");
    writer.write("karts\t", karts);

    /*write player configurations*/
    for(i=0; i<PLAYERS; ++i) {
      temp = "player ";
      temp += i+'1';
      temp += " settings";
      writer.writeComment(temp);
      temp = "player-";
      temp += i+'1';
      writer.beginList(temp);

      writer.write("name\t", player[i].name);

      writer.writeComment("keyboard layout");
      writer.write("left\t",    player[i].keys[KC_LEFT]);
      writer.write("right\t",   player[i].keys[KC_RIGHT]);
      writer.write("up\t\t",    player[i].keys[KC_UP]);
      writer.write("down\t",    player[i].keys[KC_DOWN]);
      writer.write("wheelie\t", player[i].keys[KC_WHEELIE]);
      writer.write("jump\t",    player[i].keys[KC_JUMP]);
      writer.write("rescue\t",  player[i].keys[KC_RESCUE]);
      writer.write("fire\t",    player[i].keys[KC_FIRE]);

      writer.writeComment("joystick layout");
      writer.write("joy-up",        player[i].buttons[KC_UP]);
      writer.write("joy-down",      player[i].buttons[KC_DOWN]);
      writer.write("joy-wheelie\t", player[i].buttons[KC_WHEELIE]);
      writer.write("joy-jump\t",    player[i].buttons[KC_JUMP]);
      writer.write("joy-rescue\t",  player[i].buttons[KC_RESCUE]);
      writer.write("joy-fire\t",    player[i].buttons[KC_FIRE]);

      writer.endList(temp);
    }   // for i

    writer.endList("tuxkart-config");
  } catch(std::exception& e) {
    std::cout << "Couldn't write config: " << e.what() << "\n";
  }
}   // saveConfig

/*EOF*/
