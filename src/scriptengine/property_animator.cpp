#include "scriptengine/property_animator.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_presentation.hpp"
#include "utils/log.hpp"

AnimatedProperty::AnimatedProperty(AnimatablePropery property, double from, double to,
    double duration, void* data)
{
    m_property = property;
    m_value_from = from;
    m_value_to = to;
    m_remaining_time = m_total_time = duration;
    m_data = data;
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
    double new_value = m_value_from + (m_value_to - m_value_from) * ratio;

    switch (m_property)
    {
        case AnimatablePropery::AP_LIGHT_ENERGY:
        {
            TrackObjectPresentationLight* light = (TrackObjectPresentationLight*)m_data;
            light->setEnergy((float)new_value);
            break;
        }

        case AnimatablePropery::FOG_START:
        {
            Track* track = (Track*)m_data;
            track->setFogStart((float)new_value);
            break;
        }

        case AnimatablePropery::FOG_END:
        {
            Track* track = (Track*)m_data;
            track->setFogEnd((float)new_value);
            break;
        }

        case AnimatablePropery::FOG_MAX:
        {
            Track* track = (Track*)m_data;
            track->setFogMax((float)new_value);
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
