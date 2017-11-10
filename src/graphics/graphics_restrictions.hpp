//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#ifndef HEADER_GRAPHICS_RESTrICTIONS_HPP
#define HEADER_GRAPHICS_RESTrICTIONS_HPP

/**
 * \defgroup graphics
 */

#include <string>
#include <vector>

namespace GraphicsRestrictions
{
    /** Which graphical restrictions can be defined. Note that
     *  the variable m_names_of_restrictions in the cpp file contains the
     *  string representation used in the XML files. Any change to this
     *  type declaration needs a change in that variable as well. */
    enum GraphicsRestrictionsType
    {
        GR_UNIFORM_BUFFER_OBJECT,
        GR_GEOMETRY_SHADER,
        GR_DRAW_INDIRECT,
        GR_TEXTURE_VIEW,
        GR_TEXTURE_STORAGE,
        GR_IMAGE_LOAD_STORE,
        GR_BASE_INSTANCE,
        GR_COMPUTE_SHADER,
        GR_ARRAYS_OF_ARRAYS,
        GR_SHADER_STORAGE_BUFFER_OBJECT,
        GR_MULTI_DRAW_INDIRECT,
        GR_SHADER_ATOMIC_COUNTERS,
        GR_BUFFER_STORAGE,
        GR_BINDLESS_TEXTURE,
        GR_EXT_TEXTURE_COMPRESSION_S3TC,
        GR_AMD_VERTEX_SHADER_LAYER,
        GR_EXPLICIT_ATTRIB_LOCATION,
        GR_TEXTURE_FILTER_ANISOTROPIC,
        GR_TEXTURE_FORMAT_BGRA8888,
        GR_COLOR_BUFFER_FLOAT,
        GR_DRIVER_RECENT_ENOUGH,
        GR_HIGHDEFINITION_TEXTURES,
        GR_HIGHDEFINITION_TEXTURES_256,
        GR_ADVANCED_PIPELINE,
        GR_FRAMEBUFFER_SRGB,
        GR_FRAMEBUFFER_SRGB_WORKAROUND1,
        GR_FRAMEBUFFER_SRGB_WORKAROUND2,
        GR_GI,
        GR_FORCE_LEGACY_DEVICE,
        GR_VERTEX_ID_WORKING,
        GR_HARDWARE_SKINNING,
        GR_COUNT  /** MUST be last entry. */
    } ;

    void init(const std::string &driver_version,
              const std::string &card_name,
              const std::string &vendor       );
    bool isDisabled(GraphicsRestrictionsType type);

    void unitTesting();
};   // HardwareStats

#endif
