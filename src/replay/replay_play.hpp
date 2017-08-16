//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_REPLAY__PLAY_HPP
#define HEADER_REPLAY__PLAY_HPP

#include "karts/ghost_kart.hpp"
#include "replay/replay_base.hpp"
#include "utils/ptr_vector.hpp"

#include <algorithm>
#include <string>
#include <vector>

class GhostKart;

/**
  * \ingroup replay
  */
class ReplayPlay : public ReplayBase
{

public:
    /** Order of sort for ReplayData */
    enum SortOrder
    {
        SO_DEFAULT,
        SO_TRACK = SO_DEFAULT,
        SO_REV,
        SO_KART_NUM,
        SO_DIFF,
        SO_LAPS,
        SO_TIME,
        SO_USER
    };

    class ReplayData
    {
    public:
        std::string                m_filename;
        std::string                m_track_name;
        core::stringw              m_user_name;
        std::vector<std::string>   m_kart_list;
        std::vector<core::stringw> m_name_list;
        bool                       m_reverse;
        bool                       m_custom_replay_file;
        unsigned int               m_difficulty;
        unsigned int               m_laps;
        float                      m_min_time;

        bool operator < (const ReplayData& r) const
        {
            switch (m_sort_order)
            {
                case SO_TRACK:
                    return m_track_name < r.m_track_name;
                    break;
                case SO_KART_NUM:
                    return m_kart_list.size() < r.m_kart_list.size();
                    break;
                case SO_REV:
                    return m_reverse < r.m_reverse;
                    break;
                case SO_DIFF:
                    return m_difficulty < r.m_difficulty;
                    break;
                case SO_LAPS:
                    return m_laps < r.m_laps;
                    break;
                case SO_TIME:
                    return m_min_time < r.m_min_time;
                    break;
                case SO_USER:
                    return m_user_name < r.m_user_name;
                    break;
            }   // switch
            return true;
        }   // operator <
    };   // ReplayData

private:
    static ReplayPlay       *m_replay_play;

    static SortOrder         m_sort_order;

    unsigned int             m_current_replay_file;

    std::vector<ReplayData>  m_replay_file_list;

    /** All ghost karts. */
    PtrVector<GhostKart>     m_ghost_karts;

          ReplayPlay();
         ~ReplayPlay();
    void  readKartData(FILE *fd, char *next_line);
public:
    void  reset();
    void  load();
    void  loadAllReplayFile();
    // ------------------------------------------------------------------------
    static void        setSortOrder(SortOrder so)       { m_sort_order = so; }
    // ------------------------------------------------------------------------
    void               sortReplay(bool reverse)
    {
        (reverse ? std::sort(m_replay_file_list.rbegin(),
            m_replay_file_list.rend()) : std::sort(m_replay_file_list.begin(),
            m_replay_file_list.end()));
    }
    // ------------------------------------------------------------------------
    void               setReplayFile(unsigned int n)
                                                { m_current_replay_file = n; }
    // ------------------------------------------------------------------------
    bool               addReplayFile(const std::string& fn,
                                     bool custom_replay = false);
    // ------------------------------------------------------------------------
    const ReplayData&  getReplayData(unsigned int n) const
                                          { return m_replay_file_list.at(n); }
    // ------------------------------------------------------------------------
    const unsigned int getNumReplayFile() const
                           { return (unsigned int)m_replay_file_list.size(); }
    // ------------------------------------------------------------------------
    GhostKart*         getGhostKart(int n)    { return m_ghost_karts.get(n); }
    // ------------------------------------------------------------------------
    const unsigned int getNumGhostKart() const
    {
        assert(m_replay_file_list.size() > 0);
        return (unsigned int)m_replay_file_list.at(m_current_replay_file)
                                               .m_kart_list.size();
    }   // getNumGhostKart
    // ------------------------------------------------------------------------
    const std::string& getGhostKartName(int n) const
    {
        assert(m_replay_file_list.size() > 0);
        return m_replay_file_list.at(m_current_replay_file).m_kart_list.at(n);
    }
    // ------------------------------------------------------------------------
    /** Creates a new instance of the replay object. */
    static void        create()          { m_replay_play = new ReplayPlay(); }
    // ------------------------------------------------------------------------
    /** Returns the instance of the replay object. */
    static ReplayPlay  *get()                        { return m_replay_play; }
    // ------------------------------------------------------------------------
    /** Delete the instance of the replay object. */
    static void        destroy()
                               { delete m_replay_play; m_replay_play = NULL; }
    // ------------------------------------------------------------------------
    /** Returns the filename that was opened. */
    virtual const std::string& getReplayFilename() const
    {
        assert(m_replay_file_list.size() > 0);
        return m_replay_file_list.at(m_current_replay_file).m_filename;
    }
    // ------------------------------------------------------------------------
};   // Replay

#endif
