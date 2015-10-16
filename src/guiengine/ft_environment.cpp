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

#ifdef ENABLE_FREETYPE
#include "guiengine/ft_environment.hpp"
#include "guiengine/get_font_properties.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

using namespace gui;

namespace GUIEngine
{

FTEnvironment::FTEnvironment()
{
    FTEnvironment::ft_err += FT_Init_FreeType(&(FTEnvironment::ft_lib));

    loadFont();
}

FTEnvironment::~FTEnvironment()
{
    for (int i = 0; i < FONTNUM; ++i)
        FTEnvironment::ft_err += FT_Done_Face((FTEnvironment::ft_face[i]));

    FTEnvironment::ft_err += FT_Done_FreeType(FTEnvironment::ft_lib);

    if (FTEnvironment::ft_err > 0)
        Log::error("Freetype Environment", "Can't destroy all fonts.");
    else
        Log::info("Freetype Environment", "Successfully destroy all fonts.");
}

void FTEnvironment::loadFont()
{
    FTEnvironment::ft_err += FT_New_Face(FTEnvironment::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeSans.ttf",true)).c_str(),
                                 0, &(FTEnvironment::ft_face[F_DEFAULT]));

    FTEnvironment::ft_err += FT_New_Face(FTEnvironment::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "wqy-microhei.ttf",true)).c_str(),
                                 0, &(FTEnvironment::ft_face[F_CJK]));

    FTEnvironment::ft_err += FT_New_Face(FTEnvironment::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "NotoNaskhArabicUI-Bold.ttf",true)).c_str(),
                                 0, &(FTEnvironment::ft_face[F_AR]));

    FTEnvironment::ft_err += FT_New_Face(FTEnvironment::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeSansBold.ttf",true)).c_str(),
                                 0, &(FTEnvironment::ft_face[F_BOLD]));

    FTEnvironment::ft_err += FT_New_Face(FTEnvironment::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeMonoBold.ttf",true)).c_str(),
                                 0, &(FTEnvironment::ft_face[F_DIGIT]));

    //Set charmap
    for (int h = 0; h < FONTNUM; ++h)
    {
        for (int i = 0; i < FTEnvironment::ft_face[h]->num_charmaps; ++i)
        {
            FT_UShort pid = FTEnvironment::ft_face[h]->charmaps[i]->platform_id;
            FT_UShort eid = FTEnvironment::ft_face[h]->charmaps[i]->encoding_id;
            if (((pid == 0) && (eid == 3)) || ((pid == 3) && (eid == 1)))
                FTEnvironment::ft_err += FT_Set_Charmap(FTEnvironment::ft_face[h], FTEnvironment::ft_face[h]->charmaps[i]);
        }
    }

    if (FTEnvironment::ft_err > 0)
        Log::error("Freetype Environment", "Can't load all fonts.");
    else
        Log::info("Freetype Environment", "Successfully loaded all fonts.");
}

FT_Library FTEnvironment::ft_lib = NULL;
FT_Error FTEnvironment::ft_err   = 0;

}   // guiengine
#endif // ENABLE_FREETYPE
