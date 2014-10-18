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

#include "profiler.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/2dutils.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "utils/vs.hpp"

#include <assert.h>
#include <stack>
#include <sstream>
#include <algorithm>
#include <fstream>

static const char* GPU_Phase[Q_LAST] =
{
    "Shadows Cascade 0",
    "Shadows Cascade 1",
    "Shadows Cascade 2",
    "Shadows Cascade 3",
    "Solid Pass 1",
    "RSM",
    "RH",
    "GI",
    "Env Map",
    "SunLight",
    "PointLights",
    "SSAO",
    "Solid Pass 2",
    "Transparent",
    "Particles",
    "Displacement",
    "Depth of Field",
    "Godrays",
    "Bloom",
    "Tonemap",
    "Motion Blur",
    "MLAA",
    "GUI",
};

Profiler profiler;

// Unit is in pencentage of the screen dimensions
#define MARGIN_X    0.02f    // left and right margin
#define MARGIN_Y    0.02f    // top margin
#define LINE_HEIGHT 0.030f   // height of a line representing a thread

#define MARKERS_NAMES_POS      core::rect<s32>(50,100,150,200)
#define GPU_MARKERS_NAMES_POS      core::rect<s32>(50,165,150,250)

#define TIME_DRAWN_MS 30.0f // the width of the profiler corresponds to TIME_DRAWN_MS milliseconds

// --- Begin portable precise timer ---
#ifdef WIN32
    #include <windows.h>

    double getTimeMilliseconds()
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        double perFreq = double(freq.QuadPart) / 1000.0;

        LARGE_INTEGER timer;
        QueryPerformanceCounter(&timer);
        return double(timer.QuadPart) / perFreq;
    }

#else
    #include <sys/time.h>
    double getTimeMilliseconds()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return double(tv.tv_sec * 1000) + (double(tv.tv_usec) / 1000.0);
    }
#endif
// --- End portable precise timer ---

//-----------------------------------------------------------------------------
Profiler::Profiler()
{
    m_thread_infos.resize(1);    // TODO: monothread now, should support multithreading
    m_write_id = 0;
    m_time_last_sync = getTimeMilliseconds();
    m_time_between_sync = 0.0;
    m_freeze_state = UNFROZEN;
    m_capture_report = false;
    m_first_capture_sweep = true;
    m_first_gpu_capture_sweep = true;
    m_capture_report_buffer = NULL;
}

//-----------------------------------------------------------------------------
Profiler::~Profiler()
{
}

//-----------------------------------------------------------------------------

void Profiler::setCaptureReport(bool captureReport)
{
    if (!m_capture_report && captureReport)
    {
        m_capture_report = true;
        m_first_capture_sweep = true;
        m_first_gpu_capture_sweep = true;
        // TODO: a 20 MB hardcoded buffer for now. That should amply suffice for
        // all reasonable purposes. But it's not too clean to hardcode
        m_capture_report_buffer = new StringBuffer(20 * 1024 * 1024);
        m_gpu_capture_report_buffer = new StringBuffer(20 * 1024 * 1024);
    }
    else if (m_capture_report && !captureReport)
    {
        // when disabling capture to file, flush captured data to a file
        {
            std::ofstream filewriter(file_manager->getUserConfigFile("profiling.csv").c_str(), std::ios::out | std::ios::binary);
            const char* str = m_capture_report_buffer->getRawBuffer();
            filewriter.write(str, strlen(str));
        }
        {
            std::ofstream filewriter(file_manager->getUserConfigFile("profiling_gpu.csv").c_str(), std::ios::out | std::ios::binary);
            const char* str = m_gpu_capture_report_buffer->getRawBuffer();
            filewriter.write(str, strlen(str));
        }

        m_capture_report = false;

        delete m_capture_report_buffer;
        m_capture_report_buffer = NULL;

        delete m_gpu_capture_report_buffer;
        m_gpu_capture_report_buffer = NULL;
    }
}

//-----------------------------------------------------------------------------
/// Push a new marker that starts now
void Profiler::pushCpuMarker(const char* name, const video::SColor& color)
{
    // Don't do anything when frozen
    if(m_freeze_state == FROZEN || m_freeze_state == WAITING_FOR_UNFREEZE)
        return;

    ThreadInfo& ti = getThreadInfo();
    MarkerStack& markers_stack = ti.markers_stack[m_write_id];
    double  start = getTimeMilliseconds() - m_time_last_sync;
    size_t  layer = markers_stack.size();

    // Add to the stack of current markers
    markers_stack.push(Marker(start, -1.0, name, color, layer));
}

