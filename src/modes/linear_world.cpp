//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 SuperTuxKart-Team
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

#include "modes/linear_world.hpp"

#include "achievements/achievements_manager.hpp"
#include "config/player_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track_sector.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>

//-----------------------------------------------------------------------------
/** Constructs the linear world. Note that here no functions can be called
 *  that use World::getWorld(), since it is not yet defined.
 */
LinearWorld::LinearWorld() : WorldWithRank()
{
    m_last_lap_sfx         = SFXManager::get()->createSoundSource("last_lap_fanfare");
    m_last_lap_sfx_played  = false;
    m_last_lap_sfx_playing = false;
    m_fastest_lap          = 9999999.9f;
}   // LinearWorld

// ----------------------------------------------------------------------------
/** Actually initialises the world, i.e. creates all data structures to
 *  for all karts etc. In init functions can be called that use
 *  World::getWorld().
 */
void LinearWorld::init()
{
    WorldWithRank::init();

    assert(!m_track->isArena());
    assert(!m_track->isSoccer());

    m_last_lap_sfx_played           = false;
    m_last_lap_sfx_playing          = false;

    // The values are initialised in reset()
    m_kart_info.resize(m_karts.size());
}   // init

//-----------------------------------------------------------------------------
/** The destructor frees al data structures.
 */
LinearWorld::~LinearWorld()
{
    m_last_lap_sfx->deleteSFX();
}   // ~LinearWorld

//-----------------------------------------------------------------------------
/** Called before a race is started (or restarted). It resets the data
 *  structures that keep track of position and distance long track of
 *  all karts.
 */
void LinearWorld::reset()
{
    WorldWithRank::reset();
    m_last_lap_sfx_played = false;
    m_last_lap_sfx_playing = false;

    const unsigned int kart_amount = (unsigned int) m_karts.size();
    for(unsigned int i=0; i<kart_amount; i++)
    {
        m_kart_info[i].reset();
        m_kart_info[i].getTrackSector()->update(m_karts[i]->getXYZ());
        m_karts[i]->setWrongwayCounter(0);
    }   // next kart

    // At the moment the last kart would be the one that is furthest away
    // from the start line, i.e. it would determine the amount by which
    // the track length must be extended (to avoid negative numbers in
    // estimateFinishTimeForKart()). On the other hand future game modes
    // might not follow this rule, so check the distance for all karts here:
    m_distance_increase = m_track->getTrackLength();
    for(unsigned int i=0; i<kart_amount; i++)
    {
        m_distance_increase = std::min(m_distance_increase,
                                       getDistanceDownTrackForKart(i));
    }
    // Track length - minimum distance is how much the track length must
    // be increased to avoid negative values in estimateFinishTimeForKart
    // Increase this value somewhat in case that a kart drivess/slides
    // backwards a little bit at start.
    m_distance_increase = m_track->getTrackLength() - m_distance_increase
                        + 5.0f;

    if(m_distance_increase<0) m_distance_increase = 1.0f;  // shouldn't happen

    // First all kart infos must be updated before the kart position can be
    // recomputed, since otherwise 'new' (initialised) valued will be compared
    // with old values.
    updateRacePosition();

#ifdef DEBUG
    //FIXME: this could be defined somewhere in a central header so it can
    //       be used everywhere
#define assertExpr( ARG1, OP, ARG2 ) if (!(ARG1 OP ARG2)) \
        { \
            std::cerr << "Failed assert " << #ARG1 << #OP << #ARG2 << " @ " \
                      << __FILE__ << ":" << __LINE__ \
                      << "; values are (" << ARG1 << #OP << ARG2 << ")\n"; \
            assert(false); \
        }

    for (unsigned int i=0; i<kart_amount; i++)
    {
        for (unsigned int j=i+1; j<kart_amount; j++)
        {
            assertExpr( m_karts[i]->getPosition(), !=,
                        m_karts[j]->getPosition() );
        }
    }
#endif

}   // reset

