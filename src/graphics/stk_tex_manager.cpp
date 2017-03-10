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

#include "graphics/stk_tex_manager.hpp"
#include "config/hardware_stats.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/materials.hpp"
#include "graphics/threaded_tex_loader.hpp"
#include "graphics/stk_texture.hpp"
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"

#include <algorithm>

// ----------------------------------------------------------------------------
STKTexManager::STKTexManager() : m_pbo(0), m_thread_size(0), m_tlt_added(0)
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    if (CVS->supportsThreadedTextureLoading())
    {
        m_thread_size = HardwareStats::getNumProcessors();
        m_thread_size = core::clamp(m_thread_size, 1, 8);
        static const unsigned max_pbo_size = 32 * 1024 * 1024;
        const unsigned each_capacity = max_pbo_size / m_thread_size;
        Log::info("STKTexManager", "%d thread(s) for texture loading,"
            " each capacity %d MB", m_thread_size,
            each_capacity / 1024 / 1024);
        glGenBuffers(1, &m_pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
        glBufferStorage(GL_PIXEL_UNPACK_BUFFER, max_pbo_size, NULL,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
            GL_MAP_COHERENT_BIT);
        uint8_t* pbo_ptr = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
            0, max_pbo_size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
            GL_MAP_COHERENT_BIT);
        size_t offset = 0;
        for (int i = 0; i < m_thread_size; i++)
        {
            m_all_tex_loaders.push_back(new ThreadedTexLoader(each_capacity,
                offset, pbo_ptr + offset));
            offset += each_capacity;
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

#endif
}   // STKTexManager

// ----------------------------------------------------------------------------
STKTexManager::~STKTexManager()
{
    removeTexture(NULL/*texture*/, true/*remove_all*/);
    for (ThreadedTexLoader* ttl : m_all_tex_loaders)
        delete ttl;
}   // ~STKTexManager

// ----------------------------------------------------------------------------
STKTexture* STKTexManager::findTextureInFileSystem(const std::string& filename,
                                                   std::string* full_path)
{
    io::path relative_path = file_manager->searchTexture(filename).c_str();
    if (relative_path.empty())
    {
        if (!m_texture_error_message.empty())
            Log::error("STKTexManager", "%s", m_texture_error_message.c_str());
        Log::error("STKTexManager", "Failed to load %s.", filename.c_str());
        return NULL;
    }
    *full_path =
        file_manager->getFileSystem()->getAbsolutePath(relative_path).c_str();
    for (auto p : m_all_textures)
    {
        if (p.second == NULL)
            continue;
        if (*full_path == p.first)
            return p.second;
    }

    return NULL;
}   // findTextureInFileSystem

// ----------------------------------------------------------------------------
video::ITexture* STKTexManager::getTexture(const std::string& path, bool srgb,
                                           bool premul_alpha,
                                           bool set_material, bool mesh_tex,
                                           bool no_upload, bool single_channel,
                                           bool create_if_unfound)
{
    auto ret = m_all_textures.find(path);
    if (!no_upload && ret != m_all_textures.end())
        return ret->second;

    STKTexture* new_texture = NULL;
    std::string full_path;
    if (path.find('/') == std::string::npos)
    {
        new_texture = findTextureInFileSystem(path, &full_path);
        if (full_path.empty())
            return NULL;
        if (!no_upload && new_texture)
            return new_texture;
    }

    if (create_if_unfound)
    {
        new_texture = new STKTexture(full_path.empty() ? path : full_path,
            srgb, premul_alpha, set_material, mesh_tex, no_upload,
            single_channel);
        if (new_texture->getOpenGLTextureName() == 0 && !no_upload)
        {
            const char* name = new_texture->getName().getPtr();
            if (!m_texture_error_message.empty())
            {
                Log::error("STKTexManager", "%s",
                    m_texture_error_message.c_str());
            }
            Log::error("STKTexManager", "Texture %s not found or invalid.",
                name);
            m_all_textures[name] = NULL;
            delete new_texture;
            return NULL;
        }
        if (new_texture->useThreadedLoading())
        {
            m_all_tex_loaders[m_tlt_added++ % m_thread_size]
                ->addTexture(new_texture);
        }
    }
    uploadBatch();

    if (create_if_unfound && !no_upload)
        addTexture(new_texture);
    return new_texture;
}   // getTexture

// ----------------------------------------------------------------------------
video::ITexture* STKTexManager::addTexture(STKTexture* texture)
{
    m_all_textures[texture->getName().getPtr()] = texture;
    return texture;
}   // addTexture

// ----------------------------------------------------------------------------
void STKTexManager::removeTexture(STKTexture* texture, bool remove_all)
{
#ifdef DEBUG
    std::vector<std::string> undeleted_texture;
#endif
    auto p = m_all_textures.begin();
    while (p != m_all_textures.end())
    {
        if (remove_all || p->second == texture)
        {
            if (remove_all && p->second == NULL)
            {
                p = m_all_textures.erase(p);
                continue;
            }
#ifdef DEBUG
            if (remove_all && p->second->getReferenceCount() != 1)
                undeleted_texture.push_back(p->second->getName().getPtr());
#endif
            p->second->drop();
            p = m_all_textures.erase(p);
        }
        else
        {
           p++;
        }
    }
#ifdef DEBUG
    if (!remove_all) return;
    for (const std::string& s : undeleted_texture)
    {
        Log::error("STKTexManager", "%s undeleted!", s.c_str());
    }
#endif
}   // removeTexture

// ----------------------------------------------------------------------------
void STKTexManager::dumpAllTexture(bool mesh_texture)
{
    for (auto p : m_all_textures)
    {
        if (!p.second || (mesh_texture && !p.second->isMeshTexture()))
            continue;
        Log::info("STKTexManager", "%s size: %0.2fK", p.first.c_str(),
            float(p.second->getTextureSize()) / 1024);
    }
}   // dumpAllTexture

// ----------------------------------------------------------------------------
int STKTexManager::dumpTextureUsage()
{
    int size = 0;
    for (auto p : m_all_textures)
    {
        if (p.second == NULL)
            continue;
        size += p.second->getTextureSize() / 1024 / 1024;
    }
    Log::info("STKTexManager", "Total %dMB", size);
    return size;
}   // dumpAllTexture

// ----------------------------------------------------------------------------
video::ITexture* STKTexManager::getUnicolorTexture(const irr::video::SColor &c)
{
    std::string name = StringUtils::toString(c.color) + "unic";
    auto ret = m_all_textures.find(name);
    if (ret != m_all_textures.end())
        return ret->second;

    uint8_t* data = new uint8_t[2 * 2 * 4];
    memcpy(data, &c.color, sizeof(video::SColor));
    memcpy(data + 4, &c.color, sizeof(video::SColor));
    memcpy(data + 8, &c.color, sizeof(video::SColor));
    memcpy(data + 12, &c.color, sizeof(video::SColor));
    return addTexture(new STKTexture(data, name, 2));

}   // getUnicolorTexture

// ----------------------------------------------------------------------------
core::stringw STKTexManager::reloadTexture(const irr::core::stringw& name)
{
    core::stringw result;
#ifndef SERVER_ONLY
    if (CVS->isTextureCompressionEnabled())
        return L"Please disable texture compression for reloading textures.";

    if (name.empty())
    {
        for (auto p : m_all_textures)
        {
            if (p.second == NULL || !p.second->isMeshTexture())
                continue;
            p.second->reload();
            Log::info("STKTexManager", "%s reloaded",
                p.second->getName().getPtr());
        }
        return L"All textures reloaded.";
    }

    core::stringw list = name;
    list.make_lower().replace(L'\u005C', L'\u002F');
    std::vector<std::string> names =
        StringUtils::split(StringUtils::wideToUtf8(list), ';');
    for (const std::string& fname : names)
    {
        for (auto p : m_all_textures)
        {
            if (p.second == NULL || !p.second->isMeshTexture())
                continue;
            std::string tex_path =
                StringUtils::toLowerCase(p.second->getName().getPtr());
            std::string tex_name = StringUtils::getBasename(tex_path);
            if (fname == tex_name || fname == tex_path)
            {
                p.second->reload();
                result += tex_name.c_str();
                result += L" ";
                break;
            }
        }
    }
    if (result.empty())
        return L"Texture(s) not found!";
#endif   // !SERVER_ONLY
    return result + "reloaded.";
}   // reloadTexture

// ----------------------------------------------------------------------------
void STKTexManager::reset()
{
    m_tlt_added = 0;
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    if (!CVS->isAZDOEnabled()) return;
    for (auto p : m_all_textures)
    {
        if (p.second == NULL)
            continue;
        p.second->unloadHandle();
    }
    // Driver seems to crash if texture handles are not cleared...
    ObjectPass1Shader::getInstance()->recreateTrilinearSampler(0);
#endif
}   // reset

// ----------------------------------------------------------------------------
/** Sets an error message to be displayed when a texture is not found. This
 *  error message is shown before the "Texture %s not found or invalid"
 *  message. It can be used to supply additional details like what kart is
 *  currently being loaded.
 *  \param error Error message, potentially with a '%' which will be replaced
 *               with detail.
 *  \param detail String to replace a '%' in the error message.
 */
void STKTexManager::setTextureErrorMessage(const std::string &error,
                                           const std::string &detail)
{
    if (detail=="")
        m_texture_error_message = error;
    else
        m_texture_error_message = StringUtils::insertValues(error, detail);
}   // setTextureErrorMessage

// ----------------------------------------------------------------------------
void STKTexManager::uploadBatch()
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    if (!CVS->supportsThreadedTextureLoading()) return;
    bool uploaded = false;
    for (ThreadedTexLoader* ttl : m_all_tex_loaders)
    {
        if (ttl->finishedLoading())
        {
            if (!uploaded)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
                uploaded = true;
            }
            ttl->handleCompletedTextures();
        }
    }
    if (uploaded)
    {
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        GLenum reason = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        if (reason != GL_ALREADY_SIGNALED)
        {
            do
            {
                reason = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT,
                    1000000);
            }
            while (reason == GL_TIMEOUT_EXPIRED);
        }
        glDeleteSync(sync);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        for (ThreadedTexLoader* ttl : m_all_tex_loaders)
            ttl->unlockCompletedTextures();
    }
#endif
}   // uploadBatch
