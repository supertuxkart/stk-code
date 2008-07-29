//  $Id: track_sel.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_GRAND_PRIX_SELECT_H
#define HEADER_GRAND_PRIX_SELECT_H

#include <vector>
#include "base_gui.hpp"
#include "grand_prix_data.hpp"

class GrandPrixSelect: public BaseGUI
{
private:
    std::vector<std::string>  m_gp_tracks;
    std::vector<int>          m_track_imgs;
    std::vector<unsigned int> m_gp_index;
    unsigned int              m_curr_track_img;
    float                     m_clock;
public:
    GrandPrixSelect();
    ~GrandPrixSelect();

    void update(float dt);
    void select();
};

#endif
