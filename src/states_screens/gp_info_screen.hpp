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
#include "race/grand_prix_data.hpp"

class GrandPrixData;

namespace GUIEngine
{
    class IconButtonWidget;
    class SpinnerWidget;
}

/**
 * \brief Dialog that shows information about a specific grand prix
 * \ingroup states_screens
 */
class GPInfoScreen : public GUIEngine::Screen,
                     public GUIEngine::ScreenSingleton<GPInfoScreen>
{
private:
    /** Spinner for the different track groups. */
    GUIEngine::SpinnerWidget *m_group_spinner;

    /** Spinner for reverse mode. */
    GUIEngine::SpinnerWidget *m_reverse_spinner;

    /** Spinner for number of tracks (in case of random GP). */
    GUIEngine::SpinnerWidget *m_num_tracks_spinner;
    
    /** Spinner for number of AI karts. */
    GUIEngine::SpinnerWidget* m_ai_kart_spinner;

    /** The currently selected group name. */
    std::string m_group_name;

protected: // Necessary for RandomGPInfoScreen
    GUIEngine::IconButtonWidget* m_screenshot_widget;
    float m_curr_time;

    /** The grand prix data. */
    GrandPrixData m_gp;

    /** \brief display all the tracks according to the current gp
     * For a normal gp info dialog, it just creates a label for every track.
     * But with a random gp info dialog, it tries to reuse as many
     * labels as possible by just changing their text. */
    void addTracks();
    void addScreenshot();
    void updateRandomGP();
    GrandPrixData::GPReverseType getReverse() const;

public:
    GPInfoScreen();
    /** Places the focus back on the selected GP, in the case that the dialog
     * was cancelled and we're returning to the track selection screen */
    virtual ~GPInfoScreen() {}

    void onEnterPressedInternal();
    virtual void eventCallback(GUIEngine::Widget *, const std::string &name,
                               const int player_id);
    virtual void loadedFromFile() OVERRIDE;
    virtual void init() OVERRIDE;
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void onUpdate(float dt);

    void setGP(const std::string &gp_ident);
};   // GPInfoScreen

#endif
