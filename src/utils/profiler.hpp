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

#ifndef PROFILER_HPP
#define PROFILER_HPP

#include "utils/synchronised.hpp"

#include <assert.h>
#include <atomic>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <stack>
#include <streambuf>
#include <string>
#include <vector>

#include <vector2d.h>
#include <SColor.h>
using namespace irr;

enum QueryPerf
{
    Q_SHADOWS_CASCADE0,
    Q_SHADOWS_CASCADE1,
    Q_SHADOWS_CASCADE2,
    Q_SHADOWS_CASCADE3,
    Q_SOLID_PASS,
    Q_ENVMAP,
    Q_SUN,
    Q_POINTLIGHTS,
    Q_SSAO,
    Q_LIGHTSCATTER,
    Q_GLOW,
    Q_COMBINE_DIFFUSE_COLOR,
    Q_SKYBOX,
    Q_TRANSPARENT,
    Q_PARTICLES,
    Q_DOF,
    Q_GODRAYS,
    Q_BLOOM,
    Q_TONEMAP,
    Q_MOTIONBLUR,
    Q_LIGHTNING,
    Q_MLAA,
    Q_GUI,
    Q_LAST
};

class Profiler;
extern Profiler profiler;

double getTimeMilliseconds();

#define ENABLE_PROFILER

#ifdef ENABLE_PROFILER
    #define PROFILER_PUSH_CPU_MARKER(name, r, g, b) \
        profiler.pushCPUMarker(name, video::SColor(0xFF, r, g, b))

    #define PROFILER_POP_CPU_MARKER()  \
        profiler.popCPUMarker()

    #define PROFILER_SYNC_FRAME()   \
        profiler.synchronizeFrame()

    #define PROFILER_DRAW() \
        profiler.draw()
#else
    #define PROFILER_PUSH_CPU_MARKER(name, r, g, b)
    #define PROFILER_POP_CPU_MARKER()
    #define PROFILER_SYNC_FRAME()
    #define PROFILER_DRAW()
#endif

using namespace irr;

// ============================================================================
/** \brief class that allows run-time graphical profiling through the use 
 *  of markers.
 * \ingroup utils
 */
class Profiler
{
private:
    // ------------------------------------------------------------------------
    class Marker
    {
    private:
        /** An event that is started (pushed) stores the start time in this
         *  variable. */
        double  m_start;

        /** Duration of the event in this frame (accumulated if this event
         *  should be recorded more than once). */

        double  m_duration;
        /** Distance of marker from root (for nested events), used to
         *  adjust vertical height when drawing. */
        size_t  m_layer;
    public:
        // --------------------------------------------------------------------
        Marker() { m_start = 0; m_duration = 0; m_layer = 0; }

        // --------------------------------------------------------------------
        Marker(double start, size_t layer=0)
           : m_start(start), m_duration(0), m_layer(layer)
        {
        }
        // --------------------------------------------------------------------
        Marker(const Marker& ref)
            : m_start(ref.m_start), m_duration(ref.m_duration), 
              m_layer(ref.m_layer)
        {
        }
        // --------------------------------------------------------------------
        /** Returns the start time of this event marker. */
        double getStart() const { return m_start;  }
        // --------------------------------------------------------------------
        /** Returns the end time of this event marker. */
        double getEnd() const { return m_start+m_duration; }
        // --------------------------------------------------------------------
        /** Returns the duration of this event. */
        double getDuration() const { return m_duration;  }
        // --------------------------------------------------------------------
        size_t getLayer() const { return m_layer;  }
        // --------------------------------------------------------------------
        /** Called when an entry in the cyclic buffer is reused. Makes sure
         *  that time for a new event can be accumulated. */
        void clear() { m_duration = 0; }
        // --------------------------------------------------------------------
        /** Sets start time and layer for this event. */
        void setStart(double start, size_t layer = 0)
        {
            m_start = start; m_layer = layer;
        }   // setStart
        // --------------------------------------------------------------------
        /** Sets the end time of this event. */
        void setEnd(double end)
        {
            m_duration += (end - m_start);
        }   // setEnd

    };   // class Marker

