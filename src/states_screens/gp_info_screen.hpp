//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2014 Marianne Gagnon
//                2014      Joerg Henrichs
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


#ifndef HEADER_GP_INFO_SCREEN_HPP
#define HEADER_GP_INFO_SCREEN_HPP

#include "guiengine/screen.hpp"

class GrandPrixData;

namespace GUIEngine
{
    class IconButtonWidget;
}

/**
 * \brief Dialog that shows information about a specific grand prix
 * \ingroup states_screens
 */
class GPInfoScreen : public GUIEngine::Screen,
                     public GUIEngine::ScreenSingleton<GPInfoScreen>
{
protected: // Necessary for RandomGPInfoScreen
    GUIEngine::IconButtonWidget* m_screenshot_widget;
    float m_curr_time;

    /** The grand prix data. */
    GrandPrixData *m_gp;

    /** height of the separator over the body */
    int m_over_body;
    /** height of the seperator over the buttons */
    int m_lower_bound;

    /** \brief display all the tracks according to the current gp
     * For a normal gp info dialog, it just creates a label for every track.
     * But with a random gp info dialog, it tries to reuse as many
     * labels as possible by just changing their text. */
    void addTracks();
    void addScreenshot();

public:
    GPInfoScreen();
    /** Places the focus back on the selected GP, in the case that the dialog
     * was cancelled and we're returning to the track selection screen */
    virtual ~GPInfoScreen();

    void onEnterPressedInternal();
    virtual void eventCallback(GUIEngine::Widget *, const std::string &name,
                               const int player_id);
    virtual void loadedFromFile();
    virtual void init();

    virtual void onUpdate(float dt);

    void setGP(const std::string &gp_ident);
};

#endif
