//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon, Joerg Henrichs
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


#include "states_screens/dialogs/addons_loading.hpp"

#include "audio/sfx_manager.hpp"
#include "addons/addons_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "network/protocols/client_lobby.hpp"
#include "online/request_manager.hpp"
#include "online/xml_request.hpp"
#include "race/grand_prix_manager.hpp"
#include "replay/replay_play.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/dialogs/vote_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ITexture.h>

using namespace GUIEngine;
using namespace Online;
using namespace irr::gui;

// ----------------------------------------------------------------------------
/** Creates a modal dialog with given percentage of screen width and height
*/

AddonsLoading::AddonsLoading(const std::string &id)
             : ModalDialog(0.8f, 0.9f)
#ifndef SERVER_ONLY
             , m_addon(*(addons_manager->getAddon(id)) )
#endif
{
    m_message_shown    = false;
    m_icon_shown       = false;
#ifdef SERVER_ONLY
    m_icon_downloaded  = std::make_shared<bool>(false);
#else
    m_icon_downloaded  = std::make_shared<bool>(m_addon.iconReady());
    if (*m_icon_downloaded == false)
        addons_manager->downloadIconForAddon(id, m_icon_downloaded);
#endif
    loadFromFile("addons_loading.stkgui");

    m_icon             = getWidget<IconButtonWidget> ("icon"    );
    m_progress         = getWidget<ProgressBarWidget>("progress");
    m_install_button   = getWidget<IconButtonWidget> ("install" );
    m_back_button      = getWidget<IconButtonWidget> ("back"  );
    
    RibbonWidget* actions = getWidget<RibbonWidget>("actions");
    actions->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    actions->select("back", PLAYER_ID_GAME_MASTER);

    if(m_progress)
        m_progress->setVisible(false);

}   // AddonsLoading

// ----------------------------------------------------------------------------
/** Destructor.
 */
AddonsLoading::~AddonsLoading()
{
    stopDownload();
    // Select the last selected item in the addons_screen, so that
    // users can keep on installing from the last selected item.
    // This dialog can be called in network lobby screen atm for live addon
    // install
    AddonsScreen* as = dynamic_cast<AddonsScreen*>(
        GUIEngine::getCurrentScreen());
    if (as)
        as->setLastSelected();
}   // AddonsLoading

// ----------------------------------------------------------------------------

void AddonsLoading::beforeAddingWidgets()
{
#ifndef SERVER_ONLY
    /* Init the icon here to be able to load a single image*/
    m_icon             = getWidget<IconButtonWidget> ("icon"    );
    m_progress         = getWidget<ProgressBarWidget>("progress");
    m_back_button      = getWidget<IconButtonWidget> ("back"    );

    RibbonWidget* r = getWidget<RibbonWidget>("actions");
    RatingBarWidget* rating = getWidget<RatingBarWidget>("rating");

    if (m_addon.isInstalled())
    {
        /* Turn "Install" button into "Update" if allowed to access the internet
         * and not in an errored state
         */
        if (m_addon.needsUpdate() && !addons_manager->wasError()
            && UserConfigParams::m_internet_status==RequestManager::IPERM_ALLOWED)
            getWidget<IconButtonWidget> ("install")->setText( _("Update") );
        else
            r->removeChildNamed("install");
    }
    else
    {
        r->removeChildNamed("uninstall");
    }

    getWidget<LabelWidget>("name")->setText(m_addon.getName().c_str(), false);
    getWidget<BubbleWidget>("description")
        ->setText(m_addon.getDescription().c_str());
    core::stringw revision = _("Version: %d", m_addon.getRevision());
    getWidget<LabelWidget>("revision")->setText(revision, false);
    rating->setRating(m_addon.getRating());
    rating->setStarNumber(3);

    // Display flags for this addon
    // ============================
    std::vector<core::stringw> l;
    if(UserConfigParams::m_artist_debug_mode)
    {
        // In non artist-debug-mode only approved items will be shown anyway,
        // but give even tester an idea about the status:
        if (!m_addon.testStatus(Addon::AS_APPROVED))
            l.push_back("NOT APPROVED");

        // Note that an approved addon should never have alpha, beta, or
        // RC status - and only one of those should be used
        if(m_addon.testStatus(Addon::AS_ALPHA))
            l.push_back("alpha");
        else if(m_addon.testStatus(Addon::AS_BETA))
            l.push_back("beta");
        else if(m_addon.testStatus(Addon::AS_RC))
            l.push_back("RC");

        // Don't displat those for now, they're more confusing than helpful for the average player
        //if(m_addon.testStatus(Addon::AS_BAD_DIM))
        //    l.push_back("bad-texture");
        //if(!m_addon.testStatus(Addon::AS_DFSG))
        //    l.push_back("non-DFSG");
    }
    if(m_addon.testStatus(Addon::AS_FEATURED))
        l.push_back(_("featured"));

    GUIEngine::LabelWidget *flags = getWidget<LabelWidget>("flags");
    if(flags)
    {
        core::stringw s1("");
        for(unsigned int i=0; i<l.size(); i++)
        {
            s1+=l[i];
            // if it's not the last item, add a ",".
            // Don't test for l.size()-1 - since this is unsigned!
            if(i+1<l.size())
                s1+=", ";
        }
        flags->setText(s1, false);
    }

    // Display the size
    // ================
    core::stringw unit = StringUtils::getReadableFileSize(m_addon.getSize());
    core::stringw size = _("Size: %s", unit.c_str());
    getWidget<LabelWidget>("size")->setText(size, false);
#endif
}   // AddonsLoading