//-----------------------------------------------------------------------------
/** General update function called once per frame. This updates the kart
 *  sectors, which are then used to determine the kart positions.
 *  \param dt Time step size.
 */
void LinearWorld::update(float dt)
{
    // run generic parent stuff that applies to all modes. It
    // especially updates the kart positions.
    WorldWithRank::update(dt);

    if (m_last_lap_sfx_playing &&
        m_last_lap_sfx->getStatus() != SFXBase::SFX_PLAYING)
    {
        if(music_manager->getCurrentMusic())
            music_manager->getCurrentMusic()->resetTemporaryVolume();
        m_last_lap_sfx_playing = false;
    }

    const unsigned int kart_amount = getNumKarts();

    // Do stuff specific to this subtype of race.
    // ------------------------------------------
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& kart_info = m_kart_info[n];
        AbstractKart* kart = m_karts[n];

        // Nothing to do for karts that are currently being
        // rescued or eliminated
        if(kart->getKartAnimation()) continue;

        kart_info.getTrackSector()->update(kart->getFrontXYZ());
        kart_info.m_overall_distance = kart_info.m_race_lap
                                     * m_track->getTrackLength()
                        + getDistanceDownTrackForKart(kart->getWorldKartId());
    }   // for n

    // Update all positions. This must be done after _all_ karts have
    // updated their position and laps etc, otherwise inconsistencies
    // (like two karts at same position) can occur.
    // ---------------------------------------------------------------
    WorldWithRank::updateTrack(dt);
    updateRacePosition();

    for (unsigned int i=0; i<kart_amount; i++)
    {
        // ---------- update rank ------
        if (m_karts[i]->hasFinishedRace() ||
            m_karts[i]->isEliminated()       ) continue;

        // During the last lap update the estimated finish time.
        // This is used to play the faster music, and by the AI
        if (m_kart_info[i].m_race_lap == race_manager->getNumLaps()-1)
        {
            m_kart_info[i].m_estimated_finish =
                estimateFinishTimeForKart(m_karts[i]);
        }
        checkForWrongDirection(i, dt);
    }

#ifdef DEBUG
    // Debug output in case that the double position error occurs again.
    std::vector<int> pos_used;
    pos_used.resize(kart_amount+1, -99);
    for(unsigned int i=0; i<kart_amount; i++)
    {
        if(pos_used[m_karts[i]->getPosition()]!=-99)
        {
            for(unsigned int j=0; j<kart_amount; j++)
            {
                Log::verbose("[LinearWorld]", "kart id=%u, position=%d, finished=%d, laps=%d, "
                       "distanceDownTrack=%f overallDistance=%f %s",
                    j, m_karts[j]->getPosition(),
                    m_karts[j]->hasFinishedRace(),
                    m_kart_info[j].m_race_lap,
                    getDistanceDownTrackForKart(m_karts[j]->getWorldKartId()),
                    m_kart_info[j].m_overall_distance,
                    (m_karts[j]->getPosition() == m_karts[i]->getPosition()
                     ? "<--- !!!" : "")                                      );
            }
        }
        pos_used[m_karts[i]->getPosition()]=i;
    }
#endif
}   // update

//-----------------------------------------------------------------------------
/** Is called by check structures if a kart starts a new lap.
 *  \param kart_index Index of the kart.
 */
