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

#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <irrlicht.h>
#include <list>
#include <vector>
#include <stack>
#include <string>
#include <streambuf>
#include <ostream>
#include <iostream>


enum QueryPerf
{
    Q_SHADOWS_CASCADE0,
    Q_SHADOWS_CASCADE1,
    Q_SHADOWS_CASCADE2,
    Q_SHADOWS_CASCADE3,
    Q_SOLID_PASS1,
    Q_RSM,
    Q_RH,
    Q_GI,
    Q_ENVMAP,
    Q_SUN,
    Q_POINTLIGHTS,
    Q_SSAO,
    Q_SOLID_PASS2,
    Q_TRANSPARENT,
    Q_PARTICLES,
    Q_DISPLACEMENT,
    Q_DOF,
    Q_GODRAYS,
    Q_BLOOM,
    Q_TONEMAP,
    Q_MOTIONBLUR,
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
        profiler.pushCpuMarker(name, video::SColor(0xFF, r, g, b))

    #define PROFILER_POP_CPU_MARKER()  \
        profiler.popCpuMarker()

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

/** For profiling reports, we need a custom strijng stream that writes to a large
    pre-allocated buffer, to avoid allocating as much as possible durign profiling */
template <typename char_type>
struct ostreambuf : public std::basic_streambuf<char_type, std::char_traits<char_type> >
{
    ostreambuf(char_type* buffer, std::streamsize bufferLength)
    {
        // set the "put" pointer the start of the buffer and record it's length.
        this->setp(buffer, buffer + bufferLength);
    }
};


class StringBuffer
{
private:
    char* m_buffer;
    ostreambuf<char> ostreamBuffer;
    std::ostream messageStream;

public:

    StringBuffer(unsigned int size) : m_buffer((char*)calloc(size, 1)), ostreamBuffer(m_buffer, size), messageStream(&ostreamBuffer)
    {
    }

    ~StringBuffer()
    {
        free(m_buffer);
    }

    std::ostream& getStdStream() { return messageStream; }

    char* getRawBuffer() { return m_buffer; }
};

/**
  * \brief class that allows run-time graphical profiling through the use of markers
  * \ingroup utils
  */
class Profiler
{
private:
    struct Marker
    {
        double  start;  // Times of start and end, in milliseconds,
        double  end;    // relatively to the time of last synchronization
        size_t  layer;

        std::string     name;
        video::SColor   color;

        Marker(double start, double end, const char* name="N/A", const video::SColor& color=video::SColor(), size_t layer=0)
            : start(start), end(end), layer(layer), name(name), color(color)
        {
        }

        Marker(const Marker& ref)
            : start(ref.start), end(ref.end), layer(ref.layer), name(ref.name), color(ref.color)
        {
        }
    };

    typedef    std::list<Marker>   MarkerList;
    typedef    std::stack<Marker>  MarkerStack;

    struct ThreadInfo
    {
        MarkerList   markers_done[2];
        MarkerStack  markers_stack[2];
    };

    typedef    std::vector<ThreadInfo>  ThreadInfoList;

    ThreadInfoList  m_thread_infos;
    int             m_write_id;
    double          m_time_last_sync;
    double          m_time_between_sync;

    // Handling freeze/unfreeze by clicking on the display
    enum FreezeState
    {
        UNFROZEN,
        WAITING_FOR_FREEZE,
        FROZEN,
        WAITING_FOR_UNFREEZE,
    };

    FreezeState     m_freeze_state;

    bool m_capture_report;
    bool m_first_capture_sweep;
    bool m_first_gpu_capture_sweep;
    StringBuffer* m_capture_report_buffer;
    StringBuffer* m_gpu_capture_report_buffer;

public:
    Profiler();
    virtual ~Profiler();

    void    pushCpuMarker(const char* name="N/A", const video::SColor& color=video::SColor());
    void    popCpuMarker();
    void    synchronizeFrame();

    void    draw();

    void    onClick(const core::vector2di& mouse_pos);

    bool getCaptureReport() const { return m_capture_report; }
    void setCaptureReport(bool captureReport);

    bool isFrozen() const { return m_freeze_state == FROZEN; }

protected:
    // TODO: detect on which thread this is called to support multithreading
    ThreadInfo& getThreadInfo() { return m_thread_infos[0]; }
    void        drawBackground();


};

#endif // PROFILER_HPP
