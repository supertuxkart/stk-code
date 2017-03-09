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

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

#include <vector>

class STKTexture;
class STKTexManager;

class ThreadedTexLoader : public NoCopy
{
private:
    const unsigned m_tex_capacity;

    GLuint m_pbo;

    uint8_t* m_pbo_ptr;

    unsigned m_tex_size_loaded, m_waiting_timeout;

    pthread_t m_thread;

    pthread_cond_t m_cond_request;

    Synchronised<bool> m_finished_loading;

    bool m_destroy;

    Synchronised<std::vector<STKTexture*> >
        m_threaded_loading_textures, m_completed_textures;

public:
    // ------------------------------------------------------------------------
    static void* startRoutine(void *obj);
    // ------------------------------------------------------------------------
    ThreadedTexLoader(unsigned capacity, GLuint pbo, uint8_t* pbo_ptr)
                    : m_tex_capacity(capacity), m_pbo(pbo), m_pbo_ptr(pbo_ptr),
                      m_tex_size_loaded(0), m_waiting_timeout(0),
                      m_finished_loading(false), m_destroy(false)
    {
        pthread_cond_init(&m_cond_request, NULL);
        pthread_create(&m_thread, NULL, &startRoutine, this);
    }
    // ------------------------------------------------------------------------
    ~ThreadedTexLoader()
    {
        m_destroy = true;
        pthread_cond_destroy(&m_cond_request);
        pthread_join(m_thread, NULL);
    }
    // ------------------------------------------------------------------------
    void addTexture(STKTexture* t)
    {
        m_threaded_loading_textures.lock();
        m_threaded_loading_textures.getData().push_back(t);
        pthread_cond_signal(&m_cond_request);
        m_threaded_loading_textures.unlock();
    }
    // ------------------------------------------------------------------------
    bool finishedLoading() const     { return m_finished_loading.getAtomic(); }
    // ------------------------------------------------------------------------
    void handleCompletedTextures();

};   // ThreadedTexLoader

#endif
