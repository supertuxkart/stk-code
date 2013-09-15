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
        : ModalDialog(0.8f,0.6f), m_addon_id(addon_id)
{
    m_fetch_vote_request = NULL;
    m_perform_vote_request = NULL;
    m_self_destroy = false;
    loadFromFile("online/vote_dialog.stkgui");

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_rating_widget = getWidget<RatingBarWidget>("rating");
    assert(m_rating_widget != NULL);
    m_rating_widget->setRating(0);
    m_rating_widget->allowVoting();
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    m_fetch_vote_request = CurrentUser::get()->requestGetAddonVote(m_addon_id);

    m_rating_widget->setDeactivated();
    m_cancel_widget->setDeactivated();
}

// -----------------------------------------------------------------------------
VoteDialog::~VoteDialog()
{
    delete m_fetch_vote_request;
    delete m_perform_vote_request;
}

// -----------------------------------------------------------------------------

bool VoteDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation VoteDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_rating_widget->m_properties[PROP_ID])
    {
        m_perform_vote_request = CurrentUser::get()->requestSetAddonVote(m_addon_id, m_rating_widget->getRating());
        m_rating_widget->setDeactivated();
        m_cancel_widget->setDeactivated();
        return GUIEngine::EVENT_BLOCK;
    }

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void VoteDialog::onUpdate(float dt)
{
    if(m_fetch_vote_request != NULL)
    {
        if(m_fetch_vote_request->isDone())
        {
            if(m_fetch_vote_request->isSuccess())
            {
                m_info_widget->setDefaultColor();
                std::string voted("");
                m_fetch_vote_request->getResult()->get("voted", &voted);
                if(voted == "yes")
                {
                    float rating;
                    m_fetch_vote_request->getResult()->get("rating", &rating);
                    m_rating_widget->setRating(rating);
                    m_info_widget->setText(_("You can adapt your previous rating by clicking the stars beneath."), false);
                }
                else if(voted == "no")
                {
                    m_info_widget->setText(_("You have not yet voted for this addon. Select your desired rating by clicking the stars beneath"), false);
                }
                m_cancel_widget->setActivated();
                m_rating_widget->setActivated();
            }
            else
            {
                sfx_manager->quickSound( "anvil" );
                m_info_widget->setErrorColor();
                m_info_widget->setText(m_fetch_vote_request->getInfo(), false);
                m_cancel_widget->setActivated();
            }
            delete m_fetch_vote_request;
            m_fetch_vote_request = NULL;
        }
        else
        {
            m_info_widget->setText(irr::core::stringw(_("Fetching last vote")) + Messages::loadingDots(), false);
        }
    }
    if(m_perform_vote_request != NULL)
    {
        if(m_perform_vote_request->isDone())
        {
            if(m_perform_vote_request->isSuccess())
            {
                m_info_widget->setDefaultColor();
                m_info_widget->setText(_("Vote successful! You can now close the window."), false);
                m_cancel_widget->setActivated();
            }
            else
            {
                sfx_manager->quickSound( "anvil" );
                m_info_widget->setErrorColor();
                m_info_widget->setText(m_perform_vote_request->getInfo(), false);
                m_cancel_widget->setActivated();
                m_rating_widget->setActivated();
            }
            delete m_perform_vote_request;
            m_perform_vote_request = NULL;
        }
        else
        {
            m_info_widget->setText(irr::core::stringw(_("Performing vote")) + Messages::loadingDots(), false);
        }
    }
    if (m_self_destroy)
        ModalDialog::dismiss();
}
