//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin, Joerg Henrichs
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

#include "states_screens/addons_screen.hpp"

#include <iostream>

#include "addons/addons_manager.hpp"
#include "addons/network_http.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/addons_loading.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/ptr_vector.hpp"

DEFINE_SCREEN_SINGLETON( AddonsScreen );

// ------------------------------------------------------------------------------------------------------

AddonsScreen::AddonsScreen() : Screen("addons_screen.stkgui")
{
    m_selected_index = -1;
}   // AddonsScreen

// ------------------------------------------------------------------------------------------------------

void AddonsScreen::loadedFromFile()
{
    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/package.png"         );
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/no-package.png"      );
    video::ITexture* icon3 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/package-update.png"  );
    video::ITexture* icon4 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/package-featured.png");
    video::ITexture* icon5 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/no-package-featured.png");
    video::ITexture* icon6 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/loading.png");
    
    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv());
    m_icon_installed     = m_icon_bank->addTextureAsSprite(icon1);
    m_icon_not_installed = m_icon_bank->addTextureAsSprite(icon2);
    m_icon_bank->addTextureAsSprite(icon4);
    m_icon_bank->addTextureAsSprite(icon5);
    m_icon_loading = m_icon_bank->addTextureAsSprite(icon6);
    m_icon_needs_update  = m_icon_bank->addTextureAsSprite(icon3);
    
    GUIEngine::ListWidget* w_list = getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->setColumnListener(this);
}   // loadedFromFile


// ----------------------------------------------------------------------------

void AddonsScreen::beforeAddingWidget()
{
    GUIEngine::ListWidget* w_list = getWidget<GUIEngine::ListWidget>("list_addons");
    assert(w_list != NULL);
    w_list->clearColumns();
    w_list->addColumn( _("Add-on name"), 2 );
    w_list->addColumn( _("Updated date"), 1 );
}

// ----------------------------------------------------------------------------

void AddonsScreen::init()
{
    Screen::init();
    
    m_reloading = false;
    
	getWidget<GUIEngine::RibbonWidget>("category")->setDeactivated();

    GUIEngine::getFont()->setTabStop(0.66f);
    
    if(UserConfigParams::logAddons())
        std::cout << "[addons] Using directory <" + file_manager->getAddonsDir() 
              << ">\n";
    
    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    
    m_icon_height = getHeight()/8.0f;
    m_icon_bank->setScale(m_icon_height/128.0f); // 128 is the height of the image file
    w_list->setIcons(m_icon_bank, (int)(m_icon_height));
    
    m_type = "kart";

    // Set the default sort order
    Addon::setSortOrder(Addon::SO_DEFAULT);
    loadList();
}   // init

// ----------------------------------------------------------------------------

void AddonsScreen::tearDown()
{
    // return tab stop to the center when leaving this screen!!
    GUIEngine::getFont()->setTabStop(0.5f);
}

// ----------------------------------------------------------------------------
/** Loads the list of all addons of the given type. The gui element will be
 *  updated.
 *  \param type Must be 'kart' or 'track'.
 */
void AddonsScreen::loadList()
{
    // First create a list of sorted entries
    PtrVector<const Addon, REF> sorted_list;
    for(unsigned int i=0; i<addons_manager->getNumAddons(); i++)
    {
        const Addon &addon = addons_manager->getAddon(i);
        // Ignore addons of a different type
        if(addon.getType()!=m_type) continue;
        // Ignore invisible addons
        if(addon.testStatus(Addon::AS_INVISIBLE))
            continue;
        if(!UserConfigParams::m_artist_debug_mode &&
            !addon.testStatus(Addon::AS_APPROVED)    )
            continue;
        sorted_list.push_back(&addon);
    }
    sorted_list.insertionSort(/*start=*/0);

    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->clear();

    for(int i=0; i<sorted_list.size(); i++)
    {
        const Addon *addon = &(sorted_list[i]);
        // Ignore addons of a different type
        if(addon->getType()!=m_type) continue;
        if(!UserConfigParams::m_artist_debug_mode &&
            !addon->testStatus(Addon::AS_APPROVED)    )
            continue;
        
        // Get the right icon to display
        int icon;
        if(addon->isInstalled())
            icon = addon->needsUpdate() ? m_icon_needs_update 
                                        : m_icon_installed;
	    else
        	icon = m_icon_not_installed;

        core::stringw s;
        if (addon->getDesigner().size()==0)
            s = (addon->getName()+L"\t"+core::stringc(addon->getDateAsString().c_str())).c_str();
        
        gui::IGUIFont* font = GUIEngine::getFont();
        
        // first column is 0.666% of the list's width. and icon width == icon height.
        const unsigned int available_width = (int)(w_list->m_w*0.6666f - m_icon_height);
        if (font->getDimension(s.c_str()).Width > available_width)
        {
            s = s.subString(0, int(AddonsScreen::getWidth()*0.018)+20);
            s.append("...");
        }
        else
        {
            if (addon->getDesigner().size() == 0)
            {
                s = addon->getName();
            }
            else
            {
                //I18N: as in: The Old Island by Johannes Sjolund
                s = _C("addons", "%s by %s", addon->getName().c_str(),addon->getDesigner().c_str());
            }
            
            // check if text is too long to fit
            if (font->getDimension(s.c_str()).Width >  available_width)
            {
                // start by splitting on 2 lines
                
                //I18N: as in: The Old Island by Johannes Sjolund
                s = _("%s\nby %s",addon->getName().c_str(),addon->getDesigner().c_str());
                
                core::stringw final_string;
                
                // then check if each line is now short enough.
                std::vector<irr::core::stringw> lines = StringUtils::split(s, '\n');
                for (unsigned int n=0; n<lines.size(); n++)
                {
                    if (font->getDimension(lines[n].c_str()).Width > available_width)
                    {
                        // arg, still too long! cut the text so that it fits.
                        core::stringw line = lines[n];
                        
                        // leave a margin of 14 pixels to account for the "..." that will be appended
                        int split_at = font->getCharacterFromPos(line.c_str(), available_width - 14);
                        line = line.subString(0, split_at);
                        line.append("...");
                        if (final_string.size() > 0) final_string.append("\n");
                        final_string.append(line);
                    }
                    else
                    {
                        if (final_string.size() > 0) final_string.append("\n");
                        final_string.append(lines[n]);
                    }
                }
                
                s = final_string;
            }
            s.append("\t");
            s.append(addon->getDateAsString().c_str());
        }
        
        // we have no icon for featured+updateme, so if an add-on is updatable forget about the featured icon
        if (addon->testStatus(Addon::AS_FEATURED) && icon != m_icon_needs_update)
        {
            icon += 2;
        }
        
        w_list->addItem(addon->getId(), s.c_str(), icon);

        // Highlight if it's not approved in artists debug mode.
        if(UserConfigParams::m_artist_debug_mode && !addon->testStatus(Addon::AS_APPROVED))
        {
            w_list->markItemRed(addon->getId(), true);
        }
    }

	getWidget<GUIEngine::RibbonWidget>("category")->setActivated();
	if(m_type == "kart")
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_kart", 
                                                        PLAYER_ID_GAME_MASTER);
	else if(m_type == "track")
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_track", 
                                                        PLAYER_ID_GAME_MASTER);
    else
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_update", 
                                                        PLAYER_ID_GAME_MASTER);
}   // loadList