// ----------------------------------------------------------------------------

void AddonsLoading::init()
{
    GUIEngine::LabelWidget* flags = getWidget<LabelWidget>("flags");
    if (flags)
    {
        flags->getIrrlichtElement<IGUIStaticText>()->setOverrideFont(GUIEngine::getSmallFont());
    }
}   // init

// ----------------------------------------------------------------------------
bool AddonsLoading::onEscapePressed()
{
    ModalDialog::dismiss();
    return true;
}   // onEscapePressed

// ----------------------------------------------------------------------------
void AddonsLoading::tryInstall()
{
#ifndef SERVER_ONLY
    // Only display the progress bar etc. if we are not uninstalling an addon.
    if (!m_addon.isInstalled() || m_addon.needsUpdate())
    {
        m_progress->setValue(0);
        m_progress->setVisible(true);
        GUIEngine::RibbonWidget* actions_ribbon =
            getWidget<GUIEngine::RibbonWidget>("actions");
        actions_ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        actions_ribbon->select("back", PLAYER_ID_GAME_MASTER);
        getWidget("install")->setVisible(false);
        m_back_button->setImage(file_manager->getAsset(FileManager::GUI_ICON,
            "remove.png"), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        // Change the 'back' button into a 'cancel' button.
        m_back_button->setLabel(_("Cancel"));
        startDownload();
    }
#endif
}   // tryInstall

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation AddonsLoading::processEvent(const std::string& event_source)
{
#ifndef SERVER_ONLY
    GUIEngine::RibbonWidget* actions_ribbon =
            getWidget<GUIEngine::RibbonWidget>("actions");

    if (event_source == "actions")
    {
        const std::string& selection =
                actions_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        if(selection == "back")
        {
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == "install")
        {
            tryInstall();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "uninstall")
        {
            doUninstall();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "vote")
        {
            voteClicked();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    else if (event_source == "rating")
    {
        voteClicked();
        return GUIEngine::EVENT_BLOCK;
    }
#endif
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------
void AddonsLoading::voteClicked()
{
#ifndef SERVER_ONLY
    if (PlayerManager::isCurrentLoggedIn())
    {
        // We need to keep a copy of the addon id, since dismiss() will
        // delete this object (and the copy of the addon).
        std::string addon_id = m_addon.getId();
        dismiss();
        new VoteDialog(addon_id);
    }
    else
    {
        SFXManager::get()->quickSound("anvil");
        if (!m_message_shown)
        {
            MessageQueue::add(MessageQueue::MT_ERROR,
                _("You must be logged in to rate this addon."));
            m_message_shown = true;
        }
    }
#endif
}   // voteClicked

// ----------------------------------------------------------------------------
void AddonsLoading::onUpdate(float delta)
{
#ifndef SERVER_ONLY
    if(m_progress->isVisible())
    {
        float progress = m_download_request->getProgress();
        m_progress->setValue(progress*100.0f);
        if(progress<0)
        {
            // Avoid displaying '-100%' in case of an error.
            m_progress->setVisible(false);
            dismiss();
            new MessageDialog( _("Sorry, downloading the add-on failed"));
            return;
        }
        else if(m_download_request->isDone())
        {
            m_back_button->setLabel(_("Back"));
            // No sense to update state text, since it all
            // happens before the GUI is refrehsed.
            doInstall();
            return;
        }
    }   // if(m_progress->isVisible())

    // See if the icon is loaded (but not yet displayed)
    if (!m_icon_shown && *m_icon_downloaded == true)
    {
        const std::string icon = "icons/"+m_addon.getIconBasename();
        m_icon->setImage( file_manager->getAddonsFile(icon).c_str(),
                          IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE  );
        // Check if there was an error displaying the icon. If so, the icon
        // file is (likely) corrupt, and the file needs to be downloaded again.
        std::string s = m_icon->getTexture()->getName().getPath().c_str();
        if(StringUtils::getBasename(s)!=StringUtils::getBasename(icon))
        {
            m_addon.deleteInvalidIconFile();
        }
        m_icon_shown = true;
    }
#endif
}   // onUpdate

// ----------------------------------------------------------------------------
/** This function is called when the user click on 'Install', 'Uninstall', or
 *  'Update'.
 **/
void AddonsLoading::startDownload()
{
#ifndef SERVER_ONLY
    std::string save   = "tmp/"
                       + StringUtils::getBasename(m_addon.getZipFileName());
    m_download_request = std::make_shared<Online::HTTPRequest>(
        save, /*priority*/5);
    m_download_request->setURL(m_addon.getZipFileName());
    m_download_request->queue();
#endif
}   // startDownload

// ----------------------------------------------------------------------------
/** This function is called when the user click on 'Back', 'Cancel' or press
 *  escape.
 **/
void AddonsLoading::stopDownload()
{
    // Cancel a download only if we are installing/upgrading one
    // (and not uninstalling an installed one):
    if (m_download_request)
    {
        m_download_request->cancel();
        m_download_request = nullptr;
    }
}   // startDownload


// ----------------------------------------------------------------------------
/** Called when the asynchronous download of the addon finished.
 */
void AddonsLoading::doInstall()
{
#ifndef SERVER_ONLY
    m_download_request = nullptr;

    assert(!m_addon.isInstalled() || m_addon.needsUpdate());
    bool error = !addons_manager->install(m_addon);
    if(error)
    {
        const core::stringw &name = m_addon.getName();
        core::stringw msg = _("Problems installing the addon '%s'.", name);
        getWidget<BubbleWidget>("description")->setText(msg.c_str());
    }

    if(error)
    {
        m_progress->setVisible(false);

        RibbonWidget* r = getWidget<RibbonWidget>("actions");
        r->setVisible(true);

        m_install_button->setLabel(_("Try again"));
    }
    else
    {
        // The list of the addon screen needs to be updated to correctly
        // display the newly (un)installed addon.
        AddonsScreen* as = dynamic_cast<AddonsScreen*>(
            GUIEngine::getCurrentScreen());
        if (as)
            as->loadList();
        dismiss();
    }

    track_manager->loadTrackList();
    // Update the replay file list to use latest track pointer
    ReplayPlay::get()->loadAllReplayFile();
    delete grand_prix_manager;
    grand_prix_manager = new GrandPrixManager();
    grand_prix_manager->checkConsistency();

    if (auto cl = LobbyProtocol::get<ClientLobby>())
        cl->updateAssetsToServer();
#endif
}   // doInstall

// ----------------------------------------------------------------------------

void AddonsLoading::doUninstall()
{
#ifndef SERVER_ONLY
    if (m_download_request)
        m_download_request->cancel();
    m_download_request = nullptr;
    bool error = !addons_manager->uninstall(m_addon);
    if(error)
    {
        Log::warn("Addons", "Directory '%s' can not be removed.",
                  m_addon.getDataDir().c_str());
        Log::warn("Addons", "Please remove this directory manually.");
        const core::stringw &name = m_addon.getName();
        core::stringw msg = _("Problems removing the addon '%s'.", name);
        getWidget<BubbleWidget>("description")->setText(msg.c_str());
    }

    if(error)
    {
        m_progress->setVisible(false);

        RibbonWidget* r = getWidget<RibbonWidget>("actions");
        r->setVisible(true);
        IconButtonWidget *u = getWidget<IconButtonWidget> ("uninstall" );
        u->setLabel(_("Try again"));
    }
    else
    {
        // The list of the addon screen needs to be updated to correctly
        // display the newly (un)installed addon.
        AddonsScreen* as = dynamic_cast<AddonsScreen*>(
            GUIEngine::getCurrentScreen());
        if (as)
            as->loadList();
        dismiss();
    }
    // Update the replay file list to use latest track pointer
    ReplayPlay::get()->loadAllReplayFile();
    delete grand_prix_manager;
    grand_prix_manager = new GrandPrixManager();
    grand_prix_manager->checkConsistency();

    if (auto cl = LobbyProtocol::get<ClientLobby>())
        cl->updateAssetsToServer();
#endif
}   // doUninstall
