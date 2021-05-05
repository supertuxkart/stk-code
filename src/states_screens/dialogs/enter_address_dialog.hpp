//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#ifndef HEADER_ENTER_ADDRESS_DIALOG_HPP
#define HEADER_ENTER_ADDRESS_DIALOG_HPP
#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "utils/cpp2011.hpp"
#include "network/server.hpp"
#include "guiengine/widgets/list_widget.hpp"
/**
 * \brief Dialog that shows up when user wants to enter server address
 * \ingroup states_screens
 */
class EnterAddressDialog : public GUIEngine::ModalDialog
{
private:
    class AddressListEntry{
    public:
        uint32_t lastly_connected;
        std::string address;
        bool operator<(const AddressListEntry& e1)
        {return lastly_connected<e1.lastly_connected;}
    };
    
    GUIEngine::LabelWidget* m_title;
    GUIEngine::TextBoxWidget* m_text_field;
    std::shared_ptr<Server>* m_entered_server;
    bool m_self_destroy;
    GUIEngine::ListWidget* m_list;
    // ------------------------------------------------------------------------
    void loadList();
    // ------------------------------------------------------------------------
    bool validate();
    // ------------------------------------------------------------------------
    void deleteOldest();
    // ------------------------------------------------------------------------
public:
    EnterAddressDialog(std::shared_ptr<Server>* enteredServer);
    // ------------------------------------------------------------------------
    ~EnterAddressDialog();
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void onEnterPressedInternal() OVERRIDE;
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& eventSource)
        OVERRIDE;
    // ------------------------------------------------------------------------
    GUIEngine::TextBoxWidget* getTextField() const     { return m_text_field; }
};
#endif