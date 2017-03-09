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

#include "graphics/threaded_tex_loader.hpp"
#include "graphics/stk_texture.hpp"

#include <cassert>

// ----------------------------------------------------------------------------
void* ThreadedTexLoader::startRoutine(void *obj)
{
    ThreadedTexLoader* ttl = (ThreadedTexLoader*)obj;
    while (!ttl->m_destroy)
    {
        if (ttl->finishedLoading())
        {
            continue;
        }
        ttl->m_threaded_loading_textures.lock();
        bool queue_empty = ttl->m_threaded_loading_textures.getData().empty();
        ttl->m_completed_textures.lock();
        bool no_unloaded_tex = ttl->m_completed_textures.getData().empty();
        ttl->m_completed_textures.unlock();
        bool waiting = queue_empty && no_unloaded_tex;
        while (waiting)
        {
            pthread_cond_wait(&ttl->m_cond_request,
                ttl->m_threaded_loading_textures.getMutex());
            waiting = ttl->m_threaded_loading_textures.getData().empty();
        }
        if (queue_empty)
        {
            if (ttl->m_waiting_timeout++ > 10)
            {
                ttl->m_finished_loading.setAtomic(true);
                ttl->m_tex_size_loaded = 0;
                ttl->m_waiting_timeout = 0;
            }
            ttl->m_threaded_loading_textures.unlock();
            continue;
        }
        else
        {
            ttl->m_waiting_timeout = 0;
        }
        STKTexture* target_tex = ttl->m_threaded_loading_textures.getData()[0];
        if (target_tex->getTextureSize() + ttl->m_tex_size_loaded >
            ttl->m_tex_capacity)
        {
            ttl->m_finished_loading.setAtomic(true);
            ttl->m_tex_size_loaded = 0;
            ttl->m_waiting_timeout = 0;
            ttl->m_threaded_loading_textures.unlock();
            continue;
        }
        ttl->m_threaded_loading_textures.getData().erase
            (ttl->m_threaded_loading_textures.getData().begin());
        ttl->m_threaded_loading_textures.unlock();
        target_tex->threadedReload(ttl->m_pbo_ptr + ttl->m_tex_size_loaded);
        target_tex->cleanThreadedLoader();
        ttl->m_tex_size_loaded += target_tex->getTextureSize();
        ttl->m_completed_textures.lock();
        ttl->m_completed_textures.getData().push_back(target_tex);
        ttl->m_completed_textures.unlock();
    }
    pthread_exit(NULL);
    return NULL;
}   // startRoutine

// ----------------------------------------------------------------------------
void ThreadedTexLoader::handleCompletedTextures()
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    m_completed_textures.lock();
    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
    size_t offset = 0;
    for (STKTexture* stkt : m_completed_textures.getData())
    {
        assert(!stkt->useThreadedLoading());
        glBindTexture(GL_TEXTURE_2D, stkt->getOpenGLTextureName());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, stkt->getSize().Width,
            stkt->getSize().Height, stkt->isSingleChannel() ? GL_RED : GL_BGRA,
            GL_UNSIGNED_BYTE, (const void*)offset);
        if (stkt->hasMipMaps())
            glGenerateMipmap(GL_TEXTURE_2D);
        offset += stkt->getTextureSize();
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    m_completed_textures.getData().clear();
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    GLenum reason = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    if (reason != GL_ALREADY_SIGNALED)
    {
        do
        {
            reason =
                glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
        }
        while (reason == GL_TIMEOUT_EXPIRED);
    }
    glDeleteSync(sync);
    m_finished_loading.setAtomic(false);
    m_completed_textures.unlock();
#endif
}   // handleCompletedTextures
