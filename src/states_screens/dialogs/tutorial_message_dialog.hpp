//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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


#ifndef HEADER_TUTORIAL_DIALOG_HPP
#define HEADER_TUTORIAL_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"
#include "utils/leak_check.hpp"

/**
 * \brief Messages shown during tutorial
 * \ingroup states_screens
 */
class TutorialMessageDialog : public GUIEngine::ModalDialog
{
private:

    bool m_stop_game;

public:


    TutorialMessageDialog(irr::core::stringw msg, bool stopGame);
    
    ~TutorialMessageDialog();
    
    virtual void onEnterPressedInternal() OVERRIDE;
    virtual void onUpdate(float dt) OVERRIDE;

    
    GUIEngine::EventPropagation processEvent(const std::string& eventSource) OVERRIDE;
};


#endif
