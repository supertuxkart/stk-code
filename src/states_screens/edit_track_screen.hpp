//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Marc Coll
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

#ifndef HEADER_EDIT_TRACK_SCREEN_HPP
#define HEADER_EDIT_TRACK_SCREEN_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"

namespace GUIEngine { class Widget; }

namespace irr { namespace gui { class STKModifiedSpriteBank; } }

class Track;

/**
  * \brief screen where the user can edit the details of a track inside a grand prix
  * \ingroup states_screens
  */
class EditTrackScreen :
    public GUIEngine::Screen,
    public GUIEngine::ScreenSingleton<EditTrackScreen>
{
    friend class GUIEngine::ScreenSingleton<EditTrackScreen>;

    static const char* ALL_TRACKS_GROUP_ID;

    EditTrackScreen();

    void loadTrackList();
    void selectTrack(const std::string& id);

    std::string         m_track_group;

    Track*              m_track;
    unsigned int        m_laps;
    bool                m_reverse;
    bool                m_result;

    GUIEngine::IconButtonWidget* m_screenshot;

public:

                ~EditTrackScreen();

    void         setSelection(Track* track, unsigned int laps, bool reverse);
    Track*       getTrack() const;
    unsigned int getLaps() const;
    bool         getReverse() const;
    bool         getResult() const;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
        const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;
};

#endif
