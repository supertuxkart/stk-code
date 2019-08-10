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


#ifndef HEADER_CREDITS_HPP
#define HEADER_CREDITS_HPP

#include <rect.h>
namespace irr
{
    namespace video { class IVideoDriver; }
}

using namespace irr;

#include "audio/music_manager.hpp"
#include "guiengine/screen.hpp"
#include "utils/ptr_vector.hpp"


class CreditsSection;

/**
 * \brief Screen where STK credits are shown
 * \ingroup states_screens
 */
class CreditsScreen : public GUIEngine::Screen,
                      public GUIEngine::ScreenSingleton<CreditsScreen>
{
    float m_time_element;

    PtrVector<CreditsSection, HOLD> m_sections;
    CreditsSection* getCurrentSection();

    int m_x, m_y, m_w, m_h;
    core::rect< s32 > m_section_rect;

    int m_curr_section;
    int m_curr_element;

    float time_before_next_step;

    friend class GUIEngine::ScreenSingleton<CreditsScreen>;
    CreditsScreen();
    bool getLineAsWide(std::ifstream& file, core::stringw* out);

    bool m_is_victory_music;

public:


    void setArea(const int x, const int y, const int w, const int h);

    // start from beginning again
    void reset();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement optional callback from parent class
     *  GUIEngine::Screen */
    void onDraw(float dt) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                       const int playerID) OVERRIDE;

    void setVictoryMusic(bool isVictory) { m_is_victory_music = isVictory; }

    virtual MusicInformation* getMusic() const OVERRIDE
    {
        if (m_is_victory_music)
            return music_manager->getMusicInformation("win_theme.music");
        else
            return stk_config->m_title_music;
    }
};

#endif
