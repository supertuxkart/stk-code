//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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


/*class for managing general tuxkart configuration data*/
class Config
{
	private:
		char *filename;

		void setFilename();

	public:
		bool fullscreen;
		bool sound;
		bool music;
		bool displayFPS;
		int width;
		int height;
		int karts;

		Config();
		Config(const char *filename);
		~Config();
		void setDefaults();
		void loadConfig();
		void loadConfig(const char *filename);
		void saveConfig();
		void saveConfig(const char *filename);
};


extern Config config;

#endif

/*EOF*/
