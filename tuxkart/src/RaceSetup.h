//  $Id: RaceSetup.h,v 1.2 2004/08/17 22:53:43 grumbel Exp $
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

#ifndef HEADER_RACESETUP_H
#define HEADER_RACESETUP_H

/** A class that manages all configurations that are needed for a
    single race */
class RaceSetup
{
public:
        enum RaceMode { RM_TIME_TRIAL, RM_QUICK_RACE, RM_GRAND_PRIX };

	RaceSetup() { 
                mode       = RM_QUICK_RACE;
                numLaps    = 3; 
                mirror     = false; 
                reverse    = false; 
                track      = 0;
                numKarts   = -1; // use all available karts
                numPlayers = 1; 
        }
        
        RaceMode  mode;
	int   numLaps;
	bool  mirror;
	bool  reverse;
	int   track;
	int   numKarts;
	int   numPlayers;
};

#endif

/* EOF */