void LinearWorld::newLap(unsigned int kart_index)
{
    KartInfo &kart_info = m_kart_info[kart_index];
    AbstractKart *kart  = m_karts[kart_index];

    // Reset reset-after-lap achievements
    StateManager::ActivePlayer *c = kart->getController()->getPlayer();
    PlayerProfile *p = PlayerManager::getCurrentPlayer();
    if (c && c->getConstProfile() == p)
    {
        p->getAchievementsStatus()->onLapEnd();
    }

    // Only update the kart controller if a kart that has already finished
    // the race crosses the start line again. This avoids 'fastest lap'
    // messages if the end controller does a fastest lap, but especially
    // allows the end controller to switch end cameras
    if(kart->hasFinishedRace())
    {
        kart->getController()->newLap(kart_info.m_race_lap);
        return;
    }

    const int lap_count = race_manager->getNumLaps();

    // Only increase the lap counter and set the new time if the
    // kart hasn't already finished the race (otherwise the race_gui
    // will begin another countdown).
    if(kart_info.m_race_lap+1 <= lap_count)
    {
        assert(kart->getWorldKartId()==kart_index);
        kart_info.m_time_at_last_lap=getTime();
        kart_info.m_race_lap++;
        m_kart_info[kart_index].m_overall_distance =
              m_kart_info[kart_index].m_race_lap * m_track->getTrackLength()
            + getDistanceDownTrackForKart(kart->getWorldKartId());
    }
    // Last lap message (kart_index's assert in previous block already)
    if (raceHasLaps() && kart_info.m_race_lap+1 == lap_count)
    {
        m_race_gui->addMessage(_("Final lap!"), kart,
                               3.0f, video::SColor(255, 210, 100, 50), true);
        if(!m_last_lap_sfx_played && lap_count > 1)
        {
            if (UserConfigParams::m_music)
            {
                m_last_lap_sfx->play();
                m_last_lap_sfx_played = true;
                m_last_lap_sfx_playing = true;

                // In case that no music is defined
                if(music_manager->getCurrentMusic() &&
                    music_manager->getMasterMusicVolume() > 0.2f)
                {
                    music_manager->getCurrentMusic()->setTemporaryVolume(0.2f);
                }
            }
            else
            {
                m_last_lap_sfx_played = true;
                m_last_lap_sfx_playing = false;
            }
        }
    }
    else if (raceHasLaps() && kart_info.m_race_lap > 0 &&
             kart_info.m_race_lap+1 < lap_count)
    {
        m_race_gui->addMessage(_("Lap %i", kart_info.m_race_lap+1),
                               kart, 3.0f, video::SColor(255, 210, 100, 50),
                               true);
    }

    // The race positions must be updated here: consider the situation where
    // the first kart does not cross the finish line in its last lap, instead
    // it passes it, the kart reverses and crosses the finishing line
    // backwards. Just before crossing the finishing line the kart will be on
    // the last lap, but with a distance along the track close to zero.
    // Therefore its position will be wrong. If the race position gets updated
    // after increasing the number of laps (but before tagging the kart to have
    // finished the race) the position will be correct (since the kart now
    // has one additional lap it will be ahead of the other karts).
    // Without this call the incorrect position for this kart would remain
    // (since a kart that has finished the race does not get its position
    // changed anymore), potentially resulting in a duplicated race position
    // (since the first kart does not have position 1, no other kart can get
    // position 1, so one rank will be duplicated).
    // Similarly the situation can happen if the distance along track should
    // go back to zero before actually crossing the finishing line. While this
    // should not happen, it could potentially be caused by floating point
    // errors. In this case the call to updateRacePosition will avoid
    // duplicated race positions as well.
    updateRacePosition();

    // Race finished
    if(kart_info.m_race_lap >= race_manager->getNumLaps() && raceHasLaps())
    {
        kart->finishedRace(getTime());
    }
    float time_per_lap;
    if (kart_info.m_race_lap == 1) // just completed first lap
    {
        time_per_lap=getTime();
    }
    else //completing subsequent laps
    {
        time_per_lap=getTime() - kart_info.m_lap_start_time;
    }

    // if new fastest lap
    if(time_per_lap < m_fastest_lap && raceHasLaps() &&
        kart_info.m_race_lap>0)
    {
        m_fastest_lap = time_per_lap;

        std::string s = StringUtils::timeToString(time_per_lap);

        irr::core::stringw m_fastest_lap_message;
        //I18N: as in "fastest lap: 60 seconds by Wilber"
        m_fastest_lap_message += _C("fastest_lap", "%s by %s", s.c_str(),
                                    core::stringw(kart->getName()));

        m_race_gui->addMessage(m_fastest_lap_message, NULL,
                               3.0f, video::SColor(255, 255, 255, 255), false);

        m_race_gui->addMessage(_("New fastest lap"), NULL,
                               3.0f, video::SColor(255, 255, 255, 255), false);

    } // end if new fastest lap

    kart_info.m_lap_start_time = getTime();
    kart->getController()->newLap(kart_info.m_race_lap);
}   // newLap

