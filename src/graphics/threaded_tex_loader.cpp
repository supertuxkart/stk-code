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
#include "graphics/stk_tex_manager.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

// ----------------------------------------------------------------------------
void* ThreadedTexLoader::startRoutine(void *obj)
{
    ThreadedTexLoader* ttl = (ThreadedTexLoader*)obj;
    STKTexManager* stktm = ttl->m_stktm;
    while (!ttl->m_destroy)
    {
        bool empty = stktm->isThreadedLoaderEmpty();
        while (!empty)
        {
            empty = stktm->isThreadedLoaderEmpty();
            if (empty || ttl->finishedLoading())
                break;
            const unsigned target_tex = ttl->m_tex_loaded *
                stktm->getNumLoadingThread() + ttl->m_id;
            if (target_tex + 1 > stktm->getThreadedLoadingTextureNum())
            {
                if (ttl->m_wait_count++ > 10)
                    ttl->m_tex_size_used.setAtomic(1000000000);
                continue;
            }
            STKTexture* stkt = stktm->getThreadedLoadingTexture(target_tex);
            const unsigned prev_used_size = ttl->m_tex_size_used.getAtomic();
            ttl->m_tex_size_used.setAtomic(prev_used_size +
                stkt->getTextureSize());
            if (ttl->finishedLoading())
                break;
            stkt->threadedReload(stktm->getPBOPtr() +
                ttl->m_id * 4 * 1024 * 1024 + prev_used_size);
            stkt->cleanThreadedLoader();
            ttl->m_tex_loaded++;
        }
        StkTime::sleep(50);
    }
    return NULL;
}   // startRoutine
