//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_SERVER_SELECTION_HPP
#define HEADER_SERVER_SELECTION_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

#include <memory>

namespace Online { class XMLRequest; }

namespace GUIEngine
{
    class CheckBoxWidget;
    class IconButtonWidget;
    class LabelWidget;
}

namespace irr
{
    namespace gui
    {
        class STKModifiedSpriteBank;
    }
}

class Server;
struct ServerList;

/**
  * \brief ServerSelection
  * \ingroup states_screens
  */
class ServerSelection :  public GUIEngine::Screen,
                         public GUIEngine::ScreenSingleton<ServerSelection>,
                         public GUIEngine::IListWidgetHeaderListener,
                         public GUIEngine::ITextBoxWidgetListener
{
    friend class GUIEngine::ScreenSingleton<ServerSelection>;

private:
    ServerSelection();
    ~ServerSelection();

    std::vector<std::shared_ptr<Server> > m_servers;

    GUIEngine::CheckBoxWidget* m_private_server;
    GUIEngine::CheckBoxWidget* m_ipv6;
    GUIEngine::IconButtonWidget* m_reload_widget;
    GUIEngine::IconButtonWidget* m_bookmark_widget;
    video::ITexture* m_bookmark_icon;
    video::ITexture* m_global_icon;
    GUIEngine::LabelWidget* m_update_status;
    GUIEngine::ListWidget* m_server_list_widget;
    GUIEngine::TextBoxWidget* m_searcher;
    irr::gui::STKModifiedSpriteBank* m_icon_bank;

    /** \brief To check (and set) if sort order is descending **/
    bool m_sort_desc;

    bool m_sort_default;

    int m_current_column;

    bool m_refreshing_server;
    
    float m_refresh_timer;

    /** Load the servers into the main list.*/
    void loadList();

    void updateHeader();

    void refresh();

    bool m_ipv6_only_without_nat64;
    bool m_ip_warning_shown;
    int64_t m_last_load_time;
    std::shared_ptr<ServerList> m_server_list;
public:
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void onColumnClicked(int column_id, bool sort_desc,
                                 bool sort_default) OVERRIDE;

    virtual void init() OVERRIDE;

    virtual void tearDown() OVERRIDE;

    virtual void unloaded() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;

    virtual void onTextUpdated() OVERRIDE             { copyFromServerList(); }

    virtual bool onEnterPressed(const irr::core::stringw& text) OVERRIDE
                                                              { return false; }

    void copyFromServerList();

    GUIEngine::ListWidget* getServerList() const
                                               { return m_server_list_widget; }
};   // ServerSelection

#endif
