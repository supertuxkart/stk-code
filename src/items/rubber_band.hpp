//  $Id: rubber_band.hpp 2458 2008-11-15 02:12:28Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_RUBBER_BAND_HPP
#define HEADER_RUBBER_BAND_HPP

/** This class is used together with the pluger to display a rubber band from
 *  the shooting kart to the plunger.
 */
#define _WINSOCKAPI_
#include <plib/ssg.h>

class Kart;
class Plunger;

class RubberBand : public ssgVtxTable
{
private:
    Plunger        *m_plunger;
    const Kart     &m_kart;
    ssgSimpleState *m_state;

public:
         RubberBand(Plunger *plunger, const Kart &kart);
    void update(float dt);
    void removeFromScene();
};   // RubberBand
#endif
