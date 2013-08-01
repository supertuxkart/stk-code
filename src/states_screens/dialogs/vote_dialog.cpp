//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "states_screens/dialogs/vote_dialog.hpp"

#include <IGUIEnvironment.h>

#include "audio/sfx_manager.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "online/current_user.hpp"
#include "online/messages.hpp"



using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

VoteDialog::VoteDialog(const std::string & addon_id)
        : ModalDialog(0.8f,0.8f)
{
    m_self_destroy = false;
    loadFromFile("online/vote_dialog.stkgui");

    m_rating_widget = getWidget<RatingBarWidget>("rating");
    assert(m_rating_widget != NULL);
    m_rating_widget->setRating(0);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_submit_widget = getWidget<IconButtonWidget>("submit");
    assert(m_submit_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);


}

// -----------------------------------------------------------------------------
VoteDialog::~VoteDialog()
{
    //delete m_server_join_request;
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation VoteDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_rating_widget->m_properties[PROP_ID])
    {
        m_self_destroy = true;
        return GUIEngine::EVENT_BLOCK;
    }

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void VoteDialog::onUpdate(float dt)
{
    if (m_self_destroy)
        ModalDialog::dismiss();
}
