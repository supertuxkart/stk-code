//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#include "karts/ghost_kart.hpp"
#include "karts/kart_gfx.hpp"
#include "karts/kart_model.hpp"
#include "modes/world.hpp"

#include "LinearMath/btQuaternion.h"
#include "utils/log.hpp"

GhostKart::GhostKart(const std::string& ident, unsigned int world_kart_id,
                     int position)
          : Kart(ident, world_kart_id,
                 position, btTransform(btQuaternion(0, 0, 0, 1)),
                 PLAYER_DIFFICULTY_NORMAL)
{
    m_all_times.clear();
    m_all_transform.clear();
    m_all_physic_info.clear();
    m_all_replay_events.clear();
}   // GhostKart

// ----------------------------------------------------------------------------
void GhostKart::reset()
{
    m_node->setVisible(true);
    Kart::reset();
    m_current_transform = 0;
    // This will set the correct start position
    update(0);
}   // reset

// ----------------------------------------------------------------------------
void GhostKart::addReplayEvent(float time,
                               const btTransform &trans,
                               const ReplayBase::PhysicInfo &pi,
                               const ReplayBase::KartReplayEvent &kre)
{
    // FIXME: for now avoid that transforms for the same time are set
    // twice (to avoid division by zero in update). This should be
    // done when saving in replay
    if(m_all_times.size()>0 && m_all_times.back()==time)
        return;
    m_all_times.push_back(time);
    m_all_transform.push_back(trans);
    m_all_physic_info.push_back(pi);
    m_all_replay_events.push_back(kre);

    // Use first frame of replay to calculate default suspension
    if (m_all_physic_info.size() == 1)
    {
        float f = 0;
        for (int i = 0; i < 4; i++)
            f += m_all_physic_info[0].m_suspension_length[i];
        m_graphical_y_offset = -f / 4 + getKartModel()->getLowestPoint();
        m_kart_model->setDefaultSuspension();
    }

}   // addReplayEvent

// ----------------------------------------------------------------------------
/** Updates the current event of the ghost kart using interpolation
 *  \param dt Time step size.
 */
void GhostKart::update(float dt)
{
    float t = World::getWorld()->getTime();
    // Find (if necessary) the next index to use
    if (t != 0.0f)
    {
        while (m_current_transform + 1 < m_all_times.size() &&
               t >= m_all_times[m_current_transform+1])
        {
            m_current_transform++;
        }
    }

    if (m_current_transform + 1 >= m_all_times.size())
    {
        m_node->setVisible(false);
        return;
    }

    float nitro_frac = 0;
    if (m_all_replay_events[m_current_transform].m_on_nitro)
    {
        nitro_frac = fabsf(m_all_physic_info[m_current_transform].m_speed) /
            (m_kart_properties->getEngineMaxSpeed());

        if (nitro_frac > 1.0f)
            nitro_frac = 1.0f;
    }
    getKartGFX()->updateNitroGraphics(nitro_frac);

    if (m_all_replay_events[m_current_transform].m_on_zipper)
        showZipperFire();

    float f =(t - m_all_times[m_current_transform])
           / (  m_all_times[m_current_transform+1]
              - m_all_times[m_current_transform]  );
    setXYZ((1-f)*m_all_transform[m_current_transform  ].getOrigin()
           + f  *m_all_transform[m_current_transform+1].getOrigin() );
    const btQuaternion q = m_all_transform[m_current_transform].getRotation()
        .slerp(m_all_transform[m_current_transform+1].getRotation(), f);
    setRotation(q);

    Vec3 center_shift(0, 0, 0);
    center_shift.setY(m_graphical_y_offset);
    center_shift = getTrans().getBasis() * center_shift;

    Moveable::updateGraphics(dt, center_shift, btQuaternion(0, 0, 0, 1));
    getKartModel()->update(dt, dt*(m_all_physic_info[m_current_transform].m_speed),
        m_all_physic_info[m_current_transform].m_steer,
        m_all_physic_info[m_current_transform].m_speed,
        m_current_transform);

    Vec3 front(0, 0, getKartLength()*0.5f);
    m_xyz_front = getTrans()(front);
    getKartGFX()->update(dt);
}   // update
