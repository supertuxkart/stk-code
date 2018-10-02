//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "modes/world_with_rank.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "karts/kart_properties.hpp"
#include "race/history.hpp"
#include "tracks/graph.hpp"
#include "tracks/track.hpp"
#include "tracks/track_sector.hpp"
#include "utils/log.hpp"

#include <iostream>

//-----------------------------------------------------------------------------
WorldWithRank::~WorldWithRank()
{
    for (unsigned int i = 0; i < m_kart_track_sector.size(); i++)
    {
        delete m_kart_track_sector[i];
    }
    m_kart_track_sector.clear();
}   // ~WorldWithRank

//-----------------------------------------------------------------------------
void WorldWithRank::init()
{
    World::init();

    m_display_rank = true;

    m_position_index.resize(m_karts.size());
#ifdef DEBUG
    m_position_used.resize(m_karts.size());
    m_position_setting_initialised = false;
#endif
    stk_config->getAllScores(&m_score_for_position, getNumKarts());

    Track *track = Track::getCurrentTrack();
    // Don't init track sector if navmesh is not found in arena
    if ((track->isArena() || track->isSoccer()) && !track->hasNavMesh())
        return;

    for (unsigned int i = 0; i < m_karts.size(); i++)
        m_kart_track_sector.push_back(new TrackSector());

}   // init

//-----------------------------------------------------------------------------
void WorldWithRank::reset(bool restart)
{
    World::reset(restart);
    for (unsigned int i = 0; i < m_kart_track_sector.size(); i++)
    {
        getTrackSector(i)->reset();
        getTrackSector(i)->update(m_karts[i]->getXYZ());
    }
}   // reset

//-----------------------------------------------------------------------------
/** Returns the kart with a given position.
 *  \param p The position of the kart, 1<=p<=num_karts).
 */
AbstractKart* WorldWithRank::getKartAtPosition(unsigned int p) const
{
    if(p<1 || p>m_position_index.size())
        return NULL;

    return m_karts[m_position_index[p-1]].get();
}   // getKartAtPosition

//-----------------------------------------------------------------------------
/** This function must be called before starting to set all kart positions
 *  again. It's mainly used to add some debug support, i.e. detect if the
 *  same position is set in different karts.
 */
void WorldWithRank::beginSetKartPositions()
{
#ifdef DEBUG
    assert(!m_position_setting_initialised);
    m_position_setting_initialised = true;

    for(unsigned int i=0; i<m_position_used.size(); i++)
        m_position_used[i] = false;
#endif
}   // beginSetKartPositions

//-----------------------------------------------------------------------------
/** Sets the position of a kart. This will be saved in this object to allow
 *  quick lookup of which kart is on a given position, but also in the
 *  kart objects.
 *  \param kart_id The index of the kart to set the position for.
 *  \param position The position of the kart (1<=position<=num karts).
 *  \return false if this position was already set, i.e. an inconsistency in
 *          kart positions has occurred. This is used in debug mode only to
 *          allow the calling function to print debug information.
 */
bool WorldWithRank::setKartPosition(unsigned int kart_id,
                                    unsigned int position)
{
    m_position_index[position-1] = kart_id;
    m_karts[kart_id]->setPosition(position);
#ifdef DEBUG
    assert(m_position_setting_initialised);
    if(m_position_used[position-1])
    {
        Log::error("[WorldWithRank]", "== TWO KARTS ARE BEING GIVEN THE SAME POSITION!! ==");
        for (unsigned int j=0; j < m_position_index.size(); j++)
        {
            if (!m_position_used[j])
            {
                Log::warn("WorldWithRank]", "No kart is yet set at position %u", j+1);
            }
            else
            {
                Log::warn("WorldWithRank]", "Kart %u is at position %u",
                            m_position_index[j], j);
            }
        }
        Log::warn("WorldWithRank]", "Kart %u is being given position %u,"
                    "but this position is already taken",
                    kart_id, position);
        return false;
    }
    m_position_used[position-1] = true;
#endif
    return true;
}   // setKartPosition

//-----------------------------------------------------------------------------
/** Called once the last position was set. Note that we should not test
 *  if all positions were set, since e.g. for eliminated and finished karts
 *  the position won't be set anymore.
 */
