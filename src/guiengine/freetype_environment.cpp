//
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
#include "guiengine/freetype_environment.hpp"
#include "guiengine/TTF_handling.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

namespace irr
{
namespace gui
{

Ft_Env::Ft_Env()
{
    Ft_Env::ft_err += FT_Init_FreeType(&(Ft_Env::ft_lib));

    loadFont();
}

Ft_Env::~Ft_Env()
{
    for (int i = 0; i < FONTNUM; ++i)
        Ft_Env::ft_err += FT_Done_Face((Ft_Env::ft_face[i]));

    Ft_Env::ft_err += FT_Done_FreeType(Ft_Env::ft_lib);

    if (Ft_Env::ft_err > 0)
        Log::error("Freetype Environment", "Can't destroy all fonts.");
    else
        Log::info("Freetype Environment", "Successfully destroy all fonts.");
}

void Ft_Env::loadFont()
{
    Ft_Env::ft_err += FT_New_Face(Ft_Env::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeSans.ttf",true)).c_str(),
                                 0, &(Ft_Env::ft_face[F_DEFAULT]));

    Ft_Env::ft_err += FT_New_Face(Ft_Env::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "wqy-microhei.ttf",true)).c_str(),
                                 0, &(Ft_Env::ft_face[F_CJK]));

    Ft_Env::ft_err += FT_New_Face(Ft_Env::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "amiri-bold.ttf",true)).c_str(),
                                 0, &(Ft_Env::ft_face[F_AR]));

    Ft_Env::ft_err += FT_New_Face(Ft_Env::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "Layne_Hansom.ttf",true)).c_str(),
                                 0, &(Ft_Env::ft_face[F_LAYNE])); //to be removed?

    Ft_Env::ft_err += FT_New_Face(Ft_Env::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeSansBold.ttf",true)).c_str(),
                                 0, &(Ft_Env::ft_face[F_BOLD]));

    Ft_Env::ft_err += FT_New_Face(Ft_Env::ft_lib, (file_manager->getAssetChecked
                                (FileManager::TTF, "FreeMonoBold.ttf",true)).c_str(),
                                 0, &(Ft_Env::ft_face[F_DIGIT]));

    //Set charmap
    for (int h = 0; h < FONTNUM; ++h)
    {
        for (int i = 0; i < Ft_Env::ft_face[h]->num_charmaps; ++i)
        {
            FT_UShort pid = Ft_Env::ft_face[h]->charmaps[i]->platform_id;
            FT_UShort eid = Ft_Env::ft_face[h]->charmaps[i]->encoding_id;
            if (((pid == 0) && (eid == 3)) || ((pid == 3) && (eid == 1)))
                Ft_Env::ft_err += FT_Set_Charmap(Ft_Env::ft_face[h], Ft_Env::ft_face[h]->charmaps[i]);
        }
    }

    if (Ft_Env::ft_err > 0)
        Log::error("Freetype Environment", "Can't load all fonts.");
    else
        Log::info("Freetype Environment", "Successfully loaded all fonts.");
}

FT_Library Ft_Env::ft_lib = NULL;
FT_Error Ft_Env::ft_err = 0;

} // end namespace gui
} // end namespace irr
#endif // ENABLE_FREETYPE