//-----------------------------------------------------------------------------
/** Gets the sector a kart is on. This function returns UNKNOWN_SECTOR if the
 *  kart_id is larger than the current kart info. This is necessary in the case
 *  that a collision with the track happens during resetAllKarts: at this time
 *  m_kart_info is not initialised (and has size 0), so it would trigger this
 *  assert. While this normally does not happen, it is useful for track
 *  designers that STK does not crash.
 *  \param kart_id The world kart id of the kart for which to return
 *                 the sector.
 */
int LinearWorld::getSectorForKart(const AbstractKart *kart) const
{
    if(kart->getWorldKartId()>=m_kart_info.size())
        return QuadGraph::UNKNOWN_SECTOR;
    return m_kart_info[kart->getWorldKartId()].getTrackSector()
          ->getCurrentGraphNode();
}   // getSectorForKart

//-----------------------------------------------------------------------------
/** Returns the distance the kart has travelled along the track since
 *  crossing the start line..
 *  \param kart_id Index of the kart.
 */
float LinearWorld::getDistanceDownTrackForKart(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return m_kart_info[kart_id].getTrackSector()->getDistanceFromStart();
}   // getDistanceDownTrackForKart

//-----------------------------------------------------------------------------
/** Gets the distance of the kart from the center of the driveline. Positive
 *  is to the right of the center, negative values to the left.
 *  \param kart_id Index of kart.
 */
float LinearWorld::getDistanceToCenterForKart(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return m_kart_info[kart_id].getTrackSector()->getDistanceToCenter();
}   // getDistanceToCenterForKart

//-----------------------------------------------------------------------------
int LinearWorld::getLapForKart(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return  m_kart_info[kart_id].m_race_lap;
}   // getLapForKart

//-----------------------------------------------------------------------------
/** Returns the estimated finishing time. Only valid during the last lap!
 *  \param kart_id Id of the kart.
 */
float LinearWorld::getEstimatedFinishTime(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    assert(m_kart_info[kart_id].m_race_lap == race_manager->getNumLaps()-1);
    return m_kart_info[kart_id].m_estimated_finish;
}   // getEstimatedFinishTime

//-----------------------------------------------------------------------------
float LinearWorld::getTimeAtLapForKart(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return m_kart_info[kart_id].m_time_at_last_lap;
}   // getTimeAtLapForKart

