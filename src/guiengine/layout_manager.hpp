//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#ifndef __LAYOUT_MANAGER_HPP__
#define __LAYOUT_MANAGER_HPP__

#include <cstring> // for NULL
#include <string>

#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    class Widget;
    class AbstractTopLevelContainer;

    class LayoutManager
    {

        /**
         * \brief Receives as string the raw property value retrieved from XML file.
         * Will try to make sense of it, as an absolute value or a percentage.
         *
         * Return values :
         *     Will write to either absolute or percentage, depending on the case.
         *     Returns false if couldn't convert to either
         */
        static bool convertToCoord(std::string& x, int* absolute /* out */, int* percentage /* out */);

        static void recursivelyReadCoords(PtrVector<Widget>& widgets);

        /**
         * \brief Recursive call that lays out children widget within parent (or screen if none).
         *
         * Manages 'horizontal-row' and 'vertical-row' layouts, along with the proportions
         * of the remaining children, as well as absolute sizes and locations.
         */
        static void doCalculateLayout(PtrVector<Widget>& widgets, AbstractTopLevelContainer* topLevelContainer,
                                      Widget* parent);


    public:

        /**
         * \brief Recursive call that lays out children widget within parent (or screen if none).
         *
         * Manages 'horizontal-row' and 'vertical-row' layouts, along with the proportions
         * of the remaining children, as well as absolute sizes and locations.
         */
        static void calculateLayout(PtrVector<Widget>& widgets, AbstractTopLevelContainer* topLevelContainer);

        /**
         * \brief Find a widget's x, y, w and h coords from what is specified in the XML properties.
         * Most notably, expands coords relative to parent and percentages.
         */
        static void applyCoords(Widget* self, AbstractTopLevelContainer* topLevelContainer, Widget* parent);

        /**
         * \brief Find a widget's x, y, w and h coords from what is specified in the XML properties.
         * (First step; 'applyCoords' is the second step)
         */
        static void readCoords(Widget* self);
    };
}

#endif
