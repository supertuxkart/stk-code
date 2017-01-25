//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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


#ifndef HEADER_MULTITOUCH_SETTINGS_DIALOG_HPP
#define HEADER_MULTITOUCH_SETTINGS_DIALOG_HPP

#include "guiengine/modaldialog.hpp"

/**
 * \brief Dialog that allows the player to adjust multitouch steering settings
 * \ingroup states_screens
 */
class MultitouchSettingsDialog : public GUIEngine::ModalDialog
{
private:
    void updateValues();
    
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    MultitouchSettingsDialog(const float percentWidth, const float percentHeight);
    ~MultitouchSettingsDialog();

    virtual void beforeAddingWidgets();

    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    
};

#endif
