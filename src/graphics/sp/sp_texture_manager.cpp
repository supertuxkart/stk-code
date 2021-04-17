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

namespace SP
{
SPTextureManager* SPTextureManager::m_sptm = NULL;
// ----------------------------------------------------------------------------
SPTextureManager::SPTextureManager()
                : m_max_threaded_load_obj
                  ((unsigned)std::thread::hardware_concurrency()),
                  m_gl_cmd_function_count(0)
{
    if (!CVS->isGLSL())
        return;
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
    m_textures["unicolor_white"] = SPTexture::getWhiteTexture();
    m_textures[""] = SPTexture::getTransparentTexture();
}   // SPTextureManager

// ----------------------------------------------------------------------------
SPTextureManager::~SPTextureManager()
{
    assert(m_threaded_load_obj.empty());
    removeUnusedTextures();
#ifdef DEBUG
    for (auto p : m_textures)
    {
        Log::error("SPTextureManager", "%s > 1 ref_count", p.first.c_str());
    }
#endif
}   // ~SPTextureManager

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
                                                        bool undo_srgb,
                                                        const std::string& cid)
{
    checkForGLCommand();
    auto ret = m_textures.find(p);
    if (ret != m_textures.end())
    {
        return ret->second;
    }
    std::shared_ptr<SPTexture> t =
        std::make_shared<SPTexture>(p, m, undo_srgb, cid);
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
            it = m_textures.erase(it);
        }
        else
        {
            it++;
        }
    }
}   // removeUnusedTextures

// ----------------------------------------------------------------------------
void SPTextureManager::dumpAllTextures()
{
    for (auto p : m_textures)
    {
        Log::info("SPTextureManager", "%s", p.first.c_str());
    }
}   // dumpAllTextures

// ----------------------------------------------------------------------------
core::stringw SPTextureManager::reloadTexture(const core::stringw& name)
{
    core::stringw result;
#ifndef SERVER_ONLY
    if (name.empty())
    {
        for (auto p : m_textures)
        {
            if (p.second->getPath().empty() ||
                p.second->getPath() == "unicolor_white")
            {
                continue;
            }
            addThreadedFunction(std::bind(&SPTexture::threadedLoad, p.second));
            Log::info("SPTextureManager", "%s reloaded",
                p.second->getPath().c_str());
        }
        return L"All textures reloaded.";
    }

    core::stringw list = name;
    list.make_lower().replace(L'\u005C', L'\u002F');
    std::vector<std::string> names =
        StringUtils::split(StringUtils::wideToUtf8(list), ';');
    for (const std::string& fname : names)
    {
        for (auto p : m_textures)
        {
            if (p.second->getPath().empty() ||
                p.second->getPath() == "unicolor_white")
            {
                continue;
            }
            std::string tex_path =
                StringUtils::toLowerCase(p.second->getPath());
            std::string tex_name = StringUtils::getBasename(tex_path);
            if (fname == tex_name || fname == tex_path)
            {
                addThreadedFunction(std::bind(&SPTexture::threadedLoad,
                    p.second));
                result += tex_name.c_str();
                result += L" ";
                break;
            }
        }
    }
    if (result.empty())
    {
        return L"Texture(s) not found!";
    }
#endif   // !SERVER_ONLY
    return result + "reloaded.";
}   // reloadTexture

// ----------------------------------------------------------------------------
}

#endif
