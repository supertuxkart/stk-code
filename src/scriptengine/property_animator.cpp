//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015  SuperTuxKart Team
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

#include "scriptengine/property_animator.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_presentation.hpp"
#include "utils/log.hpp"

AnimatedProperty::AnimatedProperty(AnimatablePropery property, 
    int values_count, double* values_from, double* values_to,
    double duration, void* data)
{
    m_property = property;
    m_remaining_time = m_total_time = duration;
    m_data = data;
    m_values_count = values_count;
    m_values_from = values_from;
    m_values_to = values_to;
    m_new_values = new double[values_count];
}

// ----------------------------------------------------------------------------

bool AnimatedProperty::update(double dt)
{
    bool done = false;
    m_remaining_time -= dt;
    if (m_remaining_time < 0)
    {
        m_remaining_time = 0;
        done = true;
    }

    double ratio = 1.0 - m_remaining_time / m_total_time;

    for (int i = 0; i < m_values_count; i++)
    {
        m_new_values[i] = m_values_from[i] + (m_values_to[i] - m_values_from[i]) * ratio;
    }

    switch (m_property)
    {
        case AnimatablePropery::AP_LIGHT_ENERGY:
        {
            TrackObjectPresentationLight* light = (TrackObjectPresentationLight*)m_data;
            light->setEnergy((float)m_new_values[0]);
            break;
        }

        case AnimatablePropery::FOG_RANGE:
        {
            Track* track = (Track*)m_data;
            track->setFogStart((float)m_new_values[0]);
            track->setFogEnd((float)m_new_values[1]);
            break;
        }

        case AnimatablePropery::FOG_MAX:
        {
            Track* track = (Track*)m_data;
            track->setFogMax((float)m_new_values[0]);
            break;
        }

        case AnimatablePropery::FOG_COLOR:
        {
            Track* track = (Track*)m_data;
            video::SColor color(255, (int)m_new_values[0], (int)m_new_values[1], (int)m_new_values[2]);
            track->setFogColor(color);
            break;
        }

        default:
            Log::error("PropertyAnimator", "Unknown properry %i", (int)m_property);
            break;
    }

    return done;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

PropertyAnimator* PropertyAnimator::s_instance = NULL;

PropertyAnimator* PropertyAnimator::get()
{
    if (s_instance == NULL)
        s_instance = new PropertyAnimator();

    return s_instance;
}

// ----------------------------------------------------------------------------

void PropertyAnimator::add(AnimatedProperty* prop)
{
    m_properties.push_back(prop);
}

// ----------------------------------------------------------------------------

void PropertyAnimator::update(double dt)
{
    for (int i = m_properties.size() - 1; i >= 0; i--)
    {
        bool done = m_properties[i].update(dt);
        if (done)
            m_properties.erase(i);
    }
}

// ----------------------------------------------------------------------------

void PropertyAnimator::clear()
{
    m_properties.clearAndDeleteAll();
}
