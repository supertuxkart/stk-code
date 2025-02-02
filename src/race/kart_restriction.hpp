//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#ifndef HEADER_KARTRESTRICTION_HPP
#define HEADER_KARTRESTRICTION_HPP

// When used outside of server lobby context, migrate it to somewhere else.
enum KartRestrictionMode : int 
{
    NONE,                   // All karts can be played
    LIGHT,                  // Light type karts can be player
    MEDIUM,                 // Medium type karts can be used
    HEAVY,                  // Heavy type karts can be user
    // TODO: Same for addon karts, note that the players that have no addon karts won't be able to play.
    /*
    ADDON,
    ADDON_LIGHT,
    ADDON_MEDIUM,
    ADDON_HEAVY,
    // Same for vanilla karts
    VANILLA,
    VANILLA_LIGHT,
    VANILLA_MEDIUM,
    VANILLA_HEAVY, */
};

#endif

/* EOF */
