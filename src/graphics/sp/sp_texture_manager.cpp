//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef SERVER_ONLY

#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_texture.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <string>

const int MAX_TA = 256;
#ifndef USE_GLES2
#include <squish.h>
#endif

namespace SP
{
SPTextureManager* SPTextureManager::m_sptm = NULL;
// ----------------------------------------------------------------------------
SPTextureManager::SPTextureManager()
                : m_max_threaded_load_obj
                  ((unsigned)std::thread::hardware_concurrency()),
                  m_gl_cmd_function_count(0)
{
    if (m_max_threaded_load_obj.load() == 0)
    {
        m_max_threaded_load_obj.store(2);
    }
    m_max_threaded_load_obj.store(m_max_threaded_load_obj.load() + 1);
    for (unsigned i = 0; i < m_max_threaded_load_obj; i++)
    {
        m_threaded_load_obj.emplace_back(
            [this, i]()->void
            {
                using namespace StringUtils;
                VS::setThreadName((toString(i) + "SPTM").c_str());
                while (true)
                {
                    std::unique_lock<std::mutex> ul(m_thread_obj_mutex);
                    m_thread_obj_cv.wait(ul, [this]
                        {
                            return !m_threaded_functions.empty();
                        });
                    if (m_max_threaded_load_obj == 0)
                    {
                        return;
                    }
                    std::function<bool()> copied =
                        m_threaded_functions.front();
                    m_threaded_functions.pop_front();
                    ul.unlock();
                    // if return false, re-added it to the back
                    if (copied() == false)
                    {
                        addThreadedFunction(copied);
                    }
                }
            });
    }

    if (CVS->useArrayTextures())
    {
        Log::info("SPTextureManager", "Enable array textures, size %d",
            irr_driver->getVideoDriver()->getDriverAttributes()
            .getAttributeAsDimension2d("MAX_TEXTURE_SIZE").Width);
        glGenTextures(1, &m_all_textures_array);
        sp_prefilled_tex[0] = m_all_textures_array;
        initTextureArray();
    }

    m_textures["unicolor_white"] = SPTexture::getWhiteTexture();
    m_textures[""] = SPTexture::getTransparentTexture();
}   // SPTextureManager

// ----------------------------------------------------------------------------
SPTextureManager::~SPTextureManager()
{
    m_max_threaded_load_obj.store(0);
    std::unique_lock<std::mutex> ul(m_thread_obj_mutex);
    m_threaded_functions.push_back([](){ return true; });
    m_thread_obj_cv.notify_all();
    ul.unlock();
    for (std::thread& t : m_threaded_load_obj)
    {
        t.join();
    }
    m_threaded_load_obj.clear();
    if (m_all_textures_array != 0)
    {
        glDeleteTextures(1, &m_all_textures_array);
    }
}   // ~SPTextureManager

// ----------------------------------------------------------------------------
void SPTextureManager::initTextureArray()
{
#ifdef USE_GLES2
    unsigned upload_format = GL_RGBA;
#else
    unsigned upload_format = GL_BGRA;
#endif
    const unsigned size = irr_driver->getVideoDriver()->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE").Width;
    std::vector<unsigned> white, transparent;
    white.resize(size * size, -1);
    transparent.resize(size * size, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_all_textures_array);
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    if (CVS->isTextureCompressionEnabled())
    {
        std::vector<std::pair<core::dimension2du, unsigned> > mipmap_sizes;
        unsigned width = size;
        unsigned height = size;
        mipmap_sizes.emplace_back(core::dimension2du(width, height),
            squish::GetStorageRequirements(width, height, squish::kDxt5));
        while (true)
        {
            width = width < 2 ? 1 : width >> 1;
            height = height < 2 ? 1 : height >> 1;
            mipmap_sizes.emplace_back(core::dimension2du(width, height),
                squish::GetStorageRequirements(width, height, squish::kDxt5));
            if (width == 1 && height == 1)
            {
                break;
            }
        }
        unsigned cur_mipmap_size = 0;
        for (unsigned i = 0; i < mipmap_sizes.size(); i++)
        {
            cur_mipmap_size = mipmap_sizes[i].second;
            glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, i,
                GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
                mipmap_sizes[i].first.Width, mipmap_sizes[i].first.Height,
                MAX_TA, 0, cur_mipmap_size * MAX_TA, NULL);
            glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, i,
                0, 0, 0,
                mipmap_sizes[i].first.Width, mipmap_sizes[i].first.Height, 1,
                GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, cur_mipmap_size,
                white.data());
            glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, i,
                0, 0, 1,
                mipmap_sizes[i].first.Width, mipmap_sizes[i].first.Height, 1,
                GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, cur_mipmap_size,
                transparent.data());
        }
    }
    else
