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
#include "guiengine/message_queue.hpp"
#include "network/protocols/client_lobby.hpp"
#include "online/http_request.hpp"
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
    bool m_background_download;
    virtual void afterOperation() OVERRIDE
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
        if (!m_extraction_error && m_background_download)
        {
            core::stringw msg = _("Background download completed.");
            MessageQueue::add(MessageQueue::MT_GENERIC, msg);
        }
    }
public:
    AddonsPackRequest(const std::string& url)
    : HTTPRequest(StringUtils::getBasename(url), /*priority*/5)
    {
        m_extraction_error = true;
        m_background_download = false;
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
    void backgroundDownload()                 { m_background_download = true; }
    virtual bool hadDownloadError() const OVERRIDE
              { return HTTPRequest::hadDownloadError() || m_extraction_error; }
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
    actions_ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    actions_ribbon->select("back", PLAYER_ID_GAME_MASTER);
    icon = getWidget<IconButtonWidget>("install");
    icon->setImage(file_manager->getAsset(FileManager::GUI_ICON,
        "blue_plus.png"), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    icon->setLabel(_("Background download"));
    icon->setVisible(true);
    getWidget("uninstall")->setVisible(false);
    icon = getWidget<IconButtonWidget>("back");
    icon->setImage(file_manager->getAsset(FileManager::GUI_ICON, "remove.png"),
        IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    icon->setLabel(_("Cancel"));
    if (ClientLobby::startedDownloadAddonsPack())
    {
        core::stringw msg = _("Background download has already started.");
        MessageQueue::add(MessageQueue::MT_ERROR, msg);
    }
    else
    {
        m_download_request = std::make_shared<AddonsPackRequest>(url);
        ClientLobby::downloadAddonsPack(m_download_request);
    }
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
        if (selection == "install")
        {
            if (m_download_request)
                m_download_request->backgroundDownload();
            m_download_request = nullptr;
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "back")
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
        else if (m_download_request->isDone() ||
            m_download_request->isCancelled())
        {
            // No sense to update state text, since it all
            // happens before the GUI is refrehsed.
            dismiss();
            return;
        }
    }
    else
        dismiss();
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
        if (!m_download_request->isDone())
            m_download_request->cancel();
        m_download_request = nullptr;
    }
}   // startDownload

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
        if (addon->isInstalled() &&
            addon->getRevision() == addon->getInstalledRevision())
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
