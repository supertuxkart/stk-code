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

#ifndef HEADER_ADDONS_PACK_HPP
#define HEADER_ADDONS_PACK_HPP

#include "guiengine/widgets.hpp"
#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"

class AddonsPackRequest;

/**
  * \ingroup states_screens
  */
class AddonsPack : public GUIEngine::ModalDialog
{
private:
    GUIEngine::ProgressBarWidget* m_progress;

    GUIEngine::LabelWidget* m_size;

    void stopDownload();

    /** A pointer to the download request, which gives access
     *  to the progress of a download. */
    std::shared_ptr<AddonsPackRequest> m_download_request;
    AddonsPack(const std::string& url);
    ~AddonsPack();
public:
    virtual GUIEngine::EventPropagation processEvent(const std::string& event_source) OVERRIDE;
    virtual void beforeAddingWidgets() OVERRIDE;
    virtual void init() OVERRIDE;
    void onUpdate(float delta) OVERRIDE;
    virtual bool onEscapePressed() OVERRIDE;
    static void install(const std::string& name);
    static void uninstall(const std::string& name, bool force_remove_skin = false);
    static void uninstallByName(const std::string& name, bool force_clear = false);
};   // DownloadAssets

#endif
