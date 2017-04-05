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

#if !(defined(SERVER_ONLY) || defined(USE_GLES2))

#include "graphics/threaded_tex_loader.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <cassert>
#include <ITexture.h>

// ----------------------------------------------------------------------------
void* ThreadedTexLoader::startRoutine(void *obj)
{
    ThreadedTexLoader* ttl = (ThreadedTexLoader*)obj;
    VS::setThreadName((std::string("ThrTexLoader") +
        StringUtils::toString(ttl->m_pbo_offset / 1024 / 1024)).c_str());
    while (true)
    {
        pthread_mutex_lock(&ttl->m_mutex);
        bool finished = ttl->finishedLoading();
        pthread_mutex_unlock(&ttl->m_mutex);
        if (finished)
        {
            continue;
        }
        pthread_mutex_lock(ttl->m_texture_queue_mutex);
        bool waiting = ttl->m_stktm->isThreadedLoadTexturesEmpty();
        ttl->m_last_queue_ready.setAtomic(!ttl->m_completed_textures.empty() &&
            waiting);
        while (waiting)
        {
            pthread_cond_wait(ttl->m_cond_request, ttl->m_texture_queue_mutex);
            waiting = ttl->m_stktm->isThreadedLoadTexturesEmpty();
        }
        irr::video::ITexture* target_tex =
            ttl->m_stktm->getThreadedLoadTexture();
        if (strcmp(target_tex->getName().getPtr(), "delete_ttl") == 0)
        {
            ttl->m_stktm->removeThreadedLoadTexture();
            ttl->m_stktm->setThreadedLoadTextureCounter(-1);
            pthread_mutex_unlock(ttl->m_texture_queue_mutex);
            ttl->setCanBeDeleted();
            return NULL;
        }
        assert(target_tex->getTextureSize() <= ttl->m_tex_capacity);
        if (target_tex->getTextureSize() + ttl->m_tex_size_loaded >
            ttl->m_tex_capacity)
        {
            pthread_mutex_lock(&ttl->m_mutex);
            ttl->setFinishLoading();
            pthread_mutex_unlock(&ttl->m_mutex);
            pthread_mutex_unlock(ttl->m_texture_queue_mutex);
            continue;
        }
        ttl->m_stktm->removeThreadedLoadTexture();
        pthread_mutex_unlock(ttl->m_texture_queue_mutex);
        target_tex->threadedReload(ttl->m_pbo_ptr + ttl->m_tex_size_loaded,
            ttl->m_stktm);
        target_tex->cleanThreadedLoader();
        ttl->m_tex_size_loaded += target_tex->getTextureSize();
        ttl->m_completed_textures.push_back(target_tex);
        pthread_mutex_lock(ttl->m_texture_queue_mutex);
        ttl->m_stktm->setThreadedLoadTextureCounter(-1);
        pthread_mutex_unlock(ttl->m_texture_queue_mutex);
    }
    return NULL;
}   // startRoutine

// ----------------------------------------------------------------------------
void ThreadedTexLoader::handleCompletedTextures()
{
    assert(m_locked);
    size_t offset = m_pbo_offset;
    for (irr::video::ITexture* tex : m_completed_textures)
    {
        size_t cur_offset = tex->getTextureSize();
        tex->threadedSubImage((void*)offset);
        offset += cur_offset;
    }
    m_completed_textures.clear();
}   // handleCompletedTextures

#endif
