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

#include "Config.h"
#include "lispreader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
	displayFPS = false;
	width      = 800;
	height     = 600;
	karts      = 4;
}


/*load default configuration file for this platform*/
void Config::loadConfig()
{
	loadConfig(filename);
}


/*load configuration values from file*/
void Config::loadConfig(const std::string& filename)
{
	/*make sure file exists/is readable*/
	FILE *file = fopen(filename.c_str(), "r");
	if(file) 
	{
                fclose(file);

		LispReader* config = LispReader::load(filename.c_str(), "tuxkart-config");
		LispReader reader(config->get_lisp());

		/*get toggles*/
		reader.read_bool("fullscreen", fullscreen);
		reader.read_bool("sound", sound);
		reader.read_bool("music", music);
		reader.read_bool("displayFPS", displayFPS);

		/*get resolution width/height*/
		reader.read_int("width", width);
		reader.read_int("height", height);

		/*get number of karts*/
		reader.read_int("karts", karts);
	}
}


/*call saveConfig w/ the default filename for this platform*/
void Config::saveConfig()
{
	saveConfig(filename);
}


/*write settings to config file
returns 0 on success, -1 on failure.*/
void Config::saveConfig(const std::string& filename)
{
	FILE *config = fopen(filename.c_str(), "w");

	if(config)
	{
                fclose(config);
		fprintf(config, "(tuxkart-config\n");

		fprintf(config, "\t;; the following options can be set to #t or #f:\n");
		fprintf(config, "\t(fullscreen %s)\n", fullscreen ? "#t" : "#f");
		fprintf(config, "\t(sound      %s)\n", sound      ? "#t" : "#f");
		fprintf(config, "\t(music      %s)\n", music      ? "#t" : "#f");
		fprintf(config, "\t(displayFPS %s)\n", displayFPS ? "#t" : "#f");

		fprintf(config, "\t;; screen resolution\n");
		fprintf(config, "\t(width      %d)\n", width);
		fprintf(config, "\t(height     %d)\n", height);

		fprintf(config, "\t;; number of karts. -1 means use all\n");
		fprintf(config, "\t(karts      %d)\n", karts);

		fprintf(config, ")\n");
	}
}

/*EOF*/