//-----------------------------------------------------------------------------
void LinearWorld::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    int   laps_of_leader       = -1;
    float time_of_leader       = -1;
    // Find the best time for the lap. We can't simply use
    // the time of the kart at position 1, since the kart
    // might have been overtaken by now
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        AbstractKart* kart = m_karts[i];

        // reset color
        rank_info.m_color = video::SColor(255, 255, 255, 255);
        rank_info.lap = -1;

        if(kart->isEliminated()) continue;
        const float lap_time = getTimeAtLapForKart(kart->getWorldKartId());
        const int current_lap  = getLapForKart( kart->getWorldKartId() );
        rank_info.lap = current_lap;

        if(current_lap > laps_of_leader)
        {
            // more laps than current leader --> new leader and
            // new time computation
            laps_of_leader = current_lap;
            time_of_leader = lap_time;
        } else if(current_lap == laps_of_leader)
        {
            // Same number of laps as leader: use fastest time
            time_of_leader=std::min(time_of_leader,lap_time);
        }
    }

    // we now know the best time of the lap. fill the remaining bits of info
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        KartInfo& kart_info = m_kart_info[i];
        AbstractKart* kart = m_karts[i];

        const int position = kart->getPosition();

        // Don't compare times when crossing the start line first
        if(laps_of_leader>0                                                &&
           (getTime() - getTimeAtLapForKart(kart->getWorldKartId())<5.0f ||
            rank_info.lap != laps_of_leader)                               &&
            raceHasLaps())
        {  // Display for 5 seconds
            std::string str;
            if(position == 1)
            {
                str = " " + StringUtils::timeToString(
                                 getTimeAtLapForKart(kart->getWorldKartId()) );
            }
            else
            {
                float timeBehind;
                timeBehind = (kart_info.m_race_lap==laps_of_leader
                                ? getTimeAtLapForKart(kart->getWorldKartId())
                                : getTime())
                           - time_of_leader;
                str = "+" + StringUtils::timeToString(timeBehind);
            }
            rank_info.m_text = irr::core::stringw(str.c_str());
        }
        else
        {
            rank_info.m_text = "";
        }

        int numLaps = race_manager->getNumLaps();

        if(kart_info.m_race_lap>=numLaps)
        {  // kart is finished, display in green
            rank_info.m_color.setGreen(0);
            rank_info.m_color.setBlue(0);
        }
        else if(kart_info.m_race_lap>=0 && numLaps>1)
        {
            int col = (int)(255*(1.0f-(float)kart_info.m_race_lap
                                    /((float)numLaps-1.0f)        ));
            rank_info.m_color.setBlue(col);
            rank_info.m_color.setGreen(col);
        }
    }   // next kart


}   // getKartsDisplayInfo

//-----------------------------------------------------------------------------
/** Estimate the arrival time of any karts that haven't arrived yet by using
 *  their average speed up to now and the distance still to race. This
 *  approach guarantees that the order of the karts won't change anymore
 *  (karts ahead will have covered more distance and have therefore a higher
 *  average speed and therefore finish the race earlier than karts further
 *  behind), so the position doesn't have to be updated to get the correct
 *  scoring.
 *  As so often the devil is in the details: a kart that hasn't crossed the
 *  starting line has a negative distance (when it is crossing the start line
 *  its distance becomes 0), which can result in a negative average speed
 *  (and therefore incorrect estimates). This is partly taken care of by
 *  adding m_distance_increase to the distance covered by a kart. The value
 *  of m_distance_increase is a bit more than the distance the last kart
 *  has from the start line at start time. This guarantees that the distance
 *  used in computing the average speed is positive in most cases. Only
 *  exception is if a kart is driving backwards on purpose. While this
 *  shouldn't happen (the AI doesn't do it, and if it's a player the game
 *  won't finish so the time estimation won't be called and so the incorrect
 *  result won't be displayed), this is still taken care of: if the average
 *  speed is negative, the estimated arrival time of the kart is set to
 *  99:00 plus kart position. This means that even in this case all karts
 *  will have a different arrival time.
 *  \pre The position of the karts are set according to the distance they
 *       have covered.
 *  \param kart The kart for which to estimate the finishing times.
 */
