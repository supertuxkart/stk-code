//  $Id: tuxkart.h,v 1.39 2004/08/20 22:32:31 jamesgregory Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_TUXKART_H
#define HEADER_TUXKART_H

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#ifdef __CYGWIN__
#include <unistd.h>
#endif
#include <windows.h>
#include <io.h>
//#include <direct.h>
#else
#include <unistd.h>
#endif
#include <math.h>

#include "sdldrv.h"

#include "KartProperties.h"
#include "guNet.h"
#include "constants.h"
#include "utils.h"
#include "gui/BaseGUI.h"

class WidgetSet;
class SoundSystem ;
class Track ;

extern bool use_fake_drift;

extern WidgetSet   *widgetSet ;
extern BaseGUI     *gui ;
extern SoundSystem *sound ;
extern std::vector<GUISwitch> guiStack;

void deinitTuxKart();

#define MAX_HOME_DIST  50.0f
#define MAX_HOME_DIST_SQD  (MAX_HOME_DIST * MAX_HOME_DIST)

#define DEFAULT_NUM_LAPS_IN_RACE 5

#ifndef TUXKART_DATADIR
#define TUXKART_DATADIR "/usr/local/share/games/tuxkart"
#endif

extern int finishing_position ;

#endif

/* EOF */
