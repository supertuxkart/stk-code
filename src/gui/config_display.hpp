//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_CONFIG_DISPLAY_HPP
#define HEADER_CONFIG_DISPLAY_HPP

#include <vector>

#include "gui/base_gui.hpp"
#include "gui/display_res_confirm.hpp"

class ConfigDisplay: public BaseGUI
{
public:
    ConfigDisplay();
    ~ConfigDisplay();

    void select();

private:
    std::vector< std::pair<int,int> > m_sizes;
    int m_curr_res;

    int m_curr_width;
    int m_curr_height;

    // changeResolution() reverse param is set true when changing to a previous resolution,
    // thought it is disabled for now.
    void changeResolution(int width, int height/*, bool reverse*/);
    void getScreenModes();
    void changeApplyButton();

#if 0
    // isBlacklisted() returns the index of the resolution in the blacklist 
    // or -1 if not in the blacklist
    int isBlacklisted(); 
#endif
    bool isBlacklisted(int width, int height);
    void showBlacklistButtons();

    void loadDefaultModes();
};

#endif
