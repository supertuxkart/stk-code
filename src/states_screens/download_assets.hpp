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


#ifndef HEADER_DOWNLOAD_ASSETS_HPP
#define HEADER_DOWNLOAD_ASSETS_HPP

#include <atomic>
#include <string>
#include <thread>
#include <irrString.h>

#include "guiengine/screen.hpp"

namespace GUIEngine
{
    class CheckBoxWidget;
    class IconButtonWidget;
    class ProgressBarWidget;
}

namespace Online
{
    class HTTPRequest;
}

/**
  * \ingroup states_screens
  */
class DownloadAssets : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<DownloadAssets>
{
protected:
    DownloadAssets();

private:
    GUIEngine::ProgressBarWidget* m_progress;

    GUIEngine::CheckBoxWidget* m_all_tracks;

    GUIEngine::CheckBoxWidget* m_hd_textures;

    GUIEngine::IconButtonWidget* m_ok;

    std::atomic<bool> m_downloading_now;

    Online::HTTPRequest* m_download_request;

    std::thread m_download_thread;

    void updateDownloadSize();
public:
    friend class GUIEngine::ScreenSingleton<DownloadAssets>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE {}

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;
    virtual bool onEscapePressed() OVERRIDE;
    virtual void onUpdate(float dt) OVERRIDE;
    bool needDownloadAssets();
};   // class DownloadAssets

#endif
