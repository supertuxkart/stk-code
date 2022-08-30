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


#ifndef __HEADER_OPTIONS_SCREEN_VIDEO_HPP__
#define __HEADER_OPTIONS_SCREEN_VIDEO_HPP__

#include <string>

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget; }

struct GFXPreset
{
    bool lights;
    int shadows;
    bool bloom;
    bool lightshaft;
    bool glow;
    bool mlaa;
    bool ssao;
    bool light_scatter;
    bool animatedCharacters;
    int particles;
    int image_quality;
    bool degraded_ibl;
};

struct BlurPreset
{
    bool motionblur;
    /** Depth of field */
    bool dof;
};

struct ScaleRttsCustomPreset
{
    float value;
};

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
  * \brief Graphics options screen
  * \ingroup states_screens
  */
class OptionsScreenVideo : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<OptionsScreenVideo>
{
private:
    static bool m_fullscreen_checkbox_focus;
    bool m_prev_adv_pipline;
    int m_prev_img_quality;
    OptionsScreenVideo();
    bool m_inited;
    std::vector<GFXPreset> m_presets;
    std::vector<BlurPreset> m_blur_presets;
    std::vector<ScaleRttsCustomPreset> m_scale_rtts_custom_presets;
    std::vector<Resolution> m_resolutions;

    void updateTooltip();
    void updateBlurTooltip();
    void updateResolutionsList();
    void initPresets();
    static void onScrollResolutionsList(void* data);
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

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded() OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;

    void         updateGfxSlider();
    void         updateBlurSlider();
    void         updateScaleRTTsSlider();
    static int getImageQuality();
    static void setImageQuality(int quality);
};

#endif
