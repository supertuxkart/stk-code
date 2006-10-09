// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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
#include <sstream>

// for mkdir:
#if !defined(WIN32) || defined(__CYGWIN__)
#  include <sys/stat.h>
#  include <sys/types.h>
#else
#  include <direct.h>
#endif

#include <SDL/SDL.h>
#include <plib/ul.h>

#include "config.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"

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
  filename += "supertuxkart.cfg";
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
  sfx              = true;
  music            = true;
  smoke            = false;
  displayFPS       = false;
  herringStyle     = "new";
  disableMagnet    = true;
  profile          = 0;
  useKPH           = false;
  improvedPhysics  = false;
  replayHistory    = false;
  width            = 800;
  height           = 600;
  karts            = 4;
  
  player[0].setName("Player 1");
  player[1].setName("Player 2");
  player[2].setName("Player 3");
  player[3].setName("Player 4");

  /*player 1 default keyboard settings*/
  player[0].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_LEFT,      0, 0);
  player[0].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_RIGHT,     0, 0);
  player[0].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_UP,        0, 0);
  player[0].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_DOWN,      0, 0);
  player[0].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_RSHIFT,    0, 0);
  player[0].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_MINUS,     0, 0);
  player[0].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_BACKSPACE, 0, 0);
  player[0].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_RCTRL,     0, 0);

  /*player 2 default keyboard settings*/
  player[1].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_a,         0, 0);
  player[1].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_d,         0, 0);
  player[1].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_w,         0, 0);
  player[1].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_s,         0, 0);
  player[1].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_LSHIFT,    0, 0);
  player[1].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_CAPSLOCK,  0, 0);
  player[1].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_LALT,      0, 0);
  player[1].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_LCTRL,     0, 0);

  /*player 3 default keyboard settings*/
  player[1].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_f,         0, 0);
  player[1].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_h,         0, 0);
  player[1].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_t,         0, 0);
  player[1].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_g,         0, 0);
  player[1].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_c,         0, 0);
  player[1].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_v,         0, 0);
  player[1].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_b,         0, 0);
  player[1].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_n,         0, 0);

  /*player 4 default keyboard settings*/
  player[1].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_j,         0, 0);
  player[1].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_l,         0, 0);
  player[1].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_i,         0, 0);
  player[1].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_k,         0, 0);
  player[1].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_m,         0, 0);
  player[1].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_COMMA,     0, 0);
  player[1].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_PERIOD,    0, 0);
  player[1].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_SLASH,     0, 0);
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
  bError = _mkdir(dirname.c_str()      ) != 0;
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
  } catch(std::exception& e) {
    std::cout << "Config file '"<<filename<<"' does not exist, it will be created.\n";
    delete root;
    return;
  }
  
  try {
    const lisp::Lisp* lisp = root->getLisp("tuxkart-config");
    if(!lisp)
      throw std::runtime_error("No tuxkart-config node");
    int configFileVersion = 0;
    lisp->get("configFileVersion", configFileVersion);
    if (configFileVersion < SUPPORTED_CONFIG_VERSION) {
      // Give some feedback to the user about what was changed.
      // Do NOT add a break after the case, so that all changes will be printed
      std::cout << "\nConfig file version '"<<configFileVersion<<"' is too old.\n";
      std::cout << "The following changes have been applied in the current SuperTuxKart version:\n";
      int needToAbort=0;
      switch(configFileVersion) {
        case 0:  std::cout << "- Single window menu, old status display,new keyboard style settings were removed\n";
                 needToAbort=0;
	case 1:  std::cout << "- Key bindings were changed, please check the settings. All existing values were discarded.\n";
                 needToAbort=1;  // if the old keybinds were read, they wouldn't make any sense
        case 99: break;
        default: std::cout << "Config file version " << configFileVersion 
			 << " is too old. Discarding your configuration. Sorry. :(\n";
	break;
      }
      if(needToAbort) {
	delete root;
	return;
      }
      std::cout << "This warning can be ignored.\n";
      // Keep on reading the config files as far as possible
    }   // if configFileVersion<SUPPORTED_CONFIG_VERSION

    /*get toggles*/
    lisp->get("fullscreen",       fullscreen);
    lisp->get("sfx" ,             sfx);
    lisp->get("nostartscreen",    noStartScreen);
    lisp->get("music",            music);
    lisp->get("smoke",            smoke);
    lisp->get("displayFPS",       displayFPS);
    lisp->get("herringStyle",     herringStyle);
    lisp->get("disableMagnet",    disableMagnet);
    lisp->get("useKPH",           useKPH);
    lisp->get("improvedPhysics",  improvedPhysics);

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
      std::string name;
      reader->get("name", name);
      player[i].setName(name);

      int lastKartId = 0;
      reader->get("lastKartId", lastKartId);
      player[i].setLastKartId(lastKartId);

      // Retrieves a player's input configuration
      readInput(reader, "left", KC_LEFT, player[i]);
      readInput(reader, "right", KC_RIGHT, player[i]);
      readInput(reader, "accel", KC_ACCEL, player[i]);
      readInput(reader, "brake", KC_BRAKE, player[i]);

      readInput(reader, "wheelie", KC_WHEELIE, player[i]);
      readInput(reader, "jump", KC_JUMP, player[i]);
      readInput(reader, "rescue", KC_RESCUE, player[i]);
      readInput(reader, "fire", KC_FIRE, player[i]);
    }
  } catch(std::exception& e) {
    std::cout << "Error while parsing config '" << filename << "': " << e.what() << "\n";
  }
  delete root;
}   // loadConfig

