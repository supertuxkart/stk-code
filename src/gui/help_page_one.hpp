//  $Id: help_menu.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_HELPPAGEONE_H
#define HEADER_HELPPAGEONE_H

#include <string>
#include "base_gui.hpp"

class ssgTransform;
class ssgContext;

class HelpPageOne: public BaseGUI
{
private:
    ssgContext* m_context;
    ssgTransform* m_box;
    ssgTransform* m_banana;
    ssgTransform* m_silver_coin;
    ssgTransform* m_gold_coin;
    float m_clock;

public:
    HelpPageOne();
    ~HelpPageOne();
    void select ();

    void update(float dt);
};

#endif
