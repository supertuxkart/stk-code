// $Id: Config.cxx,v 1.11 2004/10/11 13:40:07 jamesgregory Exp $
//
//  TuxKart - a fun racing game with go-kart
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
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include "Config.h"

#include "lisp/Lisp.h"
#include "lisp/Parser.h"
#include "lisp/Writer.h"

#include "SDL.h"

Config config;

Config::Config()
{
  setDefaults();
}


Config::Config(const std::string& filename)
{
  setDefaults();
  loadConfig(filename);
}


Config::~Config()
{
}


/*set the config filename for this platform*/
void Config::setFilename()
{
#ifdef WIN32
  /*creates config file in current working directory*/
  filename = "tuxkart.cfg";
#else
  /*if HOME environment variable exists
  create config file as $HOME/.tuxkart/config*/
  if(getenv("HOME")!=NULL)
  {
    filename = getenv("HOME");
    filename += "/.tuxkart/config";
  }
#endif
}


/*load default values for options*/
void Config::setDefaults()
{
  setFilename();
  fullscreen = false;
  sound      = true;
  music      = true;
  smoke      = false;
  displayFPS = false;
  width      = 800;
  height     = 600;
  karts      = 4;
  player[0].setName("Player 1");
  player[1].setName("Player 2");
  player[2].setName("Player 3");
  player[3].setName("Player 4");
  /*player 1 default keyboard settings*/
  player[0].keys[KC_LEFT]    = SDLK_LEFT;
  player[0].keys[KC_RIGHT]   = SDLK_RIGHT;
  player[0].keys[KC_UP]      = SDLK_UP;
  player[0].keys[KC_DOWN]    = SDLK_DOWN;
  player[0].keys[KC_WHEELIE] = SDLK_a;
  player[0].keys[KC_JUMP]    = SDLK_s;
  player[0].keys[KC_RESCUE]  = SDLK_d;
  player[0].keys[KC_FIRE]    = SDLK_f;
  /*player 1 default joystick settings*/
  player[0].buttons[KC_UP]      = 0;
  player[0].buttons[KC_DOWN]    = 1;
  player[0].buttons[KC_WHEELIE] = 2;
  player[0].buttons[KC_JUMP]    = 3;
  player[0].buttons[KC_RESCUE]  = 4;
  player[0].buttons[KC_FIRE]    = 5;
  /*player 2 default keyboard settings*/
  player[1].keys[KC_LEFT]    = SDLK_j;
  player[1].keys[KC_RIGHT]   = SDLK_l;
  player[1].keys[KC_UP]      = SDLK_i;
  player[1].keys[KC_DOWN]    = SDLK_k;
  player[1].keys[KC_WHEELIE] = SDLK_q;
  player[1].keys[KC_JUMP]    = SDLK_w;
  player[1].keys[KC_RESCUE]  = SDLK_e;
  player[1].keys[KC_FIRE]    = SDLK_r;
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
}


/*load default configuration file for this platform*/
void Config::loadConfig()
{
  loadConfig(filename);
}


/*load configuration values from file*/
void Config::loadConfig(const std::string& filename)
{
  std::string temp;
  const lisp::Lisp* root = 0;
  int i;

  try
  {
    lisp::Parser parser;
    root = parser.parse(filename);

    const lisp::Lisp* lisp = root->getLisp("tuxkart-config");
    if(!lisp)
      throw std::runtime_error("No tuxkart-config node");

    /*get toggles*/
    lisp->get("fullscreen", fullscreen);
    lisp->get("sound", sound);
    lisp->get("music", music);
    lisp->get("smoke", smoke);
    lisp->get("displayFPS", displayFPS);

    /*get resolution width/height*/
    lisp->get("width", width);
    lisp->get("height", height);

    /*get number of karts*/
    lisp->get("karts", karts);

    /*get player configurations*/
    for(i=0; i<PLAYERS; ++i)
    {
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
  }
  catch(std::exception& e)
  {
    std::cout << "Error while parsing config '" << filename
              << "': " << e.what() << "\n";
  }
  delete root;
}


/*call saveConfig w/ the default filename for this platform*/
void Config::saveConfig()
{
  saveConfig(filename);
}


/*write settings to config file*/
void Config::saveConfig(const std::string& filename)
{
  std::string temp;
  int i;

  try
  {
    lisp::Writer writer(filename);

    writer.beginList("tuxkart-config");
    writer.writeComment("the following options can be set to #t or #f:");
    writer.write("fullscreen\t", fullscreen);
    writer.write("sound\t", sound);
    writer.write("music\t", music);
    writer.write("smoke\t", smoke);
    writer.write("displayFPS\t", displayFPS);

    writer.writeComment("screen resolution");
    writer.write("width\t", width);
    writer.write("height\t", height);

    writer.writeComment("number of karts. -1 means use all");
    writer.write("karts\t", karts);

    /*write player configurations*/
    for(i=0; i<PLAYERS; ++i)
    {
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
    }

    writer.endList("tuxkart-config");
  }
  catch(std::exception& e)
  {
    std::cout << "Couldn't write config: " << e.what() << "\n";
  }
}

/*EOF*/
