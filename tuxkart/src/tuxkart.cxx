//  $Id: tuxkart.cxx,v 1.62 2004/08/11 00:13:05 grumbel Exp $
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

#include <plib/ssg.h>
#include "tuxkart.h"
#include "Loader.h"
#include "Herring.h"
#include "KartDriver.h"
#include "Projectile.h"
#include "Explosion.h"
#include "Shadow.h"
#include "isect.h"

#include "status.h"
#include "Camera.h"
#include "level.h"
#include "WidgetSet.h"
#include "gui/BaseGUI.h"
#include "WorldLoader.h"
#include "TrackManager.h"

#include "gfx.h"
#include "preprocessor.h"
#include "material.h"
#include "RaceSetup.h"

#include <vector>

int finishing_position = -1 ;
 
int num_herring   ;

KartProperties kart_props;
HerringInstance herring [ MAX_HERRING ] ;

ulClock      *fclock = NULL ;
SoundSystem  *sound = NULL ;
WidgetSet          *widgetSet = NULL ;
BaseGUI	*gui = NULL;

std::vector<GUISwitch> guiStack;

Karts kart;
Projectile *projectile [ NUM_PROJECTILES ] ;
Explosion   *explosion [ NUM_EXPLOSIONS  ] ;

ssgRoot      *scene = NULL ;
Track        *track = NULL ;

void restartRace()
{
  finishing_position = -1 ;
  
  for ( Karts::iterator i = kart.begin(); i != kart.end() ; ++i )
    (*i)->reset() ;
}




void shutdown()
{
	if (gui)
		delete gui;
	
	if (widgetSet)
		delete widgetSet;

	shutdownVideo ();
  
	exit (0);
}

/* EOF */
