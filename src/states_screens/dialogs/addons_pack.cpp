//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#ifndef SERVER_ONLY

#include "states_screens/dialogs/addons_pack.hpp"

#include "addons/addons_manager.hpp"
#include "addons/zip.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/protocols/client_lobby.hpp"
#include "online/http_request.hpp"
#include "race/grand_prix_manager.hpp"
#include "replay/replay_play.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/dialogs/addons_loading.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace Online;
using namespace irr::gui;

// ----------------------------------------------------------------------------
class AddonsPackRequest : public HTTPRequest
{
private:
    bool m_extraction_error;
    virtual void afterOperation()
    {
        Online::HTTPRequest::afterOperation();
        if (isCancelled())
            return;
        m_extraction_error = !file_manager->fileExists(getFileName());
        if (m_extraction_error)
            return;

        std::string tmp_extract = file_manager->getAddonsFile("tmp_extract");
        file_manager->removeDirectory(tmp_extract);
        file_manager->checkAndCreateDirectory(tmp_extract);
        m_extraction_error =
            !extract_zip(getFileName(), tmp_extract, true/*recursive*/);
    }
public:
    AddonsPackRequest(const std::string& url)
    : HTTPRequest(StringUtils::getBasename(url), /*priority*/5)
    {
        m_extraction_error = true;
        if (url.find("https://") != std::string::npos ||
            url.find("http://") != std::string::npos)
        {
            setURL(url);
            setDownloadAssetsRequest(true);
        }
        else
            m_filename.clear();
    }
    ~AddonsPackRequest()
    {
        const std::string& zip = getFileName();
        const std::string zip_part = zip + ".part";
        if (file_manager->fileExists(zip))
            file_manager->removeFile(zip);
        if (file_manager->fileExists(zip_part))
            file_manager->removeFile(zip_part);
        file_manager->removeDirectory(
            file_manager->getAddonsFile("tmp_extract"));
    }
    bool hadError() const { return hadDownloadError() || m_extraction_error; }
};   // DownloadAssetsRequest

// ----------------------------------------------------------------------------
/** Creates a modal dialog with given percentage of screen width and height
*/
AddonsPack::AddonsPack(const std::string& url) : ModalDialog(0.8f, 0.8f)
{
    loadFromFile("addons_loading.stkgui");
    getWidget<IconButtonWidget>("install")->setVisible(false);
    m_size = getWidget<LabelWidget>("size");
    m_size->setVisible(false);
    getWidget<BubbleWidget>("description")->setText(
        StringUtils::utf8ToWide(url));

    IconButtonWidget* icon = getWidget<IconButtonWidget>("icon");
    icon->setImage(file_manager->getAsset(FileManager::GUI_ICON, "logo.png"),
        IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);

    m_progress = getWidget<ProgressBarWidget>("progress");
    m_progress->setValue(0);
    m_progress->setVisible(true);
    GUIEngine::RibbonWidget* actions_ribbon =
            getWidget<GUIEngine::RibbonWidget>("actions");
    actions_ribbon->setVisible(false);
    m_download_request = std::make_shared<AddonsPackRequest>(url);
    m_download_request->queue();
}   // AddonsPack

// ----------------------------------------------------------------------------
AddonsPack::~AddonsPack()
{
    stopDownload();
}   // ~AddonsPack

// ----------------------------------------------------------------------------
void AddonsPack::beforeAddingWidgets()
{
    getWidget("uninstall")->setVisible(false);
}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
void AddonsPack::init()
{
    getWidget("rating")->setVisible(false);
}   // init

