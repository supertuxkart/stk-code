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
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/server_dummy_texture.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"

#include <algorithm>
#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#include <ge_texture.hpp>
#endif

#include <IFileSystem.h>

// ----------------------------------------------------------------------------
STKTexManager::~STKTexManager()
{
#ifndef SERVER_ONLY
    GE::GEVulkanDriver* gevd = GE::getVKDriver();
    if (gevd)
    {
        gevd->waitIdle(/*flush_command_loader*/false);
        gevd->setDisableWaitIdle(true);
    }
#endif

    removeTexture(NULL/*texture*/, true/*remove_all*/);

#ifndef SERVER_ONLY
    if (gevd)
        gevd->setDisableWaitIdle(false);
#endif
}   // ~STKTexManager

// ----------------------------------------------------------------------------
video::ITexture* STKTexManager::findTextureInFileSystem(const std::string& filename,
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
video::ITexture* STKTexManager::getTexture(const std::string& path,
                                std::function<void(video::IImage*)> image_mani)
{
    if (path.empty())
    {
        Log::error("STKTexManager", "Texture name is empty.");
        return NULL;
    }

    auto ret = m_all_textures.find(path);
    if (ret != m_all_textures.end())
        return ret->second;

    video::ITexture* new_texture = NULL;
    std::string full_path;
    if (path.find('/') == std::string::npos)
    {
        new_texture = findTextureInFileSystem(path, &full_path);
        if (full_path.empty())
            return NULL;
        if (new_texture)
            return new_texture;
    }

#ifdef SERVER_ONLY
    new_texture =
        new ServerDummyTexture(full_path.empty() ? path : full_path);
#else
    if (GUIEngine::isNoGraphics())
    {
        new_texture =
            new ServerDummyTexture(full_path.empty() ? path : full_path);
    }
    else
    {
        new_texture =
            GE::createTexture(full_path.empty() ? path : full_path,
            image_mani);
    }
    if (new_texture->loadingFailed())
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
#endif

    addTexture(new_texture);
    return new_texture;
}   // getTexture

// ----------------------------------------------------------------------------
video::ITexture* STKTexManager::addTexture(video::ITexture* texture)
{
    m_all_textures[texture->getName().getPtr()] = texture;
    return texture;
}   // addTexture

// ----------------------------------------------------------------------------
bool STKTexManager::removeTexture(video::ITexture* texture, bool remove_all)
{
    bool ret = false;
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
            ret = true;
        }
        else
        {
           p++;
        }
    }
#ifdef DEBUG
    if (!remove_all) return ret;
    for (const std::string& s : undeleted_texture)
    {
        Log::error("STKTexManager", "%s undeleted!", s.c_str());
    }
#endif
    if (remove_all)
        return true;
    return ret;
}   // removeTexture

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
}   // dumpTextureUsage

// ----------------------------------------------------------------------------
bool STKTexManager::hasTexture(const std::string& path)
{
    if (path.empty())
    {
        Log::error("STKTexManager", "Texture name is empty.");
        return false;
    }

    auto ret = m_all_textures.find(path);
    if (ret != m_all_textures.end())
        return true;

    video::ITexture* new_texture = NULL;
    std::string full_path;
    if (path.find('/') == std::string::npos)
    {
        new_texture = findTextureInFileSystem(path, &full_path);
        if (full_path.empty())
            return false;
        if (new_texture)
            return true;
    }
    return false;
}   // hasTexture

// ----------------------------------------------------------------------------
void STKTexManager::reloadAllTextures()
{
#ifndef SERVER_ONLY
    GE::GEVulkanDriver* gevd = GE::getVKDriver();
    if (gevd)
    {
        gevd->waitIdle();
        gevd->setDisableWaitIdle(true);
    }
#endif

    for (auto p : m_all_textures)
    {
        if (p.second == NULL)
            continue;
        p.second->reload();
    }

#ifndef SERVER_ONLY
    if (gevd)
        gevd->setDisableWaitIdle(false);
#endif
}   // reloadAllTextures
