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


#include "states_screens/dialogs/enter_address_dialog.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "network/socket_address.hpp"
#include "states_screens/state_manager.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include <IGUIEnvironment.h>
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "config/user_config.hpp"
// ------------------------------------------------------------------------
EnterAddressDialog::EnterAddressDialog(std::shared_ptr<Server>* enteredServer)
    : ModalDialog(0.95f,0.8f,GUIEngine::MODAL_DIALOG_LOCATION_BOTTOM),
    m_self_destroy(false)
{
    loadFromFile("enter_address_dialog.stkgui");
    m_text_field = getWidget<GUIEngine::TextBoxWidget>("textfield");
    m_title = getWidget<GUIEngine::LabelWidget>("title");
    m_title->setText(_("Enter the server address optionally followed by : and"
            " then port or select address from list."),false);
    m_entered_server=enteredServer;
    m_list = getWidget<GUIEngine::ListWidget>("list_history");
    
    loadList();
    
}   // EnterAddressDialog
// ------------------------------------------------------------------------
EnterAddressDialog::~EnterAddressDialog()
{
    m_text_field->getIrrlichtElement()->remove();
    m_text_field->clearListeners();
}   // ~EnterAddressDialog
// ------------------------------------------------------------------------
void EnterAddressDialog::onUpdate(float dt)
{
    if (m_self_destroy)
    {
        stringw name = m_text_field->getText().trim();
        GUIEngine::getGUIEnv()
            ->removeFocus(m_text_field->getIrrlichtElement());
        GUIEngine::getGUIEnv()->removeFocus(m_irrlicht_window);
        ModalDialog::dismiss();
    }
}   // onUpdate
// ------------------------------------------------------------------------
void EnterAddressDialog::onEnterPressedInternal()
{
    GUIEngine::IconButtonWidget* cancel_button = getWidget<GUIEngine::IconButtonWidget>("cancel");
    if (GUIEngine::isFocusedForPlayer(cancel_button, PLAYER_ID_GAME_MASTER))
    {
        std::string fake_event = "cancel";
        processEvent(fake_event);
        return;
    }

    if (!m_self_destroy && validate())
        m_self_destroy = true;
}   // onEnterPressedInternal
// ------------------------------------------------------------------------
GUIEngine::EventPropagation EnterAddressDialog::processEvent(const std::string& eventSource)
{
    if (eventSource=="buttons")
    {
        GUIEngine::RibbonWidget* buttons_ribbon =
            getWidget<GUIEngine::RibbonWidget>("buttons");
        const std::string& button =
	        buttons_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (button=="cancel")
        {
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (button=="ok")
        {
            if (!m_self_destroy && validate())
            {
                m_self_destroy = true;
                
                UserConfigParams::m_address_history[StringUtils::wideToUtf8(m_text_field->getText())] = (uint32_t)StkTime::getTimeSinceEpoch();
                if (UserConfigParams::m_address_history.size()>5)
                    deleteOldest();
            }
            
            return GUIEngine::EVENT_BLOCK;
        }
    }
    if (eventSource=="list_history")
    {
        m_text_field->setText(m_list->getSelectionLabel());
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
// ------------------------------------------------------------------------
bool EnterAddressDialog::validate()
{
    core::stringw addr_w = m_text_field->getText();
    std::string addr_u = StringUtils::wideToUtf8(addr_w);
    SocketAddress server_addr(addr_u);
    if (server_addr.getIP() == 0 && !server_addr.isIPv6())
    {
        core::stringw err = _("Invalid server address: %s.",
            addr_w);
        m_title->setText(err, true);
        return false;
    }
    SocketAddress ipv4_addr = server_addr;
    if (server_addr.isIPv6())
        ipv4_addr.setIP(0);
    auto server =
        std::make_shared<UserDefinedServer>(addr_w, ipv4_addr);
    if (server_addr.isIPv6())
    {
        server->setIPV6Address(server_addr);
        server->setIPV6Connection(true);
    }
    *m_entered_server = server;
    return true;
}   // validate
// ------------------------------------------------------------------------
void EnterAddressDialog::loadList()
{
    std::vector<AddressListEntry> entries;
    for(std::map<std::string,uint32_t>::iterator i=UserConfigParams::m_address_history.begin(); i!=UserConfigParams::m_address_history.end(); i++)
    {
        entries.push_back(AddressListEntry{i->second,i->first});
    }
    std::sort(entries.begin(),entries.end());
    for(int i=entries.size()-1; i>=0; i--)
    {
        m_list->addItem("list_item",
        std::vector<GUIEngine::ListWidget::ListCell>{GUIEngine::ListWidget::ListCell(irr::core::stringw(entries[i].address.c_str())),GUIEngine::ListWidget::ListCell(StringUtils::toWString(StkTime::toString(entries[i].lastly_connected)))});
    }
}   // loadList
void EnterAddressDialog::deleteOldest()
{
    uint32_t oldestTime=UserConfigParams::m_address_history.begin()->second;
    std::string key=UserConfigParams::m_address_history.begin()->first;
    for(std::map<std::string,uint32_t>::iterator i = UserConfigParams::m_address_history.begin(); i != UserConfigParams::m_address_history.end(); i++)
    {
        if (i->second<oldestTime){
            oldestTime = i->second;
            key = i->first;
        }
    }
    UserConfigParams::m_address_history.erase(key);
}   // deleteOldest