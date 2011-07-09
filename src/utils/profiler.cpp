//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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
#include "graphics/irr_driver.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include <assert.h>
#include <stack>

Profiler profiler;

// Unit is in pencentage of the screen dimensions
#define MARGIN_X    0.02    // left and right margin
#define MARGIN_Y    0.02    // top margin
#define LINE_HEIGHT 0.015   // height of a line representing a thread

#define MARKERS_NAMES_POS core::rect<s32>(10,50,260,80)

#define TIME_DRAWN_MS 100.0 // the width of the profiler corresponds to TIME_DRAWN_MS milliseconds

// --- Begin portable precise timer ---
#ifdef WIN32
    #include <windows.h>

    static double _getTimeMilliseconds()
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
    static double _getTimeMilliseconds()
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
    thread_infos.resize(1);	// TODO: monothread now, should support multithreading
    write_id = 0;
    time_last_sync = _getTimeMilliseconds();
}

//-----------------------------------------------------------------------------
Profiler::~Profiler()
{
}

//-----------------------------------------------------------------------------
/// Push a new marker that starts now
void Profiler::pushCpuMarker(const char* name, const video::SColor& color)
{
    ThreadInfo& ti = getThreadInfo();
    MarkerStack& markers_stack = ti.markers_stack[write_id];
    double  start = _getTimeMilliseconds() - time_last_sync;
    size_t  layer = markers_stack.size();
    
    // Add to the stack of current markers
    markers_stack.push(Marker(start, -1.0, name, color, layer));
}

//-----------------------------------------------------------------------------
/// Stop the last pushed marker
void Profiler::popCpuMarker()
{
    ThreadInfo&	ti = getThreadInfo();
    assert(ti.markers_stack[write_id].size() > 0);
    
    MarkerStack& markers_stack = ti.markers_stack[write_id];
    MarkerList&  markers_done  = ti.markers_done[write_id];
    
    // Update the date of end of the marker
    Marker&     marker = markers_stack.top();
    marker.end = _getTimeMilliseconds() - time_last_sync;

    // Remove the marker from the stack and add it to the list of markers done
    markers_done.push_front(marker);
    markers_stack.pop();
}

//-----------------------------------------------------------------------------
/// Swap buffering for the markers
void Profiler::synchronizeFrame()
{
    // Avoid using several times _getTimeMilliseconds(), which would yield different results
    double now = _getTimeMilliseconds();

    // Swap buffers
    int old_write_id = write_id;
    write_id = !write_id;

    // For each thread:
    ThreadInfoList::iterator it_end = thread_infos.end();
    for(ThreadInfoList::iterator it = thread_infos.begin() ; it != it_end ; it++)
    {
        // Get the thread information
        ThreadInfo& ti = *it;

        MarkerList&  old_markers_done  = ti.markers_done[old_write_id];
        MarkerStack& old_markers_stack = ti.markers_stack[old_write_id];

        MarkerList&  new_markers_done  = ti.markers_done[write_id];
        MarkerStack& new_markers_stack = ti.markers_stack[write_id];

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
            m.end = now - time_last_sync;
            old_markers_done.push_front(m);

            // - start a new one for the new frame
            Marker new_marker(0.0, -1.0, m.name.c_str(), m.color);
            new_markers_stack.push(new_marker);

            // - next iteration
            old_markers_stack.pop();
        }
    }
    
    // Remember the date of last synchronization
    time_last_sync = now;
}

//-----------------------------------------------------------------------------
/// Draw the markers
void Profiler::draw()
{
    video::IVideoDriver*    driver = irr_driver->getVideoDriver();
    std::stack<Marker>      hovered_markers;
    
    drawBackground();
    
    // Force to show the pointer
    irr_driver->showPointer();
    
    int read_id = !write_id;

    // Compute some values for drawing (unit: pixels, but we keep floats for reducing errors accumulation)
    core::dimension2d<u32>	screen_size	= driver->getScreenSize();
    const float profiler_width = (1.0 - 2.0*MARGIN_X) * screen_size.Width;
    const float x_offset    = MARGIN_X*screen_size.Width;
    const float y_offset    = (MARGIN_Y + LINE_HEIGHT)*screen_size.Height;
    const float line_height = LINE_HEIGHT*screen_size.Height;

    size_t nb_thread_infos = thread_infos.size();

    const float factor = profiler_width / TIME_DRAWN_MS;
    
    // Get the mouse pos
    core::position2di mouse_pos = GUIEngine::EventHandler::get()->getMousePos();

    // For each thread:
    for(size_t i=0 ; i < nb_thread_infos ; i++)
    {
        // Draw all markers
        MarkerList& markers = thread_infos[i].markers_done[read_id];
        
        if(markers.empty())
            continue;

        MarkerList::const_iterator it_end = markers.end();
        for(MarkerList::const_iterator it = markers.begin() ; it != it_end ; it++)
        {
            const Marker&	m = *it;
            assert(m.end >= 0.0);
            core::rect<s32>	pos(x_offset + factor*m.start,
                                y_offset + i*line_height,
                                x_offset + factor*m.end,
                                y_offset + (i+1)*line_height);
            
            pos.UpperLeftCorner.Y  += m.layer;
            pos.LowerRightCorner.Y -= m.layer;
            
            driver->draw2DRectangle(m.color, pos);
            
            // If the mouse cursor is over the marker, get its information
            if(pos.isPointInside(mouse_pos))
                hovered_markers.push(m);
        }
    }
    
    // Draw the hovered markers' names
    core::stringw text;
    while(!hovered_markers.empty())
    {
        Marker& m = hovered_markers.top();
        text += std::string(m.name + "\n").c_str();
        hovered_markers.pop();
    }
    
    /*//! draws an text and clips it to the specified rectangle if wanted
    virtual void draw(const core::stringw& text, const core::rect<s32>& position,
            video::SColor color, bool hcenter=false,
            bool vcenter=false, const core::rect<s32>* clip=0);

    virtual void draw(const core::stringw& text, const core::rect<s32>& position,
                      video::SColor color, bool hcenter,
                      bool vcenter, const core::rect<s32>* clip, bool ignoreRTL);*/
    
    
    gui::ScalableFont* font = GUIEngine::getFont();
    //font->draw(text, MARKERS_NAMES_POS, video::SColor(0xFF, 0xFF, 0xFF, 0xFF));
    font->draw(text, core::rect<s32>(50,50,150,150), video::SColor(0xFF, 0xFF, 0xFF, 0xFF));
}

//-----------------------------------------------------------------------------
/// Helper to draw a white background
void Profiler::drawBackground()
{
    video::IVideoDriver*            driver = irr_driver->getVideoDriver();
    const core::dimension2d<u32>&   screen_size = driver->getScreenSize();

    core::rect<s32> pos(MARGIN_X                        * screen_size.Width,
                        MARGIN_Y                        * screen_size.Height,
                        (1.0-MARGIN_X)                  * screen_size.Width,
                        (MARGIN_Y + 3.0*LINE_HEIGHT)    * screen_size.Height);

    video::SColor   color(0xFF, 0xFF, 0xFF, 0xFF);
    driver->draw2DRectangle(color, pos);
}
