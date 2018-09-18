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

#include "guiengine/widgets/player_name_spinner.hpp"
#include "guiengine/engine.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include <IGUIEnvironment.h>

using namespace GUIEngine;

PlayerNameSpinner::PlayerNameSpinner(KartSelectionScreen* parent,
                                     const int player_id)
{
    m_player_id       = player_id;
    m_incorrect       = false;
    m_red_mark_widget = NULL;
    m_parent          = parent;
    setUseBackgroundColor();//except for multiplayer kart selection, this is false
    setSpinnerWidgetPlayerID(m_player_id);
}   // PlayerNameSpinner
// ------------------------------------------------------------------------
void PlayerNameSpinner::setID(const int m_player_id)
{
    PlayerNameSpinner::m_player_id = m_player_id;
    setSpinnerWidgetPlayerID(m_player_id);
} // setID
// ------------------------------------------------------------------------
/** Add a red mark on the spinner to mean "invalid choice" */
void PlayerNameSpinner::markAsIncorrect()
{
    if (m_incorrect) return; // already flagged as incorrect

    m_incorrect = true;

    irr::video::ITexture* texture = irr_driver->getTexture(FileManager::GUI_ICON,
                                                           "red_mark.png"   );
    const int mark_size = m_h;
    const int mark_x = m_w - mark_size*2;
    const int mark_y = 0;
    core::recti red_mark_area(mark_x, mark_y, mark_x + mark_size,
                              mark_y + mark_size);
    m_red_mark_widget = GUIEngine::getGUIEnv()->addImage( red_mark_area,
                        /* parent */ m_element );
    m_red_mark_widget->setImage(texture);
    m_red_mark_widget->setScaleImage(true);
    m_red_mark_widget->setTabStop(false);
    m_red_mark_widget->setUseAlphaChannel(true);
} // markAsIncorrect

// ------------------------------------------------------------------------
/** Remove any red mark set with 'markAsIncorrect' */
void PlayerNameSpinner::markAsCorrect()
{
    if (m_incorrect)
    {
        m_red_mark_widget->remove();
        m_red_mark_widget = NULL;
        m_incorrect = false;
    }
} // markAsCorrect