//-----------------------------------------------------------------------------
/// Stop the last pushed marker
void Profiler::popCpuMarker()
{
    // Don't do anything when frozen
    if(m_freeze_state == FROZEN || m_freeze_state == WAITING_FOR_UNFREEZE)
        return;

    ThreadInfo&    ti = getThreadInfo();
    assert(ti.markers_stack[m_write_id].size() > 0);

    MarkerStack& markers_stack = ti.markers_stack[m_write_id];
    MarkerList&  markers_done  = ti.markers_done[m_write_id];

    // Update the date of end of the marker
    Marker&     marker = markers_stack.top();
    marker.end = getTimeMilliseconds() - m_time_last_sync;

    // Remove the marker from the stack and add it to the list of markers done
    markers_done.push_front(marker);
    markers_stack.pop();
}

//-----------------------------------------------------------------------------
/// Swap buffering for the markers
void Profiler::synchronizeFrame()
{
    // Don't do anything when frozen
    if(m_freeze_state == FROZEN)
        return;

    // Avoid using several times getTimeMilliseconds(), which would yield different results
    double now = getTimeMilliseconds();

    // Swap buffers
    int old_write_id = m_write_id;
    m_write_id = !m_write_id;

    // For each thread:
    ThreadInfoList::iterator it_end = m_thread_infos.end();
    for(ThreadInfoList::iterator it = m_thread_infos.begin() ; it != it_end ; it++)
    {
        // Get the thread information
        ThreadInfo& ti = *it;

        MarkerList&  old_markers_done  = ti.markers_done[old_write_id];
        MarkerStack& old_markers_stack = ti.markers_stack[old_write_id];

        MarkerList&  new_markers_done  = ti.markers_done[m_write_id];
        MarkerStack& new_markers_stack = ti.markers_stack[m_write_id];

        // Clear the containers for the new frame
        new_markers_done.clear();
        while(!new_markers_stack.empty())
            new_markers_stack.pop();

        // Finish the markers in the stack of the previous frame
        // and start them for the next frame.

        // For each marker in the old stack:
        while(!old_markers_stack.empty())
        {
            // - finish the marker for the previous frame and add it to the old "done" list
            Marker& m = old_markers_stack.top();
            m.end = now - m_time_last_sync;
            old_markers_done.push_front(m);

            // - start a new one for the new frame
            Marker new_marker(0.0, -1.0, m.name.c_str(), m.color);
            new_markers_stack.push(new_marker);

            // - next iteration
            old_markers_stack.pop();
        }
    }

    // Remember the date of last synchronization
    m_time_between_sync = now - m_time_last_sync;
    m_time_last_sync = now;

    // Freeze/unfreeze as needed
    if(m_freeze_state == WAITING_FOR_FREEZE)
        m_freeze_state = FROZEN;
    else if(m_freeze_state == WAITING_FOR_UNFREEZE)
        m_freeze_state = UNFROZEN;
}

