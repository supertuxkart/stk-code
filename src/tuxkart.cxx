//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
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

#include "Camera.h"
#include "WidgetSet.h"
#include "gui/BaseGUI.h"
#include "WorldLoader.h"
#include "TrackManager.h"

#include "preprocessor.h"
#include "material.h"
#include "RaceSetup.h"
#include "World.h"

#include <vector>

int finishing_position = -1 ;
bool use_fake_drift = true; 
int num_herring   ;

HerringInstance herring [ MAX_HERRING ] ;

SoundSystem  *sound = NULL ;
WidgetSet          *widgetSet = NULL ;
BaseGUI	*gui = NULL;

std::vector<GUISwitch> guiStack;

/* EOF */