    // ========================================================================
    /** The data for one event. It contains the events colours, all markers
     * for the buffer period and a stack to detect nesting of markers.
     */
    class EventData
    {
    private:
        /** Colour to use in the on-screen display */
        video::SColor m_colour;

        /** Vector of all buffered markers. */
        std::vector<Marker> m_all_markers;

    public:
        EventData() {}
        EventData(video::SColor colour, int max_size)
        {
            m_all_markers.resize(max_size);
            m_colour = colour;
        }   // EventData
        // --------------------------------------------------------------------
        /** Records the start of an event for a given frame. */
        void setStart(size_t frame, double start, int layer)
        {
            assert(frame < m_all_markers.capacity());
            m_all_markers[frame].setStart(start, layer);
        }   // setStart
        // --------------------------------------------------------------------
        /** Records the end of an event for a given frame. */
        void setEnd(size_t frame, double end)
        {
            assert(frame < m_all_markers.capacity());
            m_all_markers[frame].setEnd(end);
        }   // setEnd
        // --------------------------------------------------------------------
        const Marker& getMarker(int n) const { return m_all_markers[n]; }
        Marker& getMarker(int n) { return m_all_markers[n]; }
        // --------------------------------------------------------------------
        /** Returns the colour for this event. */
        video::SColor getColour() const { return m_colour;  }
        // --------------------------------------------------------------------
    };   // EventData

    // ========================================================================
    /** The mapping of event names to the corresponding EventData. */
    typedef std::map<std::string, EventData> AllEventData;
    // ========================================================================
    struct ThreadData
    {
        /** Stack of events to detect nesting. */
        std::vector< std::string > m_event_stack;

        /** This stores the event names in the order in which they occur.
        *  This means that 'outer' events occur here before any child
        *  events. This list is then used to determine the order in which the
        *  bar graphs are drawn, which results in the proper nesting of events.*/
        std::vector<std::string> m_ordered_headings;

        AllEventData m_all_event_data;
    };   // class ThreadData

    // ========================================================================

    /** Data structure containing all currently buffered markers. The index
     *  is the thread id. */
    std::vector< ThreadData> m_all_threads_data;

    /** Buffer for the GPU times (in ms). */
    std::vector<int> m_gpu_times;

    /** Counts the threads used. */
    std::atomic<int> m_threads_used;

    /** Index of the current frame in the buffer. */
    int m_current_frame;

    /** We don't need the bool, but easiest way to get a lock for the whole
     *  instance (since we need to avoid that a synch is done which changes
     *  the current frame while another threaded uses this variable, or
     *  while a new thread is added. */
    Synchronised<bool> m_lock;

    /** True if the circular buffer has wrapped around. */
    bool m_has_wrapped_around;

    /** The maximum number of frames to be buffered. Used to minimise
     *  reallocations. */
    int m_max_frames;

    /** Time of last sync. All start/end times are stored relative
     *  to this time. */
    double m_time_last_sync;

    /** Time between now and last sync, used to scale the GUI bar. */
    double m_time_between_sync;

    /** List of all event names. This list is sorted to make sure
     *  if the circular buffer is dumped more than once the order
     *  of events remains the same. */
    std::vector<std::string> m_all_event_names;

    // Handling freeze/unfreeze by clicking on the display
    enum FreezeState
    {
        UNFROZEN,
        WAITING_FOR_FREEZE,
        FROZEN,
        WAITING_FOR_UNFREEZE,
    };

    FreezeState     m_freeze_state;

private:
    int  getThreadID();
    void drawBackground();

public:
             Profiler();
    virtual ~Profiler();
    void     init();
    void     pushCPUMarker(const char* name="N/A",
                           const video::SColor& color=video::SColor());
    void     popCPUMarker();
    void     toggleStatus(); 
    void     synchronizeFrame();
    void     draw();
    void     onClick(const core::vector2di& mouse_pos);
    void     writeToFile();

    // ------------------------------------------------------------------------
    bool isFrozen() const { return m_freeze_state == FROZEN; }

};

#endif // PROFILER_HPP