//-----------------------------------------------------------------------------
/// Draw the markers
void Profiler::draw()
{
    PROFILER_PUSH_CPU_MARKER("ProfilerDraw", 0xFF, 0xFF, 0x00);
    video::IVideoDriver*    driver = irr_driver->getVideoDriver();
    std::stack<Marker>      hovered_markers;

    drawBackground();

    // Force to show the pointer
    irr_driver->showPointer();

    int read_id = !m_write_id;

    // Compute some values for drawing (unit: pixels, but we keep floats for reducing errors accumulation)
    core::dimension2d<u32>    screen_size    = driver->getScreenSize();
    const double profiler_width = (1.0 - 2.0*MARGIN_X) * screen_size.Width;
    const double x_offset    = MARGIN_X*screen_size.Width;
    const double y_offset    = (MARGIN_Y + LINE_HEIGHT)*screen_size.Height;
    const double line_height = LINE_HEIGHT*screen_size.Height;

    size_t nb_thread_infos = m_thread_infos.size();


    double start = -1.0f;
    double end = -1.0f;
    for (size_t i = 0; i < nb_thread_infos; i++)
    {
        MarkerList& markers = m_thread_infos[i].markers_done[read_id];

        MarkerList::const_iterator it_end = markers.end();
        for (MarkerList::const_iterator it = markers.begin(); it != it_end; it++)
        {
            const Marker& m = *it;

            if (start < 0.0) start = m.start;
            else start = std::min(start, m.start);

            if (end < 0.0) end = m.end;
            else end = std::max(end, m.end);
        }
    }
    
    const double duration = end - start;
    const double factor = profiler_width / duration;

    // Get the mouse pos
    core::vector2di mouse_pos = GUIEngine::EventHandler::get()->getMousePos();

    // For each thread:
    for (size_t i = 0; i < nb_thread_infos; i++)
    {
        // Draw all markers
        MarkerList& markers = m_thread_infos[i].markers_done[read_id];

        if (markers.empty())
            continue;

        if (m_capture_report)
        {
            if (m_first_capture_sweep)
                m_capture_report_buffer->getStdStream() << "\"Thread\";";
            else
                m_capture_report_buffer->getStdStream() << i << ";";
        }
        MarkerList::const_iterator it_end = markers.end();
        for (MarkerList::const_iterator it = markers.begin(); it != it_end; it++)
        {
            const Marker&    m = *it;
            assert(m.end >= 0.0);

            if (m_capture_report)
            {
                if (m_first_capture_sweep)
                    m_capture_report_buffer->getStdStream() << "\"" << m.name << "\";";
                else
                    m_capture_report_buffer->getStdStream() << (int)round((m.end - m.start) * 1000) << ";";
            }
            core::rect<s32>    pos((s32)( x_offset + factor*m.start ),
                                   (s32)( y_offset + i*line_height ),
                                   (s32)( x_offset + factor*m.end ),
                                   (s32)( y_offset + (i+1)*line_height ));

            // Reduce vertically the size of the markers according to their layer
            pos.UpperLeftCorner.Y  += m.layer*2;
            pos.LowerRightCorner.Y -= m.layer*2;

            GL32_draw2DRectangle(m.color, pos);

            // If the mouse cursor is over the marker, get its information
            if(pos.isPointInside(mouse_pos))
                hovered_markers.push(m);
        }

        if (m_capture_report)
        {
            m_capture_report_buffer->getStdStream() << "\n";
            m_first_capture_sweep = false;
        }
    }
    
    // GPU profiler
    QueryPerf hovered_gpu_marker = Q_LAST;
    long hovered_gpu_marker_elapsed = 0;
    int gpu_y = int(y_offset + nb_thread_infos*line_height + line_height/2);
    float total = 0;
    unsigned int gpu_timers[Q_LAST];
    for (unsigned i = 0; i < Q_LAST; i++)
    {
        gpu_timers[i] = irr_driver->getGPUTimer(i).elapsedTimeus();
        total += gpu_timers[i];
    }
    
    static video::SColor colors[] = {
        video::SColor(255, 255, 0, 0),
        video::SColor(255, 0, 255, 0),
        video::SColor(255, 0, 0, 255),
        video::SColor(255, 255, 255, 0),
        video::SColor(255, 255, 0, 255),
        video::SColor(255, 0, 255, 255)
    };

    if (hovered_markers.size() == 0)
    {
        float curr_val = 0;
        for (unsigned i = 0; i < Q_LAST; i++)
        {
            //Log::info("GPU Perf", "Phase %d : %d us\n", i, irr_driver->getGPUTimer(i).elapsedTimeus());

            float elapsed = float(gpu_timers[i]);
            core::rect<s32> pos((s32)(x_offset + (curr_val / total)*profiler_width),
                (s32)(y_offset + gpu_y),
                (s32)(x_offset + ((curr_val + elapsed) / total)*profiler_width),
                (s32)(y_offset + gpu_y + line_height));

            curr_val += elapsed;
            GL32_draw2DRectangle(colors[i % 6], pos);

            if (pos.isPointInside(mouse_pos))
            {
                hovered_gpu_marker = (QueryPerf)i;
                hovered_gpu_marker_elapsed = gpu_timers[i];
            }

            if (m_capture_report)
            {
                if (m_first_gpu_capture_sweep)
                    m_gpu_capture_report_buffer->getStdStream() << GPU_Phase[i] << ";";
                else
                    m_gpu_capture_report_buffer->getStdStream() << elapsed << ";";
            }
        }

        if (m_capture_report)
        {
            m_gpu_capture_report_buffer->getStdStream() << "\n";
            m_first_gpu_capture_sweep = false;
        }
    }

    // Draw the end of the frame
    {
        s32 x_sync = (s32)(x_offset + factor*m_time_between_sync);
        s32 y_up_sync = (s32)(MARGIN_Y*screen_size.Height);
        s32 y_down_sync = (s32)( (MARGIN_Y + (2+nb_thread_infos)*LINE_HEIGHT)*screen_size.Height );

        driver->draw2DLine(core::vector2di(x_sync, y_up_sync),
                           core::vector2di(x_sync, y_down_sync),
                           video::SColor(0xFF, 0x00, 0x00, 0x00));
    }

    // Draw the hovered markers' names
    gui::ScalableFont* font = GUIEngine::getFont();
    if (font)
    {
        core::stringw text;
        while(!hovered_markers.empty())
        {
            Marker& m = hovered_markers.top();
            std::ostringstream oss;
            oss.precision(4);
            oss << m.name << " [" << (m.end - m.start) << " ms / ";
            oss.precision(3);
            oss << (m.end - m.start)*100.0 / duration << "%]" << std::endl;
            text += oss.str().c_str();
            hovered_markers.pop();
        }
        font->draw(text, MARKERS_NAMES_POS, video::SColor(0xFF, 0xFF, 0x00, 0x00));

        if (hovered_gpu_marker != Q_LAST)
        {
            std::ostringstream oss;
            oss << GPU_Phase[hovered_gpu_marker] << " : " << hovered_gpu_marker_elapsed << " us";
            font->draw(oss.str().c_str(), GPU_MARKERS_NAMES_POS, video::SColor(0xFF, 0xFF, 0x00, 0x00));
        }
    }

    if (m_capture_report)
    {
        font->draw("Capturing profiler report...", MARKERS_NAMES_POS, video::SColor(0xFF, 0x00, 0x90, 0x00));
    }

    PROFILER_POP_CPU_MARKER();
}