void Config::readInput(const lisp::Lisp* &r,
		       const char *node,
                       KartActions action,
                       Player& player)
{
  std::string inputTypeName;
  const lisp::Lisp* subReader = r->getLisp(node);
  InputType it=IT_KEYBOARD;
  // Every unused id variable *must* be set to
  // something different than -1. Otherwise
  // the restored mapping will not be applied
  // to the player.
  int id0 = -1, id1 = -1, id2 = -1;

  subReader->get("type", inputTypeName);
  if (inputTypeName == "keyboard")
    {
      it = IT_KEYBOARD;
      subReader->get("key", id0);
      id1 = id2 = 0;
    }
  else if (inputTypeName == "stickaxis")
    {
      it = IT_STICKMOTION;
      subReader->get("stick", id0);
      subReader->get("axis", id1);
      subReader->get("direction", id2);
    }
  else if (inputTypeName == "stickbutton")
    {
      it = IT_STICKBUTTON;
      subReader->get("stick", id0);
      subReader->get("button", id1);
      id2 = 0;
    }
  else if (inputTypeName == "stickhat")
    {
      it = IT_STICKHAT;
      // TODO: Implement me
    }
  else if (inputTypeName == "mouseaxis")
    {
      it = IT_MOUSEMOTION;
      subReader->get("axis", id0);
      subReader->get("direction", id1);
      id2 = 0;
    }
  else if (inputTypeName == "mousebutton")
    {
      it = IT_MOUSEBUTTON;
      subReader->get("button", id0);
      id1 = id2 = 0;
    }

  if (id0 != -1 && id1 != -1 && id2 != -1)
    player.setInput(action, it, id0, id1, id2);
}

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
    writer.writeComment("If the game's supported config file version is higher than this number the configuration is discarded.");
    writer.write("configFileVersion\t",   CURRENT_CONFIG_VERSION);

    writer.writeComment("the following options can be set to #t or #f:");
    writer.write("sfx\t",   sfx);
    writer.write("music\t", music);
    writer.write("smoke\t", smoke);
    writer.writeComment("Display frame per seconds");
    writer.write("displayFPS\t", displayFPS);
    writer.writeComment("Name of the .herring file to use.");
    writer.write("herringStyle\t", herringStyle);
    writer.writeComment("Allow players to disable a magnet");
    writer.write("disableMagnet\t", disableMagnet);
    writer.writeComment("Use of kilometers per hours (km/h) instead of mph");
    writer.write("useKPH\t", useKPH);
    writer.writeComment("With improved physics the gravity on a non-horizontal");
    writer.writeComment("plane will add an accelerating force on the kart");
    writer.write("improvedPhysics\t", improvedPhysics);

    writer.writeComment("screen resolution and windowing mode");
    writer.write("width\t", width);
    writer.write("height\t", height);
    writer.write("fullscreen\t", fullscreen);

    writer.writeComment("number of karts. -1 means use all");
    writer.write("karts\t", karts);

    /* write player configurations */
    for(i=0; i<PLAYERS; ++i) {
      temp = "player ";
      temp += i+'1';
      temp += " settings";
      writer.writeComment(temp);
      temp = "player-";
      temp += i+'1';
      writer.beginList(temp);

      writer.write("name\t", player[i].getName());

      writer.writeComment("optional");
      writer.write("lastKartId", player[i].getLastKartId());

      writeInput(writer, "left\t", KC_LEFT, player[i]);
      writeInput(writer, "right\t", KC_RIGHT, player[i]);
      writeInput(writer, "accel\t", KC_ACCEL, player[i]);
      writeInput(writer, "brake\t", KC_BRAKE, player[i]);
      writeInput(writer, "wheelie\t", KC_WHEELIE, player[i]);
      writeInput(writer, "jump\t", KC_JUMP, player[i]);
      writeInput(writer, "rescue\t", KC_RESCUE, player[i]);
      writeInput(writer, "fire\t", KC_FIRE, player[i]);

      writer.endList(temp);
    }   // for i

    writer.endList("tuxkart-config");
  } catch(std::exception& e) {
    std::cout << "Couldn't write config: " << e.what() << "\n";
  }
}   // saveConfig

