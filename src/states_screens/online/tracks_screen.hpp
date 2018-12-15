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

#ifndef HEADER_TRACKS_SCREEN_HPP
#define HEADER_TRACKS_SCREEN_HPP

#include "guiengine/screen.hpp"
#include "utils/synchronised.hpp"

#include <deque>
#include <string>
#include <vector>

class PeerVote;
class Track;

namespace GUIEngine
{
    class CheckBoxWidget;
    class CheckBoxWidget;
    class LabelWidget;
    class SpinnerWidget;
    class ProgressBarWidget;
}

/**
  * \brief screen where the user can select a track
  * \ingroup states_screens
  */
class TracksScreen : public GUIEngine::Screen,
                     public GUIEngine::ScreenSingleton<TracksScreen>
{
    friend class GUIEngine::ScreenSingleton<TracksScreen>;

private:

    Track* m_selected_track = NULL;
    GUIEngine::CheckBoxWidget* m_reversed;
    GUIEngine::SpinnerWidget* m_laps;

    /** Pointer to progress bar widget which is used as a timer
    *  (going backwards). */
    GUIEngine::ProgressBarWidget *m_timer;

    /** Maximum number of votes, as sent by the server. */
    unsigned int m_max_num_votes;

    bool m_network_tracks, m_reverse_checked, m_quit_server;

    int m_bottom_box_height;

    /** Index of the winning vote. */
    int m_winning_index;

    /** This stores which vote (hostid) is shown at which index in
     *  the vote overview list. */
    std::vector<int> m_index_to_hostid;

    std::deque<std::string> m_random_track_list;

    /** adds the tracks from the current track group into the tracks ribbon */
    void buildTrackList();

    void voteForPlayer();

    TracksScreen() : Screen("tracks.stkgui")
    {
        m_network_tracks = false;
        m_reverse_checked = false;
        m_quit_server = false;
        m_bottom_box_height = -1;
    }

public:

    void addVote(int host_id);
    void showVote(int host_id);
    void setResult(const PeerVote &winner_vote);
    void showVoteResult();
    void updateNumPlayers(int n);

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual bool onEscapePressed() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    void setFocusOnTrack(const std::string& trackName);
    // ------------------------------------------------------------------------
    void setNetworkTracks() { m_network_tracks = true; }
    // ------------------------------------------------------------------------
    void setQuitServer() { m_quit_server = true; }
    // ------------------------------------------------------------------------
    /** Called at the beginning of the voting process to reset any previous
     *  data fields. */
    void resetVote()
    {
        m_index_to_hostid.clear();
    }

};

#endif
