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

#ifndef SERVER_ONLY // No GUI files in server builds
#ifndef __HEADER_OPTIONS_SCREEN_VIDEO_HPP__
#define __HEADER_OPTIONS_SCREEN_VIDEO_HPP__

#include <string>

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget; }

/**
  * \brief Graphics options screen
  * \ingroup states_screens
  */
class OptionsScreenVideo : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<OptionsScreenVideo>
{
private:
    bool m_prev_adv_pipline;
    OptionsScreenVideo();
    bool m_inited;

    void updateTooltip();
    void updateBlurTooltip();
    static void onScrollResolutionsList(void* data);
    /* Returns 1 or 2 if a restart will be done, 0 otherwise */
    int applySettings();
public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenVideo>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;

    virtual void onResize() OVERRIDE;

    void         updateGfxSlider();
    void         updateBlurSlider();
    void         updateScaleRTTsSlider();
    static int getImageQuality();
    static void setImageQuality(int quality, bool force_reload_texture);
    static void setSSR();
};

#endif
#endif // ifndef SERVER_ONLY