// ----------------------------------------------------------------------------
void AddonsScreen::onColumnClicked(int column_id)
{
    switch(column_id)
    {
    case 0: Addon::setSortOrder(Addon::SO_NAME); break;
    case 1: Addon::setSortOrder(Addon::SO_DATE); break;
    default: assert(0);
    }   // switch
    loadList();
}   // onColumnClicked

// ----------------------------------------------------------------------------
void AddonsScreen::eventCallback(GUIEngine::Widget* widget, 
                                 const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    else if (name == "reload")
    {
        if (!m_reloading)
        {
            m_reloading = true;
            network_http->insertReInit();
            //new MessageDialog(_("Please wait while addons are updated, or click the button below to reload in the background."),
            //    MessageDialog::MESSAGE_DIALOG_OK, this, false);
            
            GUIEngine::ListWidget* w_list = 
            getWidget<GUIEngine::ListWidget>("list_addons");
            w_list->clear();
            
            w_list->addItem("spacer", L"");
            w_list->addItem("loading",
                            _("Please wait while addons are updated"),
                            m_icon_loading);
        }
    }

    else if (name == "list_addons")
    {
        GUIEngine::ListWidget* list = 
            getWidget<GUIEngine::ListWidget>("list_addons");
        std::string id = list->getSelectionInternalName();

        if (!id.empty() && addons_manager->getAddon(id) != NULL)
        {
            m_selected_index = list->getSelectionID();
            new AddonsLoading(0.8f, 0.8f, id);
        }
    }
    if (name == "category")
    {
        std::string selection = ((GUIEngine::RibbonWidget*)widget)
                         ->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        if (selection == "tab_track")
        {
            m_type = "track";
            loadList();
        }
        else if (selection == "tab_kart")
        {
            m_type = "kart";
            loadList();
        }
        else if (selection == "tab_arena")
        {
            m_type = "arena";
            loadList();
        }
    }
}   // eventCallback

// ----------------------------------------------------------------------------
/** Selects the last selected item on the list (which is the item that
 *  is just being installed) again. This function is used from the
 *  addons_loading screen: when it is closed, it will reset the
 *  select item so that people can keep on installing from that
 *  point on.
*/
void AddonsScreen::setLastSelected()
{
    if(m_selected_index>-1)
    {
        GUIEngine::ListWidget* list = 
            getWidget<GUIEngine::ListWidget>("list_addons");
        list->setSelectionID(m_selected_index);
    }
}   // setLastSelected

// ----------------------------------------------------------------------------

void AddonsScreen::onUpdate(float dt, irr::video::IVideoDriver*)
{
    if (m_reloading)
    {
        if(UserConfigParams::m_internet_status!=NetworkHttp::IPERM_ALLOWED)
        {
            // not allowed to access the net. how did you get to this menu in the first place??
            loadList();
            m_reloading = false;
        }
        else if (addons_manager->wasError())
        {
            m_reloading = false;
            new MessageDialog( _("Sorry, an error occurred while contacting the add-ons website. Make sure you are connected to the Internet and that SuperTuxKart is not blocked by a firewall") );
        }
        else if (addons_manager->onlineReady())
        {
            m_reloading = false;
            loadList();
        }
        else
        {
            // Addons manager is still initialising/downloading.
        }
    }
}
