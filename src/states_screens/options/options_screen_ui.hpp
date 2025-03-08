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


#ifndef __HEADER_OPTIONS_SCREEN_UI_HPP__
#define __HEADER_OPTIONS_SCREEN_UI_HPP__

#include <memory>
#include <string>

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget; }

/**
  * \brief Graphics options screen
  * \ingroup states_screens
  */
class OptionsScreenUI : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<OptionsScreenUI>
{
    struct ReloadOption
    {
        bool m_reload_font;
        bool m_reload_skin;
        std::string m_focus_name;
        bool m_focus_right;
    };

    struct SkinID
    {
        core::stringw m_base_theme_name;
        core::stringw m_variant_name;
        std::string m_folder_name;
    };
    std::unique_ptr<ReloadOption> m_reload_option;
    OptionsScreenUI();
    bool m_inited;

    std::vector<SkinID>        m_skins;
    std::vector<core::stringw> m_base_skins;
    std::vector<core::stringw> m_current_skin_variants;
    core::stringw              m_active_base_skin;

    GUIEngine::SpinnerWidget* m_base_skin_selector;
    GUIEngine::SpinnerWidget* m_variant_skin_selector;

    void updateCamera();

    void loadSkins(const std::set<std::string>& files, bool addon);
    void loadCurrentSkinVariants();
    int getBaseID(SkinID skin);
    int getVariantID(SkinID skin);
    std::string getCurrentSpinnerSkin();
    void onSkinChange(bool is_variant);
public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenUI>;

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

    void         updateCameraPresetSpinner();

    virtual void onUpdate(float delta) OVERRIDE;

    void reloadGUIEngine();
};

#endif
