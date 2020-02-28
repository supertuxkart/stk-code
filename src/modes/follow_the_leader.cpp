//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#include "modes/follow_the_leader.hpp"

#include "audio/music_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "items/powerup_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ISceneManager.h>

//-----------------------------------------------------------------------------
FollowTheLeaderRace::FollowTheLeaderRace() : LinearWorld()
{
    // We have to make sure that no kart finished the set number of laps
    // in a FTL race (since otherwise its distance will not be computed
    // correctly, and as a result e.g. a leader might suddenly fall back
    // after crossing the start line
    RaceManager::get()->setNumLaps(99999);

    m_leader_intervals = stk_config->m_leader_intervals;
    for(unsigned int i=0; i<m_leader_intervals.size(); i++)
        m_leader_intervals[i] +=
            stk_config->m_leader_time_per_kart*RaceManager::get()->getNumberOfKarts();
    m_use_highscores   = false;  // disable high scores
    setClockMode(WorldStatus::CLOCK_COUNTDOWN, m_leader_intervals[0]);
    m_is_over_delay = 5.0f;
}

//-----------------------------------------------------------------------------
/** Called immediately after the constructor. Here functions that use
 *  World::getWorld() as well as overridden functions.
 */
void FollowTheLeaderRace::init()
{
    m_last_eliminated_time = 0;
    LinearWorld::init();
    // WorldWithRank determines the score based on getNumKarts(), but since
    // we ignore the leader, the points need to be based on number of karts -1
    stk_config->getAllScores(&m_score_for_position, getNumKarts() - 1);
    getKart(0)->setOnScreenText(_("Leader"));
    getKart(0)->setBoostAI(true);
}    // init

#if 0
#pragma mark -
#pragma mark clock events
#endif
//-----------------------------------------------------------------------------
FollowTheLeaderRace::~FollowTheLeaderRace()
{
}

//-----------------------------------------------------------------------------
/** Called just before a race is started.
 */
void FollowTheLeaderRace::reset(bool restart)
{
    LinearWorld::reset(restart);
    m_last_eliminated_time = 0.0f;
    m_leader_intervals.clear();
    m_leader_intervals    = stk_config->m_leader_intervals;
    for(unsigned int i=0; i<m_leader_intervals.size(); i++)
        m_leader_intervals[i] +=
            stk_config->m_leader_time_per_kart*RaceManager::get()->getNumberOfKarts();
    WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
                              m_leader_intervals[0]);
    
    m_is_over_delay = 2.0f;
}   // reset

//-----------------------------------------------------------------------------
/** Returns the number of points for a kart at a specified position.
 *  \param p Position (starting with 1).
 */
int FollowTheLeaderRace::getScoreForPosition(int p)
{
    // Kart 0 (the leader) does not get any points
    if (p == 1) return 0;

    assert(p-2 >= 0);
    assert(p - 2 <(int) m_score_for_position.size());
    return m_score_for_position[p - 2];
}   // getScoreForPosition

//-----------------------------------------------------------------------------
const btTransform &FollowTheLeaderRace::getStartTransform(int index)
{
    if (index == 0)   // Leader start position
        return Track::getCurrentTrack()->getStartTransform(index);

    // Otherwise the karts will start at the rear starting positions
    int start_index = stk_config->m_max_karts
                    - RaceManager::get()->getNumberOfKarts() + index;
    return Track::getCurrentTrack()->getStartTransform(start_index);
}   // getStartTransform

//-----------------------------------------------------------------------------
/** Called when a kart must be eliminated.
 */
