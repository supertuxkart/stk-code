 //  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 SuperTuxKart Team
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

//#ifndef SERVER_ONLY // No GUI files in server builds
#ifndef __HEADER_LAYOUT_UTILS_HPP__
#define __HEADER_LAYOUT_UTILS_HPP__

// This file contains functions to determine the best number of
// rows and columns to display items in a grid, used among others
// by dynamic ribbon widgets. Further utilities may be added.

namespace LayoutUtils
{
    const float READABILITY_FACTOR = 1.15f;

    float estimateRowScore(const int rowCount, const int width, const int height,
        const float iconAspectRatio, const int maxIcons, float* heightRatio,
        float soft_min_height, float capSize = 0.5f);
}

#endif
//#endif // ifndef SERVER_ONLY