float LinearWorld::estimateFinishTimeForKart(AbstractKart* kart)
{
    const KartInfo &kart_info = m_kart_info[kart->getWorldKartId()];

    float full_distance = race_manager->getNumLaps()
                        * m_track->getTrackLength();

    if(full_distance == 0)
        full_distance = 1.0f;   // For 0 lap races avoid warning below

#ifdef DEBUG
    if(kart_info.m_overall_distance > full_distance)
    {
        Log::debug("[LinearWorld]", "Full distance < distance covered for kart '%s':",
               kart->getIdent().c_str());
        Log::debug("[LinearWorld]", "%f < %f", full_distance, kart_info.m_overall_distance);
    }
#endif
    // Avoid potential problems (floating point issues, coding bug?) if a
    // kart has driven more than the full distance, but not finished:
    // Return the current time plus initial position to spread arrival
    // times a bit. This code should generally not be used at all, it's
    // just here to avoid invalid finishing times.
    if(kart_info.m_overall_distance > full_distance)
        return getTime() + kart->getInitialPosition();

    // Finish time is the time needed for the whole race with
    // the computed average speed computed. The distance is always positive
    // due to the way m_distance_increase was computed, so average speed
    // is also always positive.
    float average_speed = getTime()==0
                        ? 1.0f
                        : (m_distance_increase + kart_info.m_overall_distance)
                          / getTime();

    // Avoid NAN or invalid results when average_speed is very low
    // or negative (which can happen if a kart drives backwards and
    // m_overall distance becomes smaller than -m_distance_increase).
    // In this case set the time to 99 minutes, offset by kart
    // position (to spread arrival times for all karts that arrive
    // even later). This works for up to 60 karts (otherwise the
    // time displayed would become too long: 100:xx:yy).
    if(average_speed<0.01f)
        return 99*60.0f + kart->getPosition();

    float est_time = getTime() + (full_distance - kart_info.m_overall_distance)
                                 / average_speed;

    // Avoid times > 99:00 - in this case use kart position to spread
    // arrival time so that each kart has a unique value. The pre-condition
    // guarantees that this works correctly (higher position -> less distance
    // covered -> later arrival time).
    if(est_time>99*60.0f)
        return 99*60.0f + kart->getPosition();

    return est_time;
}   // estimateFinishTimeForKart

// ------------------------------------------------------------------------
/** Returns the number of rescue positions on a given track, which in
 *  linear races is just the number of driveline quads.
  */
unsigned int LinearWorld::getNumberOfRescuePositions() const
{
    return QuadGraph::get()->getNumNodes();
}   // getNumberOfRescuePositions

// ------------------------------------------------------------------------
unsigned int LinearWorld::getRescuePositionIndex(AbstractKart *kart)
{
    KartInfo& info = m_kart_info[kart->getWorldKartId()];

    info.getTrackSector()->rescue();
    // Setting XYZ for the kart is important since otherwise the kart
    // will not detect the right material again when doing the next
    // raycast to detect where it is driving on (--> potential rescue loop)
    return info.getTrackSector()->getCurrentGraphNode();
}   // getRescuePositionIndex

// ------------------------------------------------------------------------
btTransform LinearWorld::getRescueTransform(unsigned int index) const
{
    const Vec3 &xyz = QuadGraph::get()->getQuadOfNode(index).getCenter();
    btTransform pos;
    pos.setOrigin(xyz);
    pos.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f),
                    m_track->getAngle(index)));
    return pos;
}   // getRescueTransform

//-----------------------------------------------------------------------------
/** Find the position (rank) of every kart. ATM it uses a stable O(n^2)
 *  algorithm by counting for each kart how many other karts are ahead of
 *  it.
 */
