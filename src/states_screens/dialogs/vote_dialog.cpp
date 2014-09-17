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

#include "addons/addons_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------
/** Constructor.
 */
VoteDialog::VoteDialog(const std::string & addon_id)
         : ModalDialog(0.8f,0.6f), m_addon_id(addon_id)
{
    m_fetch_vote_request   = NULL;
    m_perform_vote_request = NULL;
    m_self_destroy         = false;
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

    m_fetch_vote_request = new XMLRequest();
    PlayerManager::setUserDetails(m_fetch_vote_request, "get-addon-vote");
    m_fetch_vote_request->addParameter("addonid", addon_id.substr(6));
    m_fetch_vote_request->queue();

    m_rating_widget->setDeactivated();
    m_cancel_widget->setDeactivated();
}    // VoteDialog

// -----------------------------------------------------------------------------
/** Destructor, frees the various requests.
 */
VoteDialog::~VoteDialog()
{
    delete m_fetch_vote_request;
    delete m_perform_vote_request;
}   // ~VoteDialog

// -----------------------------------------------------------------------------
/** When escape is pressed, trigger a self destroy.
 */
bool VoteDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}   // onEscapePressed

// ----------------------------------------------------------------------------
/** A request to the server, to perform a vote on an addon.
 *  \param rating the voted rating.
 */
void VoteDialog::sendVote()
{
    /** A vote request. The callback will update the addon manager with the
     *  new average. The VoteDialog polls this request till it is finished
     *  to inform the user about the new average.
     */
    class SetAddonVoteRequest : public XMLRequest
    {
        virtual void callback()
        {
            if (isSuccess())
            {
                std::string addon_id;
                float average;

                getXMLData()->get("addon-id", &addon_id);
                getXMLData()->get("new-average", &average);
                addons_manager->getAddon(Addon::createAddonId(addon_id))
                              ->setRating(average);
            }   // isSuccess
        }   // callbac
    public:
        SetAddonVoteRequest() : XMLRequest() {}
    };   // SetAddonVoteRequest
    // ------------------------------------------------------------------------

    m_perform_vote_request = new SetAddonVoteRequest();
    PlayerManager::setUserDetails(m_perform_vote_request, "set-addon-vote");
    m_perform_vote_request->addParameter("addonid", m_addon_id.substr(6));
    m_perform_vote_request->addParameter("rating", m_rating_widget->getRating());
    m_perform_vote_request->queue();

    m_rating_widget->setDeactivated();
    m_cancel_widget->setDeactivated();

}   // sendVote

// -----------------------------------------------------------------------------
/** Callback when a user event is triggered.
 *  \param event Information about the event that was triggered.
 */
GUIEngine::EventPropagation VoteDialog::processEvent(const std::string& event)
{

    if (event == m_rating_widget->m_properties[PROP_ID])
    {
        sendVote();
        return GUIEngine::EVENT_BLOCK;
    }

    if (event == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection =
            m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;

            return GUIEngine::EVENT_BLOCK;
        }
    }

    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------
/** Updates a potentiall still outstanding fetch vote request.
 */
void VoteDialog::updateFetchVote()
{
    // No request, nothing to do
    if (!m_fetch_vote_request) return;
    if (!m_fetch_vote_request->isDone())
    {
        // request still pending
        m_info_widget->setText(
            StringUtils::loadingDots(_("Fetching last vote")),
            false
        );

        return;
    }   // !isDone

    if (m_fetch_vote_request->isSuccess())
    {
        std::string voted("");

        m_info_widget->setDefaultColor();
        m_fetch_vote_request->getXMLData()->get("voted", &voted);

        if (voted == "yes")
        {
            float rating;
            m_fetch_vote_request->getXMLData()->get("rating", &rating);
            m_rating_widget->setRating(rating);
            m_info_widget->setText(_("You can adapt your previous rating by "
                                     "clicking the stars beneath."), false);
        }
        else if (voted == "no")
        {
            m_info_widget->setText(_("You have not yet voted for this addon. "
                                     "Select your desired rating by clicking "
                                     "the stars beneath"),              false);
        }
        m_cancel_widget->setActivated();
        m_rating_widget->setActivated();
    }   // isSuccess
    else
    {
        SFXManager::get()->quickSound("anvil");
        m_info_widget->setErrorColor();
        m_info_widget->setText(m_fetch_vote_request->getInfo(), false);
        m_cancel_widget->setActivated();
    }   // !isSuccess

    delete m_fetch_vote_request;
    m_fetch_vote_request = NULL;

}   // updateFetchVote

// -----------------------------------------------------------------------------
/** Called every frame. Checks if any of the pending requests are finished.
 *  \param dt Time step size.
 */
void VoteDialog::onUpdate(float dt)
{
    updateFetchVote();

    if(m_perform_vote_request != NULL)
    {
        if(m_perform_vote_request->isDone())
        {
            if(m_perform_vote_request->isSuccess())
            {
                m_info_widget->setDefaultColor();
                m_info_widget->setText(_("Vote successful! You can now close "
                                         "the window."),                false);
                m_cancel_widget->setActivated();
            }   // isSuccess
            else
            {
                SFXManager::get()->quickSound( "anvil" );
                m_info_widget->setErrorColor();
                m_info_widget->setText(m_perform_vote_request->getInfo(), false);
                m_cancel_widget->setActivated();
                m_rating_widget->setActivated();
            }   // !isSuccess
            delete m_perform_vote_request;
            m_perform_vote_request = NULL;
        }   // isDone
        else
        {
            m_info_widget->setText(StringUtils::loadingDots(_("Performing vote")),
                                   false);
        }   // !isDone
    }

    if (m_self_destroy)
        ModalDialog::dismiss();

}   // onUpdate
