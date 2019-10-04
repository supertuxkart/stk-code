//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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


#ifndef HEADER_PRESSAKEY_DIALOG_HPP
#define HEADER_PRESSAKEY_DIALOG_HPP

#include "guiengine/modaldialog.hpp"

/**
 * \brief Dialog shown when editing input bindings and it's time to press a key
 * \ingroup states_screens
 */
class PressAKeyDialog : public GUIEngine::ModalDialog
{
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    PressAKeyDialog(const float percentWidth, const float percentHeight, const bool isKeyboardFlag);
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
};

#endif
