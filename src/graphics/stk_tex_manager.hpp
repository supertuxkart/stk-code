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

#ifndef HEADER_STK_TEX_MANAGER_HPP
#define HEADER_STK_TEX_MANAGER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include "irrString.h"
#include "ITexture.h"

#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>

class STKTexture;
namespace irr
{
    namespace video { class SColor; }
}

class STKTexManager : public Singleton<STKTexManager>, NoCopy
{
private:
    std::unordered_map<std::string, irr::video::ITexture*> m_all_textures;

    /** Additional details to be shown in case that a texture is not found.
     *  This is used to specify details like: "while loading kart '...'" */
    std::string m_texture_error_message;

    // ------------------------------------------------------------------------
    irr::video::ITexture* findTextureInFileSystem(const std::string& filename,
                                        std::string* full_path);
public:
    // ------------------------------------------------------------------------
    STKTexManager() {}
    // ------------------------------------------------------------------------
    ~STKTexManager();
    // ------------------------------------------------------------------------
    irr::video::ITexture* getTexture(const std::string& path,
                std::function<void(irr::video::IImage*)> image_mani = nullptr);
    // ------------------------------------------------------------------------
    irr::video::ITexture* addTexture(irr::video::ITexture* texture);
    // ------------------------------------------------------------------------
    bool hasTexture(const std::string& path);
    // ------------------------------------------------------------------------
    bool removeTexture(irr::video::ITexture* texture, bool remove_all = false);
    // ------------------------------------------------------------------------
    int dumpTextureUsage();
    // ------------------------------------------------------------------------
    void reloadAllTextures();
    // ------------------------------------------------------------------------
    /** Returns the currently defined texture error message, which is used
     *  by event_handler.cpp to print additional info about irrlicht
     *  internal errors or warnings. If no error message is currently
     *  defined, the error message is "".
     */
    const std::string &getTextureErrorMessage()
    {
        return m_texture_error_message;
    }   // getTextureErrorMessage
    // ------------------------------------------------------------------------
    void setTextureErrorMessage(const std::string &error,
                                const std::string &detail="");
    // ------------------------------------------------------------------------
    /** Disables the texture error message again.
     */
    void unsetTextureErrorMessage()           { m_texture_error_message = ""; }
    // ------------------------------------------------------------------------
    /** Convenience function that loads a texture with default parameters
     *  but includes an error message.
     *  \param filename File name of the texture to load.
     *  \param error Error message, potentially with a '%' which will be
     *               replaced with detail.
     *  \param detail String to replace a '%' in the error message.
     */
    irr::video::ITexture* getTexture(const std::string &filename,
                                     const std::string &error_message,
                                     const std::string &detail="")
    {
        setTextureErrorMessage(error_message, detail);
        irr::video::ITexture *tex = getTexture(filename);
        unsetTextureErrorMessage();
        return tex;
    }   // getTexture
    // ------------------------------------------------------------------------
    /** Convenience function that loads a texture with default parameters
     *  but includes an error message.
     *  \param filename File name of the texture to load.
     *  \param error Error message, potentially with a '%' which will be
     *               replaced with detail.
     *  \param detail String to replace a '%' in the error message.
     */
    irr::video::ITexture* getTexture(const std::string &filename,
                                     char *error_message,
                                     char *detail = NULL)
    {
        if (!detail)
            return getTexture(filename, std::string(error_message),
                              std::string(""));

        return getTexture(filename, std::string(error_message),
                          std::string(detail));
    }   // getTexture
    // ------------------------------------------------------------------------
    std::unordered_map<std::string, irr::video::ITexture*>& getAllTextures()
                                                     { return m_all_textures; }

};   // STKTexManager

#endif
