// $Id: Config.cxx,v 1.6 2004/08/29 19:50:45 oaf_thadres Exp $
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
}


/*load default configuration file for this platform*/
void Config::loadConfig()
{
  loadConfig(filename);
}


/*load configuration values from file*/
void Config::loadConfig(const std::string& filename)
{
  const lisp::Lisp* lisp = 0;
  try
  {
    lisp::Parser parser;
    lisp = parser.parse(filename);

    lisp = lisp->getLisp("tuxkart-config");
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
  }
  catch(std::exception& e)
  {
    std::cout << "Error while parsing config '" << filename
              << "': " << e.what() << "\n";
  }
  delete lisp;
}


/*call saveConfig w/ the default filename for this platform*/
void Config::saveConfig()
{
  saveConfig(filename);
}


/*write settings to config file*/
void Config::saveConfig(const std::string& filename)
{
  try
  {
    lisp::Writer writer(filename);

    writer.beginList("tuxkart-config");
    writer.writeComment("the following options can be set to #t or #f:");
    writer.write("fullscreen", fullscreen);
    writer.write("sound", sound);
    writer.write("music", music);
    writer.write("smoke", smoke);
    writer.write("displayFPS", displayFPS);

    writer.writeComment("screen resolution");
    writer.write("width", width);
    writer.write("height", height);

    writer.writeComment("number of karts. -1 means use all");
    writer.write("karts", karts);

    writer.endList("tuxkart-config");
  }
  catch(std::exception& e)
  {
    std::cout << "Couldn't write config: " << e.what() << "\n";
  }
}

/*EOF*/
