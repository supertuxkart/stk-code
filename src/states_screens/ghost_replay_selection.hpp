//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_GHOST_REPLAY_SELECTION_HPP
#define HEADER_GHOST_REPLAY_SELECTION_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"

namespace GUIEngine { class Widget; }

/**
  * \brief GhostReplaySelection
  * \ingroup states_screens
  */
class GhostReplaySelection : public GUIEngine::Screen,
                             public GUIEngine::ScreenSingleton<GhostReplaySelection>,
                             public GUIEngine::IListWidgetHeaderListener

{
    friend class GUIEngine::ScreenSingleton<GhostReplaySelection>;

private:
    GhostReplaySelection();
    ~GhostReplaySelection();

    GUIEngine::ListWidget *                     m_replay_list_widget;

public:

    void refresh();

    /** Load the addons into the main list.*/
    void loadList();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void onColumnClicked(int columnId) {};

    virtual void init() OVERRIDE;

    virtual void tearDown() OVERRIDE {};

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE {};

};   // GhostReplaySelection

#endif