#endif
    {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
            size, size, MAX_TA, 0, upload_format, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
            size, size, 1, upload_format, GL_UNSIGNED_BYTE,
            white.data());
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1,
            size, size, 1, upload_format, GL_UNSIGNED_BYTE,
            transparent.data());
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}   // initTextureArray

// ----------------------------------------------------------------------------
void SPTextureManager::checkForGLCommand(bool before_scene)
{
    if (m_gl_cmd_function_count.load() == 0)
    {
        return;
    }
    while (true)
    {
        std::unique_lock<std::mutex> ul(m_gl_cmd_mutex);
        if (m_gl_cmd_functions.empty())
        {
            if (before_scene && m_gl_cmd_function_count.load() != 0)
            {
                ul.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            else
            {
                return;
            }
        }
        std::function<bool()> gl_cmd = m_gl_cmd_functions.front();
        m_gl_cmd_functions.pop_front();
        ul.unlock();
        // if return false, re-added it to the back
        if (gl_cmd() == false)
        {
            std::lock_guard<std::mutex> lock(m_gl_cmd_mutex);
            m_gl_cmd_functions.push_back(gl_cmd);
            if (!before_scene)
            {
                return;
            }
        }
        else
        {
            m_gl_cmd_function_count.fetch_sub(1);
        }
    }
}   // checkForGLCommand

// ----------------------------------------------------------------------------
std::shared_ptr<SPTexture> SPTextureManager::getTexture(const std::string& p,
                                                        Material* m,
                                                        bool undo_srgb)
{
    checkForGLCommand();
    auto ret = m_textures.find(p);
    if (ret != m_textures.end())
    {
        return ret->second;
    }
    int ta_idx = -1;

    if (CVS->useArrayTextures())
    {
        ta_idx = getTextureArrayIndex();
        if (ta_idx > MAX_TA)
        {
            Log::error("SPTextureManager", "Too many textures");
            // Give user a transparent texture
            return m_textures.at("");
        }
    }

    std::shared_ptr<SPTexture> t =
        std::make_shared<SPTexture>(p, m, undo_srgb, ta_idx);
    addThreadedFunction(std::bind(&SPTexture::threadedLoad, t));
    m_textures[p] = t;
    return t;
}   // getTexture

// ----------------------------------------------------------------------------
void SPTextureManager::removeUnusedTextures()
{
    for (auto it = m_textures.begin(); it != m_textures.end();)
    {
        if (it->second.use_count() == 1)
        {
            int ta_idx = it->second->getTextureArrayIndex();
            if (ta_idx != -1)
            {
                m_freed_texture_array_idx.push_back(ta_idx);
            }
            it = m_textures.erase(it);
        }
        else
        {
            it++;
        }
    }
}   // removeUnusedTextures

// ----------------------------------------------------------------------------
void SPTextureManager::dumpAllTexture()
{
    for (auto p : m_textures)
    {
        Log::info("STKTexManager", "%s size: %0.2fK", p.first.c_str(),
            (p.second->getWidth() * p.second->getHeight() * 4) / 1024.0f);
    }
}   // dumpAllTexture

// ----------------------------------------------------------------------------
}

#endif
