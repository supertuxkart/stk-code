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

#ifdef MOBILE_STK

#include "utils/extract_mobile_assets.hpp"
#include "addons/zip.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "race/grand_prix_manager.hpp"
#include "replay/replay_play.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------
bool ExtractMobileAssets::hasFullAssets()
{
    const std::string& dir = file_manager->getSTKAssetsDownloadDir();
    if (dir.empty())
        return false;
    return file_manager->fileExists(dir + "stk-assets." + STK_VERSION);
}   // hasFullAssets

// ----------------------------------------------------------------------------
bool ExtractMobileAssets::extract(const std::string& zip_file,
                                  const std::string& dst)
{
    if (!file_manager->fileExists(zip_file))
        return false;

    bool succeed = false;
    // Remove previous stk-assets version and create a new one
    file_manager->removeDirectory(dst);
    file_manager->checkAndCreateDirectory(dst);
    if (extract_zip(zip_file, dst, true/*recursive*/))
    {
        std::string extract_ok = dst + "stk-assets." + STK_VERSION;
        FILE* fp = fopen(extract_ok.c_str(), "wb");
        if (!fp)
        {
            Log::error("ExtractMobileAssets",
                "Failed to create extract ok file.");
        }
        else
        {
            fclose(fp);
            succeed = true;
        }
    }
    file_manager->removeFile(zip_file);
    return succeed;
}   // extract

// ----------------------------------------------------------------------------
void ExtractMobileAssets::reinit()
{
    file_manager->reinitAfterDownloadAssets();
    irr_driver->sameRestart();
    track_manager->loadTrackList();
    // Update the replay file list to use latest track pointer
    ReplayPlay::get()->loadAllReplayFile();

    delete grand_prix_manager;
    grand_prix_manager = new GrandPrixManager();
    grand_prix_manager->checkConsistency();
}   // reinit

// ----------------------------------------------------------------------------
void ExtractMobileAssets::uninstall()
{
    // Remove the version file in stk-assets folder first, so if it crashes /
    // restarted by mobile it will auto discard downloaded assets
    file_manager->removeFile(file_manager->getSTKAssetsDownloadDir() +
        "stk-assets." + STK_VERSION);
    file_manager->removeDirectory(file_manager->getSTKAssetsDownloadDir());
    reinit();
}   // uninstall

#endif
