//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef OFFLINE_KART_SELECTION_HPP
#define OFFLINE_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"
#include "guiengine/screen.hpp"

class OfflineKartSelectionScreen : public KartSelectionScreen, public GUIEngine::ScreenSingleton<OfflineKartSelectionScreen>
{
    friend class GUIEngine::ScreenSingleton<OfflineKartSelectionScreen>;
    protected:
        OfflineKartSelectionScreen();
        virtual ~OfflineKartSelectionScreen();

    public:
        static bool isRunning();

        // we do not override anything, this class is just there to have a singleton
};

#endif // OFFLINE_KART_SELECTION_HPP
