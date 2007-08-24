//  $Id: credits_menu.hpp 694 2006-08-29 07:42:36Z hiker $
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

#ifndef HEADER_SCROLL_TEXT_HPP
#define HEADER_SCROLL_TEXT_HPP

#include <string>
#include <vector>
#include "base_gui.hpp"
#include "player.hpp"


class ScrolledText: public BaseGUI
{
protected:
	typedef std::vector<std::string> StringList;

private:
    int         m_x_left, m_x_right, m_y_bottom, m_y_top;
    float       m_y_pos, m_y_speed;
    int         m_font_size;
    StringList  m_string_list;
    int         m_rect;

public:
    ScrolledText();
    ~ScrolledText();
    void setText      (StringList const &sl_);

    void select        ();
    void update        (float dt);
    void inputKeyboard (int key, int pressed);
};

#endif
