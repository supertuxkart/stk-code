//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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

#ifndef HEADER_THREADED_TEX_LOADER_HPP
#define HEADER_THREADED_TEX_LOADER_HPP

#include "utils/no_copy.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

class STKTexture;
class STKTexManager;

class ThreadedTexLoader : public NoCopy
{
private:
    unsigned m_id, m_tex_loaded, m_wait_count;

    bool m_destroy;

    pthread_t m_thread;

    Synchronised<unsigned> m_tex_size_used;

    STKTexManager* m_stktm;

public:
    // ------------------------------------------------------------------------
    static void* startRoutine(void *obj);
    // ------------------------------------------------------------------------
    ThreadedTexLoader(unsigned id, STKTexManager* stktm)
    {
        m_destroy = false;
        m_stktm = stktm;
        m_id = id;
        reset();
        pthread_create(&m_thread, NULL, &startRoutine, this);
    }
    // ------------------------------------------------------------------------
    ~ThreadedTexLoader()
    {
        m_destroy = true;
        pthread_join(m_thread, NULL);
    }
    // ------------------------------------------------------------------------
    void reset()
    {
        m_tex_size_used.setAtomic(0);
        m_tex_loaded = 0;
        m_wait_count = 0;
    }
    // ------------------------------------------------------------------------
    unsigned getTextureLoaded() const                  { return m_tex_loaded; }
    // ------------------------------------------------------------------------
    bool finishedLoading() const
    {
        return m_tex_size_used.getAtomic() > 1024 * 1024 * 4;
    }
};   // ThreadedTexLoader

#endif