//-----------------------------------------------------------------------------
/// Handle freeze/unfreeze
void Profiler::onClick(const core::vector2di& mouse_pos)
{
    video::IVideoDriver*            driver = irr_driver->getVideoDriver();
    const core::dimension2d<u32>&   screen_size = driver->getScreenSize();

    core::rect<s32>background_rect((int)(MARGIN_X                      * screen_size.Width),
                                   (int)(MARGIN_Y                      * screen_size.Height),
                                   (int)((1.0-MARGIN_X)                * screen_size.Width),
                                   (int)((MARGIN_Y + 3.0f*LINE_HEIGHT) * screen_size.Height));

    if(!background_rect.isPointInside(mouse_pos))
        return;

    switch(m_freeze_state)
    {
    case UNFROZEN:
        m_freeze_state = WAITING_FOR_FREEZE;
        break;

    case FROZEN:
        m_freeze_state = WAITING_FOR_UNFREEZE;
        break;

    case WAITING_FOR_FREEZE:
    case WAITING_FOR_UNFREEZE:
        // the user should not be that quick, and we prefer avoiding to introduce
        // bugs by unfrozing it while it has not frozen yet.
        // Same the other way around.
        break;
    }
}

//-----------------------------------------------------------------------------
/// Helper to draw a white background
void Profiler::drawBackground()
{
    video::IVideoDriver*            driver = irr_driver->getVideoDriver();
    const core::dimension2d<u32>&   screen_size = driver->getScreenSize();

    core::rect<s32>background_rect((int)(MARGIN_X                      * screen_size.Width),
                                   (int)((MARGIN_Y + 0.25f)             * screen_size.Height),
                                   (int)((1.0-MARGIN_X)                * screen_size.Width),
                                   (int)((MARGIN_Y + 1.75f*LINE_HEIGHT) * screen_size.Height));

    video::SColor   color(0x88, 0xFF, 0xFF, 0xFF);
    GL32_draw2DRectangle(color, background_rect);
}
