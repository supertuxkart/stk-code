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

#include "items/attachment.hpp"
#include "items/powerup_manager.hpp"
#include "karts/controller/kart_control.hpp"
#include "replay/replay_base.hpp"

#include <vector>

/**
  * \ingroup replay
  */
class ReplayRecorder : public ReplayBase
{
private:
    std::string m_filename;

    /** A separate vector of Replay Events for all transforms. */
    std::vector< std::vector<TransformEvent> > m_transform_events;

    /** A separate vector of Replay Events for all physic info. */
    std::vector< std::vector<PhysicInfo> > m_physic_info;

    /** A separate vector of Replay Events for all item/nitro info. */
    std::vector< std::vector<BonusInfo> > m_bonus_info;

    /** A separate vector of Replay Events for all other events. */
    std::vector< std::vector<KartReplayEvent> > m_kart_replay_event;

    /** Time at which a transform was saved for the last time. */
    std::vector<float> m_last_saved_time;

    /** Counts the number of transform events for each kart. */
    std::vector<unsigned int> m_count_transforms;

    /** Static pointer to the one instance of the replay object. */
    static ReplayRecorder *m_replay_recorder;

    bool  m_complete_replay;

    bool  m_incorrect_replay;

    unsigned int m_max_frames;

    // Stores the steering value at the previous transform.
    // Used to trigger the recording of new transforms.
    float m_previous_steer = 0.0f;

    const float DISTANCE_FAST_UPDATES = 10.0f;

    const float DISTANCE_MAX_UPDATES = 1.0f;

    uint64_t m_last_uid;

#ifdef DEBUG
    /** Counts overall number of events stored. */
    unsigned int m_count;

    /** Counts number of events skipped due to minimum time between events. */
    unsigned int m_count_skipped_time;

    /** Counts number of events skipped due to interpolation. */
    unsigned int m_count_skipped_interpolation;
#endif

    /** Compute the replay's UID ; partly based on race data ; partly randomly */
    uint64_t computeUID(float min_time);


          ReplayRecorder();
         ~ReplayRecorder();
public:
    void  init();
    void  reset();
    void  save();
    void  update(int ticks);

    const uint64_t getLastUID() { return m_last_uid; }

    /** Functions to encode and decode attahcments and item types,
        so that the stored value is independent from internal
        representation and resilient to such changes. */
    static int enumToCode (Attachment::AttachmentType type);
    static int enumToCode (PowerupManager::PowerupType type);
    static Attachment::AttachmentType codeToEnumAttach (int code);
    static PowerupManager::PowerupType codeToEnumItem (int code);

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
    // ------------------------------------------------------------------------
    /** Returns the filename that was opened. */
    virtual const std::string& getReplayFilename(int replay_file_number = 1) const { return m_filename; }
    // ------------------------------------------------------------------------
};   // ReplayRecorder

#endif

