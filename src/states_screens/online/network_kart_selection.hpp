//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef NETWORK_KART_SELECTION_HPP
#define NETWORK_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"
#include "guiengine/screen.hpp"

namespace GUIEngine
{
    class ProgressBarWidget;
}

#include <set>

class NetworkKartSelectionScreen : public KartSelectionScreen,
                  public GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>
{
private:
    /** Pointer to progress bar widget which is used as a timer 
     *  (going backwards). */
    GUIEngine::ProgressBarWidget *m_timer;

    friend class GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>;

    bool m_live_join;

    bool m_all_players_done;

    uint64_t m_exit_timeout;
protected:
    // ------------------------------------------------------------------------
    NetworkKartSelectionScreen() 
                        : KartSelectionScreen("online/network_karts.stkgui")
    {
        m_live_join = false;
        m_all_players_done = false;
    }
    // ------------------------------------------------------------------------
    ~NetworkKartSelectionScreen() {}
    // ------------------------------------------------------------------------
    virtual void allPlayersDone() OVERRIDE;

private:
    std::set<std::string> m_available_karts;

    // ------------------------------------------------------------------------
    virtual bool isIgnored(const std::string& ident) const OVERRIDE;
    // ------------------------------------------------------------------------
    void updateProgressBarText();
    // ------------------------------------------------------------------------
    virtual void beforeAddingWidget() OVERRIDE;

public:
    /** \brief Implement per-frame callback. */
    virtual void onUpdate(float dt) OVERRIDE;

    // ------------------------------------------------------------------------
    void setAvailableKartsFromServer(const std::set<std::string>& k)
                                                     { m_available_karts = k; }
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool onEscapePressed() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool playerQuit(StateManager::ActivePlayer* player) OVERRIDE
                                                               { return true; }
    // ------------------------------------------------------------------------
    void setLiveJoin(bool val)                           { m_live_join = val; }
};

#endif // NETWORK_KART_SELECTION_HPP
