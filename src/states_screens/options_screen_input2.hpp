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


#ifndef __HEADER_OPTIONS_SCREEN_INPUT2_HPP__
#define __HEADER_OPTIONS_SCREEN_INPUT2_HPP__

#include <string>
#include "irrlicht.h"

#include "guiengine/screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"

namespace GUIEngine { class Widget; }
class DeviceConfig;
namespace irr { namespace gui { class STKModifiedSpriteBank; } }


struct Input;


/**
  * \brief Input options screen
  * \ingroup states_screens
  */
class OptionsScreenInput2 : public GUIEngine::Screen,
                            public GUIEngine::ScreenSingleton<OptionsScreenInput2>,
                            public MessageDialog::IConfirmDialogListener
{
    OptionsScreenInput2();
    
    void updateInputButtons();

    DeviceConfig* m_config;
    
    irr::core::stringw makeLabel(const irr::core::stringw translatedName, PlayerAction action) const;

public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenInput2>;
    
    void setDevice(DeviceConfig* config) { m_config = config; }
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);
    
    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded();
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init();
    
    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual bool onEscapePressed();

    /**
      * \brief invoke in "input sensing" mode, when input was sensed.
      * Updates the input bindings accordingly with the sensed input.
      */
    void gotSensedInput(Input sensedInput);
    
    /** \brief Implement IConfirmDialogListener callback */
    virtual void onConfirm();
};

#endif