void WorldWithRank::endSetKartPositions()
{
#ifdef DEBUG
    assert(m_position_setting_initialised);
    m_position_setting_initialised = false;
#endif
}   // endSetKartPositions

//-----------------------------------------------------------------------------
/** Determines the rescue position for a kart. The rescue position is the
 *  start position which is has the biggest accumulated distance to all other
 *  karts, and which has no other kart very close. The latter avoids dropping
 *  a kart on top of another kart. This is the method used 
 *  \param kart The kart that is going to be rescued.
 *  \returns The index of the start position to which the rescued kart
 *           should be moved to.
 */

unsigned int WorldWithRank::getRescuePositionIndex(AbstractKart *kart)
{
    const int start_spots_amount =
                         Track::getCurrentTrack()->getNumberOfStartPositions();
    assert(start_spots_amount > 0);

    float largest_accumulated_distance_found = -1;
    int   furthest_id_found                  = -1;

    for(int n=0; n<start_spots_amount; n++)
    {
        const btTransform &s = getStartTransform(n);
        const Vec3 &v=s.getOrigin();
        float accumulated_distance = .0f;
        bool spawn_point_clear = true;

        for(unsigned int k=0; k<getCurrentNumKarts(); k++)
        {
            if(kart->getWorldKartId()==k) continue;
            float abs_distance2 = (getKart(k)->getXYZ()-v).length2();
            const float CLEAR_SPAWN_RANGE2 = 5*5;
            if( abs_distance2 < CLEAR_SPAWN_RANGE2)
            {
                spawn_point_clear = false;
                break;
            }
            accumulated_distance += sqrt(abs_distance2);
        }

        if(accumulated_distance > largest_accumulated_distance_found &&
            spawn_point_clear)
        {
            furthest_id_found = n;
            largest_accumulated_distance_found = accumulated_distance;
        }
    }

    assert(furthest_id_found != -1);
    return furthest_id_found;
}   // getRescuePositionIndex

//-----------------------------------------------------------------------------
/** Returns the number of points for a kart at a specified position.
 *  \param p Position (starting with 1).
 */
int WorldWithRank::getScoreForPosition(int p)
{
    assert(p-1 >= 0);
    assert(p - 1 <(int) m_score_for_position.size());
    return m_score_for_position[p - 1];
}   // getScoreForPosition

//-----------------------------------------------------------------------------
/** Returns true if the kart is on a valid graph quad.
 *  \param kart_index  Index of the kart.
 */
bool WorldWithRank::isOnRoad(unsigned int kart_index) const
{
    return getTrackSector(kart_index)->isOnRoad();
}   // isOnRoad

//-----------------------------------------------------------------------------
/** Gets the sector a kart is on. This function returns UNKNOWN_SECTOR if the
 *  kart_id is larger than the current kart sector. This is necessary in the
 *  case that a collision with the track happens during resetAllKarts: at this
 *  time m_kart_track_sector is not initialised (and has size 0), so it would
 *  trigger this assert. While this normally does not happen, it is useful for
 *  track designers that STK does not crash.
 *  \param kart Kart for which to return the sector.
 */
int WorldWithRank::getSectorForKart(const AbstractKart *kart) const
{
    if (kart->getWorldKartId() >= m_kart_track_sector.size())
        return Graph::UNKNOWN_SECTOR;
    return getTrackSector(kart->getWorldKartId())->getCurrentGraphNode();
}   // getSectorForKart

//-----------------------------------------------------------------------------
/** Localize each kart on the graph using its center xyz.
 */
void WorldWithRank::updateSectorForKarts()
{
    if (isRaceOver()) return;

    const unsigned int n = getNumKarts();
    assert(n == m_kart_track_sector.size());
    for (unsigned int i = 0; i < n; i++)
    {
        SpareTireAI* sta =
            dynamic_cast<SpareTireAI*>(m_karts[i]->getController());
        if (!m_karts[i]->isEliminated() || (sta && sta->isMoving()))
            getTrackSector(i)->update(m_karts[i]->getXYZ());
    }
}   // updateSectorForKarts
