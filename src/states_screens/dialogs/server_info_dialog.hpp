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


#ifndef HEADER_SERVER_INFO_DIALOG_HPP
#define HEADER_SERVER_INFO_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/types.hpp"

namespace GUIEngine
{
    class LabelWidget;
    class RibbonWidget;
    class IconButtonWidget;
    class TextBoxWidget;
}

#include <memory>
#include <irrString.h>

class Server;

/** \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class ServerInfoDialog : public GUIEngine::ModalDialog
{

private:
    bool m_self_destroy, m_join_server;

    const std::shared_ptr<Server> m_server;

    GUIEngine::RibbonWidget *m_options_widget;

    /** The join button. */
    GUIEngine::IconButtonWidget *m_join_widget;

    /** The cancel button. */
    GUIEngine::IconButtonWidget *m_cancel_widget;

    GUIEngine::IconButtonWidget *m_bookmark_widget;

    /** Specify server password if needed. */
    GUIEngine::TextBoxWidget* m_password;

    video::ITexture* m_bookmark_icon;

    video::ITexture* m_remove_icon;

    void updateBookmarkStatus(bool change_bookmark);
public:
    ServerInfoDialog(std::shared_ptr<Server> server);
    ~ServerInfoDialog();

    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    virtual void beforeAddingWidgets();

    virtual bool onEscapePressed();
    virtual void onUpdate(float dt);
    void requestJoin();
};   // class ServerInfoDialog

#endif