// ----------------------------------------------------------------------------
bool AddonsPack::onEscapePressed()
{
    ModalDialog::dismiss();
    return true;
}   // onEscapePressed

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation AddonsPack::processEvent(const std::string& event_source)
{
    GUIEngine::RibbonWidget* actions_ribbon =
            getWidget<GUIEngine::RibbonWidget>("actions");

    if (event_source == "actions")
    {
        const std::string& selection =
            actions_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == "back")
        {
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------
void AddonsPack::onUpdate(float delta)
{
    if (m_download_request)
    {
        if (!m_size->isVisible() && m_download_request->getTotalSize() > 0)
        {
            m_size->setVisible(true);
            core::stringw unit = StringUtils::getReadableFileSize(
                m_download_request->getTotalSize());
            core::stringw size = _("Size: %s", unit.c_str());
            m_size->setText(size, false);
        }

        float progress = m_download_request->getProgress();
        // Last 1% for unzipping
        m_progress->setValue(progress * 99.0f);
        if (progress < 0)
        {
            // Avoid displaying '-100%' in case of an error.
            m_progress->setVisible(false);
            dismiss();
            new MessageDialog(_("Sorry, downloading the add-on failed"));
            return;
        }
        else if (m_download_request->isDone())
        {
            // No sense to update state text, since it all
            // happens before the GUI is refrehsed.
            doInstall();
            return;
        }
    }   // if (m_progress->isVisible())
}   // onUpdate

// ----------------------------------------------------------------------------
/** This function is called when the user click on 'Back', 'Cancel' or press
 *  escape.
 **/
void AddonsPack::stopDownload()
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
void AddonsPack::doInstall()
{
    core::stringw msg;
    if (m_download_request->hadError())
    {
        // Reset the download buttons so user can redownload if needed
        // I18N: Shown when there is download error for assets download
        // in the first run
        msg = _("Failed to download assets, check your storage space or internet connection and try again later.");
    }

    if (!msg.empty())
    {
        dismiss();
        new MessageDialog(msg);
    }
    else
    {
        std::shared_ptr<AddonsPackRequest> request = m_download_request;
        dismiss();
        std::set<std::string> result;
        std::string tmp_extract = file_manager->getAddonsFile("tmp_extract");
        file_manager->listFiles(result, tmp_extract);
        tmp_extract += "/";
        bool addon_kart_installed = false;
        bool addon_track_installed = false;
        bool track_installed = false;
        for (auto& r : result)
        {
            if (r == ".." || r == ".")
                continue;
            std::string addon_id = Addon::createAddonId(r);
            // We assume the addons pack the user downloaded use the latest
            // revision from the stk-addons (if exists)
            if (file_manager->fileExists(tmp_extract + r + "/stkskin.xml"))
            {
                std::string skins = file_manager->getAddonsFile("skins");
                file_manager->checkAndCreateDirectoryP(skins);
                if (file_manager->isDirectory(skins + "/" + r))
                    file_manager->removeDirectory(skins + "/" + r);
                file_manager->moveDirectoryInto(tmp_extract + r, skins);
                // Skin is not supported in stk-addons atm
            }
            else if (file_manager->fileExists(tmp_extract + r + "/kart.xml"))
            {
                std::string karts = file_manager->getAddonsFile("karts");
                file_manager->checkAndCreateDirectoryP(karts);
                if (file_manager->isDirectory(karts + "/" + r))
                {
                    const KartProperties* prop =
                        kart_properties_manager->getKart(addon_id);
                    // If the model already exist, first remove the old kart
                    if (prop)
                        kart_properties_manager->removeKart(addon_id);
                    file_manager->removeDirectory(karts + "/" + r);
                }
                if (file_manager->moveDirectoryInto(tmp_extract + r, karts))
                {
                    kart_properties_manager->loadKart(karts + "/" + r);
                    Addon* addon = addons_manager->getAddon(addon_id);
                    if (addon)
                    {
                        addon_kart_installed = true;
                        addon->setInstalled(true);
                    }
                }
            }
            else if (file_manager->fileExists(tmp_extract + r + "/track.xml"))
            {
                track_installed = true;
                std::string tracks = file_manager->getAddonsFile("tracks");
                file_manager->checkAndCreateDirectoryP(tracks);
                if (file_manager->isDirectory(tracks + "/" + r))
                    file_manager->removeDirectory(tracks + "/" + r);
                if (file_manager->moveDirectoryInto(tmp_extract + r, tracks))
                {
                    Addon* addon = addons_manager->getAddon(addon_id);
                    if (addon)
                    {
                        addon_track_installed = true;
                        addon->setInstalled(true);
                    }
                }
            }
        }
        if (addon_kart_installed || addon_track_installed)
            addons_manager->saveInstalled();
        if (track_installed)
        {
            track_manager->loadTrackList();
            // Update the replay file list to use latest track pointer
            ReplayPlay::get()->loadAllReplayFile();
            delete grand_prix_manager;
            grand_prix_manager = new GrandPrixManager();
            grand_prix_manager->checkConsistency();
        }
        AddonsScreen* as = dynamic_cast<AddonsScreen*>(
            GUIEngine::getCurrentScreen());
        if (as)
            as->loadList();
        if (auto cl = LobbyProtocol::get<ClientLobby>())
            cl->updateAssetsToServer();
    }
}   // doInstall

// ----------------------------------------------------------------------------
void AddonsPack::install(const std::string& name)
{
    // Only install addon live in menu with no dialog opened
    if (StateManager::get()->getGameState() != GUIEngine::MENU ||
        ModalDialog::isADialogActive())
        return;

    NetworkingLobby* nl = dynamic_cast<NetworkingLobby*>(
        GUIEngine::getCurrentScreen());
    Addon* addon = addons_manager->getAddon(Addon::createAddonId(name));
    if (addon)
    {
        if (addon->isInstalled())
        {
            if (nl)
                nl->addMoreServerInfo(L"Addon already installed");
            return;
        }
        AddonsLoading* al = new AddonsLoading(addon->getId());
        al->tryInstall();
    }
    else if (StringUtils::startsWith(name, "http"))
    {
        // Assume it's addon pack url
        new AddonsPack(name);
    }
    else
    {
        if (nl)
            nl->addMoreServerInfo(L"Bad addon id");
    }
}   // install

// ----------------------------------------------------------------------------
void AddonsPack::uninstallByName(const std::string& name,
                                 bool force_clear)
{
    if (StateManager::get()->getGameState() != GUIEngine::MENU)
        return;
    NetworkingLobby* nl = dynamic_cast<NetworkingLobby*>(
        GUIEngine::getCurrentScreen());
    // force_clear is true when removing the existing folder when install new
    // addon, in this case we don't need more logging in lobby
    if (force_clear)
        nl = NULL;

    std::string addon_id = Addon::createAddonId(name);
    const KartProperties* prop =
        kart_properties_manager->getKart(addon_id);
    if (prop)
    {
        kart_properties_manager->removeKart(addon_id);
        if (nl)
            nl->addMoreServerInfo(L"Addon kart uninstalled");
        file_manager->removeDirectory(
            file_manager->getAddonsFile("karts/") + name);
        if (auto cl = LobbyProtocol::get<ClientLobby>())
            cl->updateAssetsToServer();
        return;
    }
    if (track_manager->getTrack(addon_id))
    {
        track_manager->removeTrack(addon_id);
        if (nl)
            nl->addMoreServerInfo(L"Addon track uninstalled");
        file_manager->removeDirectory(
            file_manager->getAddonsFile("tracks/") + name);
        if (auto cl = LobbyProtocol::get<ClientLobby>())
            cl->updateAssetsToServer();
        return;
    }
    std::string skin_folder = file_manager->getAddonsFile("skins/") + name;
    std::string skin_file = skin_folder + "/stkskin.xml";
    if (file_manager->fileExists(skin_file))
    {
        if (!force_clear &&
            addon_id == UserConfigParams::m_skin_file.c_str())
        {
            if (nl)
                nl->addMoreServerInfo(L"Can't remove current used skin");
        }
        else
        {
            file_manager->removeDirectory(skin_folder);
            if (nl)
                nl->addMoreServerInfo(L"Addon skin removed");
        }
        return;
    }
    if (nl)
        nl->addMoreServerInfo(L"Invalid addon");
}   // uninstallByName

// ----------------------------------------------------------------------------
void AddonsPack::uninstall(const std::string& name, bool force_remove_skin)
{
    // Only uninstall addon live in menu or no dialog opened
    if (StateManager::get()->getGameState() != GUIEngine::MENU ||
        ModalDialog::isADialogActive())
        return;

    uninstallByName(name, force_remove_skin);
    std::string addon_id = Addon::createAddonId(name);
    Addon* addon = addons_manager->getAddon(addon_id);
    if (addon && addon->isInstalled())
    {
        addon->setInstalled(false);
        addons_manager->saveInstalled();
    }
}   // uninstall

#endif
