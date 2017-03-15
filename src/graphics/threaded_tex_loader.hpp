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

#if !(defined(SERVER_ONLY) || defined(USE_GLES2))

#include "utils/can_be_deleted.hpp"
#include "utils/no_copy.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

#include <vector>

namespace irr
{
    namespace video { class ITexture; }
}
class STKTexManager;

class ThreadedTexLoader : public NoCopy, public CanBeDeleted
{
private:
    const unsigned m_tex_capacity;

    const size_t m_pbo_offset;

    uint8_t* m_pbo_ptr;

    pthread_mutex_t m_mutex;

    pthread_mutex_t* m_texture_queue_mutex;

    pthread_cond_t* m_cond_request;

    STKTexManager* m_stktm;

    unsigned m_tex_size_loaded;

    pthread_t m_thread;

    bool m_finished_loading, m_locked;

    Synchronised<bool> m_last_queue_ready;

    std::vector<irr::video::ITexture*> m_completed_textures;

public:
    // ------------------------------------------------------------------------
    static void* startRoutine(void *obj);
    // ------------------------------------------------------------------------
    ThreadedTexLoader(unsigned capacity, size_t offset, uint8_t* pbo_ptr,
                      pthread_mutex_t* mutex, pthread_cond_t* cond,
                      STKTexManager* stktm)
                    : m_tex_capacity(capacity), m_pbo_offset(offset),
                      m_pbo_ptr(pbo_ptr), m_texture_queue_mutex(mutex),
                      m_cond_request(cond), m_stktm(stktm),
                      m_tex_size_loaded(0), m_finished_loading(false),
                      m_locked(false), m_last_queue_ready(false)
    {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_create(&m_thread, NULL, &startRoutine, this);
    }
    // ------------------------------------------------------------------------
    ~ThreadedTexLoader()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_join(m_thread, NULL);
    }
    // ------------------------------------------------------------------------
    bool finishedLoading() const                 { return m_finished_loading; }
    // ------------------------------------------------------------------------
    void setFinishLoading()
    {
        m_last_queue_ready.setAtomic(false);
        m_finished_loading = true;
        m_tex_size_loaded = 0;
    }
    // ------------------------------------------------------------------------
    bool lastQueueReady() const      { return m_last_queue_ready.getAtomic(); }
    // ------------------------------------------------------------------------
    void handleCompletedTextures();
    // ------------------------------------------------------------------------
    void lock()
    {
        pthread_mutex_lock(&m_mutex);
        m_locked = true;
    }
    // ------------------------------------------------------------------------
    void unlock(bool finish_it)
    {
        if (!m_locked) return;
        m_locked = false;
        if (finish_it)
            m_finished_loading = false;
        pthread_mutex_unlock(&m_mutex);
    }

};   // ThreadedTexLoader

#endif

#endif
