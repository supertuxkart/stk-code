//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#ifndef HEADER_RACEOVER_DIALOG_HPP
#define HEADER_RACEOVER_DIALOG_HPP

#include "guiengine/modaldialog.hpp"

namespace irr { namespace video { class ITexture; } }

/**
  * \ingroup states_screens
  */
class RaceOverDialog : public GUIEngine::ModalDialog
{
    void renderThreeStrikesGraph(const int x, const int y, const int w, const int h);

    int m_buttons_y_from;
    int m_rankings_y_bottom;

    /** A timer to make this display shown for a certain amount of time. */
    float m_auxiliary_timer;
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    RaceOverDialog(const float percentWidth, const float percentHeight);  
    
    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    
    virtual void escapePressed();
    
    virtual void onUpdate(float dt);
    bool    menuIsFinished();
};


#endif