void LinearWorld::updateRacePosition()
{
    // Mostly for debugging:
    beginSetKartPositions();
    const unsigned int kart_amount = (unsigned int) m_karts.size();

#ifdef DEBUG
    bool rank_changed = false;
#endif

    // NOTE: if you do any changes to this loop, the next loop (see
    // DEBUG_KART_RANK below) needs to have the same changes applied
    // so that debug output is still correct!!!!!!!!!!!
    for (unsigned int i=0; i<kart_amount; i++)
    {
        AbstractKart* kart = m_karts[i];
        // Karts that are either eliminated or have finished the
        // race already have their (final) position assigned. If
        // these karts would get their rank updated, it could happen
        // that a kart that finished first will be overtaken after
        // crossing the finishing line and become second!
        if(kart->isEliminated() || kart->hasFinishedRace())
        {
            // This is only necessary to support debugging inconsistencies
            // in kart position parameters.
            setKartPosition(i, kart->getPosition());
            continue;
        }
        KartInfo& kart_info = m_kart_info[i];

        int p = 1 ;

        const unsigned int my_id = kart->getWorldKartId();
        const float my_distance  = m_kart_info[my_id].m_overall_distance;

        // Count karts ahead of the current kart, i.e. kart that are
        // already finished or have covered a larger overall distance.
        for (unsigned int j = 0 ; j < kart_amount ; j++)
        {
            // don't compare a kart with itself and ignore eliminated karts
            if(j == my_id || m_karts[j]->isEliminated())
                continue;

            // If the other kart has:
            // - finished the race (but this kart hasn't)
            // - or is ahead
            // - or has the same distance (very unlikely) but started earlier
            // it is ahead --> increase position
            if((!kart->hasFinishedRace() && m_karts[j]->hasFinishedRace()) ||
                m_kart_info[j].m_overall_distance > my_distance            ||
               (m_kart_info[j].m_overall_distance == my_distance &&
                m_karts[j]->getInitialPosition()<kart->getInitialPosition() ) )
            {
                p++;
            }

        } //next kart

#ifndef DEBUG
        setKartPosition(i, p);
#else
        rank_changed |= kart->getPosition()!=p;
        if (!setKartPosition(i,p))
        {
            Log::error("[LinearWorld]", "Same rank used twice!!");

            Log::debug("[LinearWorld]", "Info used to decide ranking :");
            for (unsigned int d=0; d<kart_amount; d++)
            {
                Log::debug("[LinearWorld]", "Kart %s has finished (%d), is at lap (%u),"
                            "is at distance (%u), is eliminated(%d)",
                            m_karts[d]->getIdent().c_str(),
                            m_karts[d]->hasFinishedRace(),
                            getLapForKart(d),
                            m_kart_info[d].m_overall_distance,
                            m_karts[d]->isEliminated());
            }

            Log::debug("[LinearWorld]", "Who has each ranking so far :");
            for (unsigned int d=0; d<i; d++)
            {
                Log::debug("[LinearWorld]", "%s has rank %d", m_karts[d]->getIdent().c_str(),
                            m_karts[d]->getPosition());
            }

            Log::debug("[LinearWorld]", "    --> And %s is being set at rank %d",
                        kart->getIdent().c_str(), p);
            history->Save();
            assert(false);
        }
#endif

        // Switch on faster music if not already done so, if the
        // first kart is doing its last lap, and if the estimated
        // remaining time is less than 30 seconds.
        if(!m_faster_music_active                                  &&
           kart_info.m_race_lap == race_manager->getNumLaps()-1    &&
           p==1                                                    &&
           useFastMusicNearEnd()                                   &&
           kart_info.m_estimated_finish > 0                        &&
           kart_info.m_estimated_finish - getTime() < 30.0f              )
        {
            music_manager->switchToFastMusic();
            m_faster_music_active=true;
        }
    }   // for i<kart_amount

    // Define this to get a detailled analyses each time a race position
    // changes.
#ifdef DEBUG
#undef DEBUG_KART_RANK
#ifdef DEBUG_KART_RANK
    if(rank_changed)
    {
        Log::debug("[LinearWorld]", "Counting laps at %u seconds.", getTime());
        for (unsigned int i=0; i<kart_amount; i++)
        {
            AbstractKart* kart = m_karts[i];
            Log::debug("[LinearWorld]", "counting karts ahead of %s (laps %u,"
                        " progress %u, finished %d, eliminated %d, initial position %u.",
                        kart->getIdent().c_str(),
                        m_kart_info[i].m_race_lap,
                        m_kart_info[i].m_overall_distance,
                        kart->hasFinishedRace(),
                        kart->isEliminated(),
                        kart->getInitialPosition());
            // Karts that are either eliminated or have finished the
            // race already have their (final) position assigned. If
            // these karts would get their rank updated, it could happen
            // that a kart that finished first will be overtaken after
            // crossing the finishing line and become second!
            if(kart->isEliminated() || kart->hasFinishedRace()) continue;
            KartInfo& kart_info = m_kart_info[i];
            int p = 1 ;
            const int my_id         = kart->getWorldKartId();
            const float my_distance = m_kart_info[my_id].m_overall_distance;

            for (unsigned int j = 0 ; j < kart_amount ; j++)
            {
                if(j == my_id) continue;
                if(m_karts[j]->isEliminated())
                {
                    Log::debug("[LinearWorld]", " %u: %s because it is eliminated.",
                                p, m_karts[j]->getIdent().c_str());
                    continue;
                }
                if(!kart->hasFinishedRace() && m_karts[j]->hasFinishedRace())
                {
                    p++;
                    Log::debug("[LinearWorld]", " %u: %s because it has finished the race.",
                                p, m_karts[j]->getIdent().c_str());
                    continue;
                }
                if(m_kart_info[j].m_overall_distance > my_distance)
                {
                    p++;
                    Log::debug("[LinearWorld]", " %u: %s because it is ahead %u.",
                                p, m_karts[j]->getIdent().c_str(),
                                m_kart_info[j].m_overall_distance);
                    continue;
                }
                if(m_kart_info[j].m_overall_distance == my_distance &&
                   m_karts[j]->getInitialPosition()<kart->getInitialPosition())
                {
                    p++;
                    Log::debug("[LinearWorld]"," %u: %s has same distance, but started ahead %d",
                                p, m_karts[j]->getIdent().c_str(),
                                m_karts[j]->getInitialPosition());
                }
            }   // next kart j
        }   // for i<kart_amount
        Log::debug("LinearWorld]", "-------------------------------------------");
    }   // if rank_changed
#endif
#endif

    endSetKartPositions();
}   // updateRacePosition

