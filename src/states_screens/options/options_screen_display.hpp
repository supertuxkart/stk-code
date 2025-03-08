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


#ifndef __HEADER_OPTIONS_SCREEN_DISPLAY_HPP__
#define __HEADER_OPTIONS_SCREEN_DISPLAY_HPP__

#include <string>

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget; }

struct Resolution
{
    int width;
    int height;
    bool fullscreen;

    Resolution()
    {
        width = 0;
        height = 0;
    }

    Resolution(int w, int h)
    {
        width = w;
        height = h;
    }

    bool operator< (Resolution r) const
    {
        return width < r.width || (width == r.width && height < r.height);
    }

    float getRatio() const
    {
        return (float) width / height;
    }
};

/**
  * \brief Display options screen
  * \ingroup states_screens
  */
class OptionsScreenDisplay : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<OptionsScreenDisplay>
{
private:
    static bool m_fullscreen_checkbox_focus;
    OptionsScreenDisplay();
    bool m_inited;
    std::vector<Resolution> m_resolutions;

    void updateResolutionsList();
    void configResolutionsList();

    static void onScrollResolutionsList(void* data);
public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenDisplay>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded() OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;

    virtual void onResize() OVERRIDE;

    void updateCamera();
};

#endif
