//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Ben Au
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

#include "graphics/irr_driver.hpp"
#include "guiengine/ft_environment.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <algorithm>

using namespace gui;

namespace GUIEngine
{

// ----------------------------------------------------------------------------

FTEnvironment::FTEnvironment()
{
    checkError(FT_Init_FreeType(&m_ft_lib), "loading freetype library");

    loadFont();
}

// ----------------------------------------------------------------------------

FTEnvironment::~FTEnvironment()
{
    for (int i = 0; i < F_COUNT; ++i)
        checkError(FT_Done_Face(m_ft_face[i]), "removing freetype face");

    checkError(FT_Done_FreeType(m_ft_lib), "removing freetype library");

    if (m_ft_err > 0)
    {
        Log::error("Freetype Environment", "Can't destroy all fonts.");
        m_ft_err = 0;
    }
    else
        Log::info("Freetype Environment", "Successfully destroy all fonts.");
}

// ----------------------------------------------------------------------------

FT_Face FTEnvironment::getFace(const FontUse font)
{
    return m_ft_face[font];
}

// ----------------------------------------------------------------------------

void FTEnvironment::checkError(FT_Error err, const irr::core::stringc desc)
{
    if (err)
    {
        Log::error("Freetype Environment", "Something wrong when %s!", desc);
        m_ft_err++;
    }
}

// ----------------------------------------------------------------------------

void FTEnvironment::loadFont()
{
    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "Ubuntu-R.ttf", true)).c_str(),
                                0, &m_ft_face[F_DEFAULT]), "loading F_DEFAULT");

    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeSans.ttf",true)).c_str(),
                                0, &m_ft_face[F_DEFAULT_FALLBACK]), "loading F_DEFAULT_FALLBACK");

    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "wqy-microhei.ttf",true)).c_str(),
                                 0, &m_ft_face[F_CJK]), "loading F_CJK");

    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "NotoNaskhArabicUI-Bold.ttf",true)).c_str(),
                                 0, &m_ft_face[F_AR]), "loading F_AR");

    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "Ubuntu-B.ttf", true)).c_str(),
                                0, &m_ft_face[F_BOLD]), "loading F_BOLD");

    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeSansBold.ttf", true)).c_str(),
                                0, &m_ft_face[F_BOLD_FALLBACK]), "loading F_BOLD_FALLBACK");

    checkError(FT_New_Face(m_ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "SigmarOne.otf",true)).c_str(),
                                 0, &m_ft_face[F_DIGIT]), "loading F_DIGIT");

    //Set charmap
    for (int h = 0; h < F_COUNT; ++h)
    {
        for (int i = 0; i < m_ft_face[h]->num_charmaps; ++i)
        {
            FT_UShort pid = m_ft_face[h]->charmaps[i]->platform_id;
            FT_UShort eid = m_ft_face[h]->charmaps[i]->encoding_id;
            if (((pid == 0) && (eid == 3)) || ((pid == 3) && (eid == 1)))
                checkError(FT_Set_Charmap(m_ft_face[h], m_ft_face[h]->charmaps[i]), "setting charmaps");
        }
    }

    // Set face dpi
    // font size is resolution-dependent.
    // normal text will range from 0.8, in 640x* resolutions (won't scale
    // below that) to 1.0, in 1024x* resolutions, and linearly up
    // normal text will range from 0.2, in 640x* resolutions (won't scale
    // below that) to 0.4, in 1024x* resolutions, and linearly up
    const s32 screen_width = irr_driver->getFrameSize().Width;
    const s32 screen_height = irr_driver->getFrameSize().Height;
    float scale = std::max(0, screen_width - 640)/564.0f;

    // attempt to compensate for small screens
    if (screen_width < 1200) scale = std::max(0, screen_width - 640) / 750.0f;
    if (screen_width < 900 || screen_height < 700) scale = std::min(scale, 0.05f);

    const u32 normal_dpi = ((0.7f + 0.2f*scale)*27);
    const u32 title_dpi  = ((0.2f + 0.2f*scale)*120);
    const u32 digit_dpi  = ((0.7f + 0.2f*scale)*40);

    Log::info("Freetype Environment", "DPI for Normal Font is %d.", normal_dpi);
    Log::info("Freetype Environment", "DPI for Title Font is %d.", title_dpi);
    Log::info("Freetype Environment", "DPI for Digit Font is %d.", digit_dpi);

    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_DEFAULT], 0, normal_dpi), "setting F_DEFAULT size");
    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_DEFAULT_FALLBACK], 0, normal_dpi), "setting F_DEFAULT_FALLBACK size");
    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_CJK], 0, normal_dpi), "setting F_CJK size");
    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_AR], 0, normal_dpi), "setting F_AR size");
    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_BOLD], 0, title_dpi), "setting F_BOLD size");
    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_BOLD_FALLBACK], 0, title_dpi), "setting F_BOLD_FALLBACK size");
    checkError(FT_Set_Pixel_Sizes(m_ft_face[F_DIGIT], 0, digit_dpi), "setting F_DIGIT size");

    if (m_ft_err > 0)
    {
        Log::error("Freetype Environment", "Can't load all fonts.");
        m_ft_err = 0;
    }
    else
        Log::info("Freetype Environment", "Successfully loaded all fonts.");
}

FT_Library FTEnvironment::m_ft_lib = NULL;
FT_Error FTEnvironment::m_ft_err   = 0;

}   // guiengine
