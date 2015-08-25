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

#ifndef HEADER_REPLAY_RECORDER_HPP
#define HEADER_REPLAY_RECORDER_HPP

#include "karts/controller/kart_control.hpp"
#include "replay/replay_base.hpp"

#include <vector>

/**
  * \ingroup replay
  */
class ReplayRecorder : public ReplayBase
{
private:

    /** A separate vector of Replay Events for all transforms. */
    std::vector< std::vector<TransformEvent> > m_transform_events;

    /** Time at which a transform was saved for the last time. */
    std::vector<float> m_last_saved_time;

    /** Counts the number of transform events for each kart. */
    std::vector<unsigned int> m_count_transforms;

    /** Stores the last skid state. */
    std::vector<KartControl::SkidControl> m_skid_control;

    std::vector< std::vector<KartReplayEvent> > m_kart_replay_event;

    /** Static pointer to the one instance of the replay object. */
    static ReplayRecorder *m_replay_recorder;

#ifdef DEBUG
    /** Counts overall number of events stored. */
    unsigned int m_count;

    /** Counts number of events skipped due to minimum time between events. */
    unsigned int m_count_skipped_time;

    /** Counts number of events skipped due to interpolation. */
    unsigned int m_count_skipped_interpolation;
#endif


          ReplayRecorder();
         ~ReplayRecorder();
public:
    void  init();
    void  update(float dt);
    void  reset();
    void  Save();

    // ------------------------------------------------------------------------
    /** Creates a new instance of the replay object. */
    static void create() {
        assert(!m_replay_recorder);
        m_replay_recorder = new ReplayRecorder();
    }
    // ------------------------------------------------------------------------
    /** Returns the instance of the replay object. Returns NULL if no
     *  recorder is available, i.e. recording can be disabled. */
    static ReplayRecorder *get() { return m_replay_recorder; }
    // ------------------------------------------------------------------------
    /** Delete the instance of the replay object. */
    static void destroy() { delete m_replay_recorder; m_replay_recorder=NULL; }
};   // ReplayRecorder

#endif
