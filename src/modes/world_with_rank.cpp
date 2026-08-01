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

#include "graphics/material.hpp"
#include "karts/kart.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "karts/kart_properties.hpp"
#include "physics/triangle_mesh.hpp"
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
    // 3SB now uses WorldWithRank too, so we have to account for spare tire karts
    int sta = RaceManager::get()->getNumSpareTireKarts();
    int kart_num = RaceManager::get()->getNumberOfKarts() - sta;
    stk_config->getAllScores(&m_score_for_position, kart_num);

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
Kart* WorldWithRank::getKartAtPosition(unsigned int p) const
{
    if(p<1 || p>m_position_index.size())
        return NULL;

    return m_karts[m_position_index[p-1]].get();
}   // getKartAtPosition

//-----------------------------------------------------------------------------
std::pair<int, video::SColor> WorldWithRank::getSpeedometerDigit(const Kart *kart) const
{
    return std::make_pair(kart->getPosition(), video::SColor(255, 255, 255, 255));
}    // getSpeedometerDigit

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
 *  a kart on top of another kart.
 * 
 *  IMPORTANT: This is the method used in battle mode. Linear mode use another
 *  rescue logic.
 * 
 *  \param kart The kart that is going to be rescued.
 *  \returns The index of the start position to which the rescued kart
 *           should be moved to.
 */

unsigned int WorldWithRank::getRescuePositionIndex(Kart *kart)
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

        // If another kart is too close to this rescue position, skip it
        if(!isRescuePointClear(v, kart, getRescueTransform(n)))
            continue;

        for(unsigned int k=0; k<getCurrentNumKarts(); k++)
        {
            if(kart->getWorldKartId()==k) continue;
            float abs_distance2 = (getKart(k)->getXYZ()-v).length2();
            accumulated_distance += sqrt(abs_distance2);
        }

        if(accumulated_distance > largest_accumulated_distance_found)
        {
            furthest_id_found = n;
            largest_accumulated_distance_found = accumulated_distance;
        }
    }

    assert(furthest_id_found != -1);
    return furthest_id_found;
}   // getRescuePositionIndex

//-----------------------------------------------------------------------------
/** Check if the considered rescue position is adequate:
 * - Not too close to other karts (to avoid rescuing a kart on top of another)
 * - Actually over some ground. The ground checks may produce false positives
 * or negatives if the ground has small holes or tiny floating ground areas.
 */
bool WorldWithRank::isRescuePointClear(Vec3 rescue_pos, Kart *kart, btTransform rescue_trans)
{
    // We start our raycasts slightly higher than the rescue points,
    // to avoid situations where we cast a ray from just below the road
    // and get a misleading result.
    const btMatrix3x3& basis = rescue_trans.getBasis();
    Vec3 up_vector(basis[0].y(), basis[1].y(), basis[2].y());
    up_vector = up_vector * 0.3f;

    // If there is no ground close enough below the proposed rescue position,
    // it is an invalid rescue position.
    if (!isPointAboveGround(rescue_pos + up_vector, rescue_trans))
        return false;

    // We build a forward vector based on the rescue transformation.
    Vec3 forward_vector(basis[0].z(), basis[1].z(), basis[2].z());

    // We also check that there is ground in front of the rescue position,
    // both close by and a bit farther, to avoid the kart falling
    // as soon as it starts driving.
    Vec3 forward_point = rescue_pos + 0.5f * forward_vector + up_vector;
    if (!isPointAboveGround(forward_point, rescue_trans)                          ||
        !isPointAboveGround(forward_point + 0.75f * forward_vector, rescue_trans) ||
        !isPointAboveGround(forward_point + 1.5f * forward_vector, rescue_trans)  ||
        !isPointAboveGround(forward_point + 2.25f * forward_vector, rescue_trans))
        return false;

    // We also check that there is ground slightly behind the rescue position,
    // to ensure the back wheels can also rest on solid ground.
    Vec3 back_point = rescue_pos - 0.5f * forward_vector + up_vector;
    if (!isPointAboveGround(back_point, rescue_trans))
        return false;

    // Now we check that other karts aren't too close, to avoid rescuing
    // on top of another kart.
    const float CLEAR_SPAWN_RANGE2 = 9; // 3^2 = 9
    for(unsigned int k=0; k<getCurrentNumKarts(); k++)
    {
        if(kart->getWorldKartId()==k) continue;
        float abs_distance2 = (getKart(k)->getXYZ() - rescue_pos).length2();
        if( abs_distance2 < CLEAR_SPAWN_RANGE2)
            return false;
    }

    return true;
}   // checkRescuePointClear
//-----------------------------------------------------------------------------
/** Check if the given position is sufficiently close to ground.
 */
bool WorldWithRank::isPointAboveGround(Vec3 pos_to_check, btTransform rescue_trans)
{
    const Material* material_hit;
    Vec3 normal;
    Vec3 to = pos_to_check + rescue_trans.getBasis() * Vec3(0, -100, 0);
    Vec3 hit_point;
    Track::getCurrentTrack()->getTriangleMesh().castRay(pos_to_check, to, &hit_point,
                                                        &material_hit, &normal);

    // If there is no ground close enough below the proposed rescue position,
    // it is an invalid rescue position.
    if (!material_hit || material_hit->isDriveReset() ||
        (hit_point - pos_to_check).length2() > 16)
        return false;

    return true;
}   // isPointAboveGround

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
int WorldWithRank::getSectorForKart(const Kart *kart) const
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