void FollowTheLeaderRace::countdownReachedZero()
{
    m_last_eliminated_time += m_leader_intervals[0];
    if(m_leader_intervals.size()>1)
        m_leader_intervals.erase(m_leader_intervals.begin());
    WorldStatus::setTime(m_leader_intervals[0]);

    // If the leader kart is not the first kart, remove the first
    // kart, otherwise remove the last kart.
    int position_to_remove = m_karts[0]->getPosition()==1
                           ? getCurrentNumKarts() : 1;
    AbstractKart *kart = getKartAtPosition(position_to_remove);
    if(!kart || kart->isEliminated())
    {
        Log::error("[FTL]", "Problem with removing leader: position %d not found",
                position_to_remove);
        for(unsigned int i=0; i<m_karts.size(); i++)
        {
            Log::error("[FTL]", "kart %u: eliminated %d position %d",
                    i, m_karts[i]->isEliminated(), m_karts[i]->getPosition());
        }   // for i
    }  //
    else
    {
        if(UserConfigParams::m_ftl_debug)
        {
            Log::debug("[FTL", "Eliminiating kart '%s' at position %d.",
                kart->getIdent().c_str(), position_to_remove);
        }
        eliminateKart(kart->getWorldKartId());

        // In case that the kart on position 1 was removed, we have
        // to set the correct position (which equals the remaining
        // number of karts) for the removed kart, as well as recompute
        // the position for all other karts
        if(position_to_remove==1)
        {
            // We have to add 1 to the number of karts to get the correct
            // position, since the eliminated kart was already removed
            // from the value returned by getCurrentNumKarts (and we have
            // to remove the kart before we can call updateRacePosition).
            // Note that we can not call WorldWithRank::setKartPosition
            // here, since it would not properly support debugging kart
            // ranks (since this kart would get its position set again
            // in updateRacePosition). We only set the rank of the eliminated
            // kart, and updateRacePosition will then call setKartPosition
            // for the now eliminated kart.
            kart->setPosition(getCurrentNumKarts()+1);
            updateRacePosition();
        }
        // Time doesn't make any sense in FTL (and it is not displayed)
        kart->finishedRace(m_last_eliminated_time);

        // Move any camera for this kart to the leader, facing backwards,
        // so that the eliminated player has something to watch.
        if (RaceManager::get()->getNumPlayers() > 1)
        {
            for(unsigned int i=0; i<Camera::getNumCameras(); i++)
            {
                Camera *camera = Camera::getCamera(i);
                if(camera->getKart()==kart)
                {
                    camera->setMode(Camera::CM_LEADER_MODE);
                    camera->setKart(getKart(0));
                }
            }   // for i<number of cameras
        }
    }   // if kart to eliminate exists

    // almost over, use fast music
    if(getCurrentNumKarts()==3)
    {
        music_manager->switchToFastMusic();
    }

}   // countdownReachedZero

//-----------------------------------------------------------------------------
/** The follow the leader race is over if there is only one kart left (plus
 *  the leader), or if all (human) players have been eliminated.
 */
bool FollowTheLeaderRace::isRaceOver()
{
    bool is_over = (getCurrentNumKarts()==2 || getCurrentNumPlayers()==0);
    if (is_over)
    {
        if (m_is_over_delay < 0.0f)
        {
            return true;
        }
        else
        {
            m_is_over_delay -= GUIEngine::getLatestDt();
            return false;
        }
    }
    else
    {
        return false;
    }
}   // isRaceOver

//-----------------------------------------------------------------------------
/** If the leader kart is hit, increase the delay to the next elimination */
void FollowTheLeaderRace::leaderHit()
{
    int countdown = getTimeTicks();
    countdown += stk_config->time2Ticks(5.0f);
    setTicks(countdown);
} // leaderHit

//-----------------------------------------------------------------------------
/** Called at the end of a race. Updates highscores, pauses the game, and
 *  informs the unlock manager about the finished race. This function must
 *  be called after all other stats were updated from the different game
 *  modes.
 */
void FollowTheLeaderRace::terminateRace()
{
    int pos_leader = m_karts[0]->getPosition();

    // Handle special FTL situations: the leader is kart number 3 when
    // the last kart gets eliminated. In this case kart on position 1
    // is eliminated, and the kart formerly on position 2 is on
    // position 1, the leader now position 2, but the point distribution
    // depends on the 'first' (non-leader) kart to be on position 2.
    // That situation can also occur during the delay after eliminating
    // the last kart before the race result is shown (when the leader
    // is overtaken during that time). To avoid this problem, adjust the
    // position of any kart that is ahead of the leader.
    beginSetKartPositions();
    for (unsigned int i = 0; i < getNumKarts(); i++)
    {
        if (!m_karts[i]->hasFinishedRace() && !m_karts[i]->isEliminated() &&
            m_karts[i]->getPosition() < pos_leader)
        {
            setKartPosition(i, m_karts[i]->getPosition() + 1);
        }
    }   // i < number of karts
    setKartPosition(/*kart id*/0, /*position*/1);
    endSetKartPositions();

    // Mark all still racing karts to be finished.
    for (int i = (int)m_karts.size(); i>0; i--)
    {
        AbstractKart *kart = getKartAtPosition(i);
        if (kart->isEliminated() || kart->hasFinishedRace())
            continue;
        m_last_eliminated_time += m_leader_intervals[0];
        if (m_leader_intervals.size() > 1)
            m_leader_intervals.erase(m_leader_intervals.begin());
        kart->finishedRace(m_last_eliminated_time);
    }


    World::terminateRace();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this kind of race.
 */
const std::string& FollowTheLeaderRace::getIdent() const
{
    return IDENT_FTL;
}   // getIdent

//-----------------------------------------------------------------------------
/** Sets the title for all karts that is displayed in the icon list. In
 *  this mode the title for the first kart is set to 'leader'.
 */
void FollowTheLeaderRace::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    LinearWorld::getKartsDisplayInfo(info);
    (*info)[0].special_title = _("Leader");
}   // getKartsDisplayInfo