//-----------------------------------------------------------------------------
/** Checks if a kart is going in the wrong direction. This is done only for
 *  player karts to display a message to the player.
 *  \param i Kart id.
 */
void LinearWorld::checkForWrongDirection(unsigned int i, float dt)
{
    if (!m_karts[i]->getController()->isPlayerController()) 
        return;

    float wrongway_counter = m_karts[i]->getWrongwayCounter();
    
    const AbstractKart *kart=m_karts[i];
    // If the kart can go in more than one directions from the current track
    // don't do any reverse message handling, since it is likely that there
    // will be one direction in which it isn't going backwards anyway.
    int sector = m_kart_info[i].getTrackSector()->getCurrentGraphNode();
    
    if (QuadGraph::get()->getNumberOfSuccessors(sector) > 1)
        return;

    // check if the player is going in the wrong direction
    float angle_diff = kart->getHeading() - m_track->getAngle(sector);
    
    if (angle_diff > M_PI) 
        angle_diff -= 2*M_PI;
    else if (angle_diff < -M_PI) 
        angle_diff += 2*M_PI;
        
    // Display a warning message if the kart is going back way (unless
    // the kart has already finished the race).
    if ((angle_diff > DEGREE_TO_RAD * 120.0f ||
        angle_diff < -DEGREE_TO_RAD * 120.0f) &&
        kart->getVelocityLC().getY() > 0.0f &&
        !kart->hasFinishedRace())
    {
        wrongway_counter += dt;
        
        if (wrongway_counter > 2.0f)
            wrongway_counter = 2.0f;
    }
    else
    {
        wrongway_counter -= dt;

        if (wrongway_counter < 0)
            wrongway_counter = 0;
    }
    
    if (kart->getKartAnimation())
        wrongway_counter = 0;
    
    if (wrongway_counter > 1.0f)
    {
        m_race_gui->addMessage(_("WRONG WAY!"), kart,
                               /* time */ -1.0f,
                               video::SColor(255,255,255,255),
                               /*important*/ true,
                               /*big font*/  true);
    }  // if angle is too big
    
    m_karts[i]->setWrongwayCounter(wrongway_counter);
}   // checkForWrongDirection

//-----------------------------------------------------------------------------
