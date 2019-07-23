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

#include "states_screens/download_assets.hpp"
#include "addons/zip.hpp"
#include "config/stk_config.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "io/file_manager.hpp"
#include "online/http_request.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "main_loop.hpp"

#include <stdio.h>

using namespace GUIEngine;

// -----------------------------------------------------------------------------
DownloadAssets::DownloadAssets() : GUIEngine::Screen("download_assets.stkgui")
{
}   // OnlineLanScreen

// -----------------------------------------------------------------------------
void DownloadAssets::beforeAddingWidget()
{

}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void DownloadAssets::init()
{
    m_progress = getWidget<ProgressBarWidget>("progress");
    m_progress->setVisible(false);
    m_all_tracks = getWidget<CheckBoxWidget>("all-tracks");
    m_all_tracks->setActive(true);
    m_all_tracks->setVisible(true);
    m_all_tracks->setState(false);
    m_hd_textures = getWidget<CheckBoxWidget>("hd-textures");
    m_hd_textures->setActive(true);
    m_hd_textures->setVisible(true);
    m_hd_textures->setState(false);
    m_ok = getWidget<IconButtonWidget>("ok");
    m_ok->setActive(true);
    m_ok->setVisible(true);
    m_downloading_now = false;
    m_download_request = NULL;
    // Remove any previous left over .zip (and part)
    const std::string& dir = file_manager->getAddonsDir();
    if (file_manager->fileExists(dir + "nonfull-nonhd.zip"))
        file_manager->removeFile(dir + "nonfull-nonhd.zip");
    if (file_manager->fileExists(dir + "full-nonhd.zip"))
        file_manager->removeFile(dir + "full-nonhd.zip");
    if (file_manager->fileExists(dir + "nonfull-hd.zip"))
        file_manager->removeFile(dir + "nonfull-hd.zip");
    if (file_manager->fileExists(dir + "full-hd.zip"))
        file_manager->removeFile(dir + "full-hd.zip");
    if (file_manager->fileExists(dir + "nonfull-nonhd.zip.part"))
        file_manager->removeFile(dir + "nonfull-nonhd.zip.part");
    if (file_manager->fileExists(dir + "full-nonhd.zip.part"))
        file_manager->removeFile(dir + "full-nonhd.zip.part");
    if (file_manager->fileExists(dir + "nonfull-hd.zip.part"))
        file_manager->removeFile(dir + "nonfull-hd.zip.part");
    if (file_manager->fileExists(dir + "full-hd.zip.part"))
        file_manager->removeFile(dir + "full-hd.zip.part");
}   // init

// -----------------------------------------------------------------------------
void DownloadAssets::eventCallback(Widget* widget, const std::string& name,
                                   const int player_id)
{
    if (m_downloading_now)
        return;
    if (name == "buttons")
    {
        const std::string& button = getWidget<GUIEngine::RibbonWidget>("buttons")
            ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (button == "ok")
        {
            m_downloading_now = true;
            m_ok->setActive(false);
            m_progress->setValue(0);
            m_progress->setVisible(true);
            m_all_tracks->setActive(false);
            m_hd_textures->setActive(false);
            std::string download_url = stk_config->m_assets_download_url;
            std::string filename;

            if (m_all_tracks->getState())
                filename = "full";
            else
                filename = "nonfull";
            filename += "-";
            if (m_hd_textures->getState())
                filename += "hd";
            else
                filename += "nonhd";
            filename += ".zip";

            download_url += STK_VERSION;
            download_url += "/";
            download_url += filename;

            m_download_request = new Online::HTTPRequest(filename,
                true/*manage_memory*/);
            m_download_request->setDownloadAssetsRequest(true);
            m_download_request->setURL(download_url);
            m_download_thread = std::thread([this]()
                {
                    m_download_request->executeNow();
                    const std::string& zip = m_download_request->getFileName();
                    const std::string& dir =
                        file_manager->getSTKAssetsDownloadDir();
                    if (file_manager->fileExists(zip))
                    {
                        // Remove previous stk-assets version and create a new
                        // one
                        file_manager->removeDirectory(dir);
                        file_manager->checkAndCreateDirectory(dir);
                        if (extract_zip(zip, dir, true/*recursive*/))
                        {
                            std::string extract_ok =
                                dir + "/stk-assets." + STK_VERSION;
                            FILE* fp = fopen(extract_ok.c_str(), "wb");
                            if (!fp)
                            {
                                Log::error("FileUtils",
                                    "Failed to create extract ok file.");
                            }
                            else
                            {
                                fclose(fp);
                                file_manager->reinitAfterDownloadAssets();
                            }
                        }
                        file_manager->removeFile(zip);
                    }
                    m_downloading_now = false;
                });
        }
    }
}   // eventCallback

// ----------------------------------------------------------------------------
bool DownloadAssets::onEscapePressed()
{
    if (m_downloading_now)
        return false;
    return true;
}   // onEscapePressed

// ----------------------------------------------------------------------------
bool DownloadAssets::needDownloadAssets()
{
    const std::string& dir = file_manager->getSTKAssetsDownloadDir();
    if (dir.empty())
        return false;
    return !file_manager->fileExists(dir + "/stk-assets." + STK_VERSION);
}   // needDownloadAssets

// ----------------------------------------------------------------------------
void DownloadAssets::onUpdate(float dt)
{
    if (m_download_request)
        m_progress->setValue(m_download_request->getProgress() * 99.0f);
    if (m_download_thread.joinable())
    {
        if (m_downloading_now)
            return;
        if (!m_downloading_now)
        {
            m_download_thread.join();
            if (m_download_request)
            {
                delete m_download_request;
                m_download_request = NULL;
            }
        }
        if (!needDownloadAssets())
            main_loop->abort();
        else
        {
            // Reset the download buttons so user can redownload if needed
            // I18N: Shown when there is download error for assets download
            // in the first run
            core::stringw msg = _("Failed to download assets, please try again later.");
            MessageQueue::add(MessageQueue::MT_ERROR, msg);
            DownloadAssets::init();
        }
    }
}   // onUpdate
