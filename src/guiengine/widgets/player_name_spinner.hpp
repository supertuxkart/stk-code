//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#ifndef PLAYER_NAME_SPINNER_HPP
#define PLAYER_NAME_SPINNER_HPP

#include "guiengine/widgets/spinner_widget.hpp"
#include <IGUIImage.h>

class KartSelectionScreen;

namespace GUIEngine
{
    /** A small extension to the spinner widget to add features like player ID
     *  management or badging */
    class PlayerNameSpinner : public GUIEngine::SpinnerWidget
    {
        int m_player_id;
        bool m_incorrect;
        irr::gui::IGUIImage* m_red_mark_widget;
        KartSelectionScreen* m_parent;
        //virtual EventPropagation focused(const int m_playerID) ;

    public:
        PlayerNameSpinner(KartSelectionScreen* parent, const int playerID);
        // ------------------------------------------------------------------------
        void setID(const int m_playerID);

        // ------------------------------------------------------------------------
        /** Add a red mark on the spinner to mean "invalid choice" */
        void markAsIncorrect();
        // ------------------------------------------------------------------------
        /** Remove any red mark set with 'markAsIncorrect' */
        void markAsCorrect();
    };
}

#endif