void Config::writeInput(lisp::Writer &writer, const char *node, KartActions action, Player& player)
{
  Input *input = player.getInput(action);

  writer.beginList(node);

  switch (input->type)
    {
      case IT_KEYBOARD:
        writer.write("type", "keyboard");
        writer.write("key", input->id0);
        break;
      case IT_STICKMOTION:
        writer.write("type", "stickaxis");
        writer.write("stick", input->id0);
        writer.write("axis", input->id1);
        writer.writeComment("0 is negative/left/up, 1 is positive/right/down");
        writer.write("direction", input->id2);
        break;
      case IT_STICKBUTTON:
        writer.write("type", "stickbutton");
        writer.write("stick", input->id0);
        writer.write("button", input->id1);
        break;
      case IT_STICKHAT:
        // TODO: Implement me
        break;
      case IT_MOUSEMOTION:
        writer.write("type", "mouseaxis");
        writer.write("axis", input->id0);
        writer.writeComment("0 is negative/left/up, 1 is positive/right/down");
        writer.write("direction", input->id1);
        break;
      case IT_MOUSEBUTTON:
        writer.write("type", "mousebutton");
        writer.write("button", input->id0);
        break;
    }
  
  writer.endList(node);
}

// -----------------------------------------------------------------------------
std::string Config::getInputAsString(int player_index, KartActions control) {
  Input *input         = player[player_index].getInput(control);
  std::ostringstream stm;

  switch (input->type)
  {
    case IT_KEYBOARD:
      stm << SDL_GetKeyName((SDLKey) input->id0);
      break;
    case IT_STICKMOTION:
      stm << "joy " << input->id0 << " axis " << input->id1 << ((input->id2 == AD_NEGATIVE) ? " -" : " +");
      break;
    case IT_STICKBUTTON:
      stm << "joy " << input->id0 << " btn " << input->id1;
      break;
    case IT_STICKHAT:
      stm << "joy " << input->id0 << " hat " << input->id1;
      break;
    case IT_MOUSEBUTTON:
      stm << "mouse btn " << input->id0;
      break;
    case IT_MOUSEMOTION:
      stm << "mouse axis " << input->id0 << ((input->id1 == AD_NEGATIVE) ? " -" : " +");
      break;
    default:
      stm << "invalid";
  }
  return stm.str();
}   // GetKeyAsString

/*EOF*/
