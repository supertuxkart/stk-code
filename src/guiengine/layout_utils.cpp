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

#include "guiengine/layout_utils.hpp"

#include "utils/log.hpp"

#include <algorithm> // FIXME CHECK
#include <assert.h>
#include <cmath>

namespace LayoutUtils
{
    // -----------------------------------------------------------------------------
    
    /** Computes a score based on multiple icon properties
      * (used to estimate the best number of rows)
      *  There are three parameters we try to optimize for:
      * 1 - Showing as many items as possible ;
      * 2 - having icons sufficiently big ;
      * 3 - Using the available area (least important)
      * \param[out] heightRatio   the proportion of max height that should be used
      */
    float estimateRowScore(const int rowCount, const int width, const int height,
        const float iconAspectRatio, const int maxIcons, float* heightRatio,
        float soft_min_height, float capSize)
    {
        assert(height > 0);
    
        const float row_height = (float)height / (float)rowCount;
        float test_height_ratio = 1.0f;
        float max_score_so_far = -1;
        float nextRowRatio = (float)(rowCount)/(float)(rowCount + 1);
    
        // We test multiple icons heights, as a smaller height might
        // be better than full height or an additional row
        while (test_height_ratio > nextRowRatio)
        {
            float icon_height = row_height * test_height_ratio;
            float icon_width = icon_height * iconAspectRatio * READABILITY_FACTOR;
    
            // FIXME - this doesn't account for the space that is lost to scrolling arrows,
            // if there are scrolling arrows
            const int icons_per_row = int(width / icon_width);
            int visible_items = std::min(maxIcons, icons_per_row * rowCount);
            // Screens with a huge amount of icons lose readability,
            // so items beyond the 30th count less
            float visible_items_score = std::min((float)visible_items, 18.0f + 0.4f * visible_items);
    
            // Used to penalize layouts where the icons are smaller than requested in XML
            // The penalty starts as less than quadratic, but becomes quickly more than
            // quadratic, to avoid layouts with tiny icons
            // Icons above the requested size don't receive a penalty
            // TODO: check how this interacts with high-dpi
            // TODO: properly account for the margins between rows, which reduce the true available height
            float icon_size_ratio = std::min(1.0f, icon_height / soft_min_height);
            icon_size_ratio = (icon_size_ratio * icon_size_ratio * icon_size_ratio * sqrtf(icon_size_ratio)) *
                            (2.0f - icon_size_ratio) * (2.0f - icon_size_ratio) * std::min(1.0f, icon_size_ratio * 1.5f);
    
            // We slightly penalize layouts that don't cover well the available area,
            // this mostly acts as a tie-breaker when all items can be displayed
            // at the requested icon size
            int taken_area = int(visible_items * icon_width * icon_height);
            float total_area = (float)(width * height);
            float area_factor = std::min(taken_area/total_area, 1.0f);
            area_factor = 0.9f + area_factor * 0.1f;
    
            // We compute the final score by combining the three elements,
            // with an extra penalty for missing the target number of icons
            // (which helps to avoid layouts that barely miss the target)
            float score = visible_items_score * icon_size_ratio * area_factor;
            if (visible_items < maxIcons)
                score *= 0.7f;
            if (visible_items < (maxIcons/2))
                score *= 0.9f;
            if (visible_items < (maxIcons/3))
                score *= 0.9f;
            if (visible_items < (maxIcons/4))
                score *= 0.9f;
            
            /*Log::info("LayoutUtils", "rows = %d; height ratio = %f; visible items = %d; area factor = %f; "
                "icon_height = %f; icon size ratio = %f; score = %f", rowCount, visible_items, test_height_ratio,
                area_factor, icon_height, icon_size_ratio, score);*/
    
            if (score > max_score_so_far)
            {
                *heightRatio = test_height_ratio;
                max_score_so_far = score;
            }
    
            test_height_ratio -= 0.05f;
            if (test_height_ratio < capSize)
                break;
        } // while icon height > greatest icon height possible with another row
    
        return max_score_so_far;
    }   // estimateRowScore

} // LayoutUtils

//#endif // ifndef SERVER_ONLY