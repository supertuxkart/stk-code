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


#ifndef __HEADER_OPTIONS_SCREEN_INPUT_HPP__
#define __HEADER_OPTIONS_SCREEN_INPUT_HPP__

#include <string>
#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget; }
class DeviceConfig;
namespace irr { namespace gui { class STKModifiedSpriteBank; } namespace video { class IVideoDriver; } }


struct Input;

/**
  * \brief Input options screen
  * \ingroup states_screens
  */
class OptionsScreenInput : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<OptionsScreenInput>
{
    OptionsScreenInput();

    void updateInputButtons(DeviceConfig* config);
    void buildDeviceList();

    irr::gui::STKModifiedSpriteBank* m_icon_bank;

    std::map<std::string, float> m_highlights;

public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenInput>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init();

    /**
     * \brief invoke if the list of devices changed after the creation of this screen.
     * This will cause the displayed list to be updated accordingly with the data in the device manager.
     */
    void rebuildDeviceList();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void filterInput(Input::InputType type,
                             int deviceID,
                             int btnID,
                             int axisDir,
                             int value);

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt);
};

#endif
