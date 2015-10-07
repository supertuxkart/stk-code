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


#ifndef HEADER_OPTIONS_SCREEN_DEVICE_HPP
#define HEADER_OPTIONS_SCREEN_DEVICE_HPP

#include <string>
#include <irrString.h>

#include "guiengine/screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"

namespace GUIEngine { class Widget; class ListWidget; }
class DeviceConfig;
namespace irr { namespace gui { class STKModifiedSpriteBank; } }


struct Input;


/**
  * \brief Input options screen
  * \ingroup states_screens
  */
class OptionsScreenDevice : public GUIEngine::Screen,
                        public GUIEngine::ScreenSingleton<OptionsScreenDevice>,
                        public MessageDialog::IConfirmDialogListener
{
    OptionsScreenDevice();

    void updateInputButtons();

    bool conflictsBetweenKbdConfig(PlayerAction action, PlayerAction from,
                                   PlayerAction to);

    /** The configuration to use. */
    DeviceConfig* m_config;

    void renameRow(GUIEngine::ListWidget* actions,
        int idRow,
        const irr::core::stringw &translatedName,
        PlayerAction action) const;

    void addListItem(GUIEngine::ListWidget* actions, PlayerAction pa);
    void addListItemSubheader(GUIEngine::ListWidget* actions,
        const char* id,
        const core::stringw& text);

public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenDevice>;

    /** Sets the configuration to be used. */
    void setDevice(DeviceConfig* config) { m_config = config; }

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name, const int playerID) OVERRIDE;

    /** \brief implement optional callback from parent class
     *  GUIEngine::Screen */
    virtual void unloaded() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement optional callback from parent class
     *  GUIEngine::Screen */
    virtual bool onEscapePressed() OVERRIDE;

    /**
      * \brief invoke in "input sensing" mode, when input was sensed.
      * Updates the input bindings accordingly with the sensed input.
      */
    void gotSensedInput(const Input& sensedInput);

    /** \brief Implement IConfirmDialogListener callback */
    virtual void onConfirm() OVERRIDE;

    virtual void beforeAddingWidget() OVERRIDE;
};

#endif
