//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Lucas Baudin, Joerg Henrichs
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

#include "addons/addons_manager.hpp"
#include "addons/news_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "io/file_manager.hpp"
#include "online/request_manager.hpp"
#include "states_screens/dialogs/addons_loading.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/translation.hpp"

#include <iostream>

using namespace Online;
// ----------------------------------------------------------------------------

AddonsScreen::AddonsScreen() : Screen("addons_screen.stkgui")
{
    m_selected_index = -1;

    // Add date filters.
    // I18N: Time filters for add-ons
    DateFilter filter_all = {_("All"), 0, 0, 0};
    DateFilter filter_1w = {_("1 week"), 0, 0, 7};
    DateFilter filter_2w = {_("2 weeks"), 0, 0, 14};
    DateFilter filter_1m = {_("1 month"), 0, 1, 0};
    DateFilter filter_3m = {_("3 months"), 0, 3, 0};
    DateFilter filter_6m = {_("6 months"), 0, 6, 0};
    DateFilter filter_9m = {_("9 months"), 0, 9, 0};
    DateFilter filter_1y = {_("1 year"), 1, 0, 0};
    DateFilter filter_2y = {_("2 years"), 2, 0, 0};
    m_date_filters.push_back(filter_all);
    m_date_filters.push_back(filter_1w);
    m_date_filters.push_back(filter_2w);
    m_date_filters.push_back(filter_1m);
    m_date_filters.push_back(filter_3m);
    m_date_filters.push_back(filter_6m);
    m_date_filters.push_back(filter_9m);
    m_date_filters.push_back(filter_1y);
    m_date_filters.push_back(filter_2y);
}   // AddonsScreen

// ----------------------------------------------------------------------------

void AddonsScreen::loadedFromFile()
{
    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "package.png"         ));
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "no-package.png"      ));
    video::ITexture* icon3 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "package-update.png"  ));
    video::ITexture* icon4 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "package-featured.png"));
    video::ITexture* icon5 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                  "no-package-featured.png"));
    video::ITexture* icon6 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "loading.png"));

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv());
    m_icon_installed     = m_icon_bank->addTextureAsSprite(icon1);
    m_icon_not_installed = m_icon_bank->addTextureAsSprite(icon2);
    m_icon_bank->addTextureAsSprite(icon4);
    m_icon_bank->addTextureAsSprite(icon5);
    m_icon_loading = m_icon_bank->addTextureAsSprite(icon6);
    m_icon_needs_update  = m_icon_bank->addTextureAsSprite(icon3);

    GUIEngine::ListWidget* w_list =
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->setColumnListener(this);
}   // loadedFromFile


// ----------------------------------------------------------------------------

void AddonsScreen::beforeAddingWidget()
{
    GUIEngine::ListWidget* w_list =
        getWidget<GUIEngine::ListWidget>("list_addons");
    assert(w_list != NULL);
    w_list->clearColumns();
    w_list->addColumn( _("Add-on name"), 3 );
    w_list->addColumn( _("Updated date"), 1 );
    
    GUIEngine::SpinnerWidget* w_filter_date =
                        getWidget<GUIEngine::SpinnerWidget>("filter_date");
    w_filter_date->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    w_filter_date->m_properties[GUIEngine::PROP_MAX_VALUE] =
                            StringUtils::toString(m_date_filters.size() - 1);
    
    for (unsigned int n = 0; n < m_date_filters.size(); n++)
    {
        w_filter_date->addLabel(m_date_filters[n].label);
    }

    GUIEngine::SpinnerWidget* w_filter_rating =
                        getWidget<GUIEngine::SpinnerWidget>("filter_rating");
    w_filter_rating->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    w_filter_rating->m_properties[GUIEngine::PROP_MAX_VALUE] = "6";
    
    for (int n = 0; n < 7; n++)
    {
        w_filter_rating->addLabel(StringUtils::toWString(n / 2.0));
    }

    GUIEngine::SpinnerWidget* w_filter_installation =
                        getWidget<GUIEngine::SpinnerWidget>("filter_installation");
    w_filter_installation->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    w_filter_installation->m_properties[GUIEngine::PROP_MAX_VALUE] = "2";
    w_filter_installation->addLabel(_("All"));
    w_filter_installation->addLabel(_("Installed"));
    //I18N: Addon not installed for fillter
    w_filter_installation->addLabel(_("Not installed"));
}
// ----------------------------------------------------------------------------

void AddonsScreen::init()
{
    Screen::init();

    m_sort_desc = false;
    m_reloading = false;

    getWidget<GUIEngine::RibbonWidget>("category")->setActive(false);

    if(UserConfigParams::logAddons())
        Log::info("addons", "Using directory <%s>", file_manager->getAddonsDir().c_str());

    GUIEngine::ListWidget* w_list =
        getWidget<GUIEngine::ListWidget>("list_addons");

    // This defines the row height !
    m_icon_height = GUIEngine::getFontHeight() * 2;
    // 128 is the height of the image file
    m_icon_bank->setScale((float)GUIEngine::getFontHeight() / 72.0f);
    m_icon_bank->setTargetIconSize(128,128);
    w_list->setIcons(m_icon_bank, (int)(m_icon_height));

    m_type = "kart";

    bool ip = UserConfigParams::m_internet_status == RequestManager::IPERM_ALLOWED;
    getWidget<GUIEngine::IconButtonWidget>("reload")->setActive(ip);

    // Reset filter.
    GUIEngine::TextBoxWidget* w_filter_name =
                        getWidget<GUIEngine::TextBoxWidget>("filter_name");
    w_filter_name->setText(L"");
    // Add listener for incremental update when search text is changed
    w_filter_name->clearListeners();
    w_filter_name->addListener(this);
    GUIEngine::SpinnerWidget* w_filter_date =
                        getWidget<GUIEngine::SpinnerWidget>("filter_date");
    w_filter_date->setValue(0);
    GUIEngine::SpinnerWidget* w_filter_rating =
                        getWidget<GUIEngine::SpinnerWidget>("filter_rating");
    w_filter_rating->setValue(0);

    GUIEngine::SpinnerWidget* w_filter_installation =
                        getWidget<GUIEngine::SpinnerWidget>("filter_installation");
    w_filter_installation->setValue(0);

    // Set the default sort order
    Addon::setSortOrder(Addon::SO_DEFAULT);
    loadList();
}   // init

// ----------------------------------------------------------------------------

void AddonsScreen::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}

// ----------------------------------------------------------------------------

void AddonsScreen::tearDown()
{
}

// ----------------------------------------------------------------------------
/** Loads the list of all addons of the given type. The gui element will be
 *  updated.
 */
void AddonsScreen::loadList()
{
#ifndef SERVER_ONLY
    // Get the filter by words.
    GUIEngine::TextBoxWidget* w_filter_name =
                        getWidget<GUIEngine::TextBoxWidget>("filter_name");
    core::stringw words = w_filter_name->getText();

    // Get the filter by date.
    GUIEngine::SpinnerWidget* w_filter_date =
                        getWidget<GUIEngine::SpinnerWidget>("filter_date");
    int date_index = w_filter_date->getValue();
    StkTime::TimeType date = StkTime::getTimeSinceEpoch();
    date = StkTime::addInterval(date,
                -m_date_filters[date_index].year,
                -m_date_filters[date_index].month,
                -m_date_filters[date_index].day);

    // Get the filter by rating.
    GUIEngine::SpinnerWidget* w_filter_rating =
                        getWidget<GUIEngine::SpinnerWidget>("filter_rating");
    float rating = w_filter_rating->getValue() / 2.0f;
    
    GUIEngine::SpinnerWidget* w_filter_installation =
                        getWidget<GUIEngine::SpinnerWidget>("filter_installation");

    // First create a list of sorted entries
    PtrVector<const Addon, REF> sorted_list;
    for(unsigned int i=0; i<addons_manager->getNumAddons(); i++)
    {
        const Addon & addon = addons_manager->getAddon(i);
        // Ignore not installed addons if the checkbox is enabled
        if(   (w_filter_installation->getValue() == 1 && !addon.isInstalled())
           || (w_filter_installation->getValue() == 2 &&  addon.isInstalled()))
            continue;
        // Ignore addons of a different type
        if(addon.getType()!=m_type) continue;
        // Ignore invisible addons
        if(addon.testStatus(Addon::AS_INVISIBLE))
            continue;
        if(!UserConfigParams::m_artist_debug_mode &&
            !addon.testStatus(Addon::AS_APPROVED)    )
            continue;
        if (!addon.isInstalled() && (addons_manager->wasError() ||
                                     UserConfigParams::m_internet_status !=
                                     RequestManager::IPERM_ALLOWED ))
            continue;

        // Filter by rating.
        if (addon.getRating() < rating)
            continue;

        // Filter by date.
        if (date_index != 0 && StkTime::compareTime(date, addon.getDate()) > 0)
            continue;

        // Filter by name, designer and description.
        if (!addon.filterByWords(words))
            continue;

        sorted_list.push_back(&addon);
    }
    sorted_list.insertionSort(/*start=*/0, m_sort_desc);

    GUIEngine::ListWidget* w_list =
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->clear();

    for(unsigned int i=0; i<sorted_list.size(); i++)
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
        {
            s = (addon->getName()+L"\t" +
                    core::stringc(addon->getDateAsString().c_str())).c_str();
        }

       //FIXME I'd like to move this to CGUISTKListBox.cpp

       /* gui::IGUIFont* font = GUIEngine::getFont();

        // first column is 0.666% of the list's width.
        // and icon width == icon height.

        const unsigned int available_width = (int)(w_list->m_w*0.6666f
                                                   - m_icon_height);
        if (font->getDimension(s.c_str()).Width > available_width)
        {
            s = s.subString(0, int(AddonsScreen::getWidth()*0.018)+20);
            s.append("...");
        }
        else
        {*/
            if (addon->getDesigner().size() == 0)
            {
                s = addon->getName();
            }
            else
            {
                //I18N: as in: The Old Island by Johannes Sjolund
                s = _C("addons", "%s by %s", addon->getName().c_str(),
                        addon->getDesigner().c_str());
            }
            /*
            // check if text is too long to fit
            if (font->getDimension(s.c_str()).Width >  available_width)
            {
                // start by splitting on 2 lines

                //I18N: as in: The Old Island by Johannes Sjolund
                s = _("%s\nby %s",addon->getName().c_str(),
                      addon->getDesigner().c_str());

                core::stringw final_string;

                // then check if each line is now short enough.
                std::vector<irr::core::stringw> lines =
                    StringUtils::split(s, '\n');
                for (unsigned int n=0; n<lines.size(); n++)
                {
                    if (font->getDimension(lines[n].c_str()).Width
                          > available_width)
                    {
                        // arg, still too long! cut the text so that it fits.
                        core::stringw line = lines[n];

                        // leave a margin of 14 pixels to account for the "..."
                        // that will be appended
                        int split_at = font->getCharacterFromPos(line.c_str(),
                                                         available_width - 14);
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
                }   // for nlines.size()

                s = final_string;
                */
            //}   // if
        //}

        // we have no icon for featured+updateme, so if an add-on is updatable
        // forget about the featured icon
        if (addon->testStatus(Addon::AS_FEATURED) &&
            icon != m_icon_needs_update)
        {
            icon += 2;
        }

        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(s.c_str(), icon, 3, false));
        row.push_back(GUIEngine::ListWidget::ListCell(addon->getDateAsString().c_str(), -1, 1, true));
        w_list->addItem(addon->getId(), row);

        // Highlight if it's not approved in artists debug mode.
        if(UserConfigParams::m_artist_debug_mode &&
            !addon->testStatus(Addon::AS_APPROVED))
        {
            w_list->markItemRed(addon->getId(), true);
        }
    }

    getWidget<GUIEngine::RibbonWidget>("category")->setActive(true);
    if(m_type == "kart")
        getWidget<GUIEngine::RibbonWidget>("category")->select("tab_kart",
                                                        PLAYER_ID_GAME_MASTER);
    else if(m_type == "track")
        getWidget<GUIEngine::RibbonWidget>("category")->select("tab_track",
                                                        PLAYER_ID_GAME_MASTER);
    else
        getWidget<GUIEngine::RibbonWidget>("category")->select("tab_update",
                                                        PLAYER_ID_GAME_MASTER);
#endif
}   // loadList

// ----------------------------------------------------------------------------
void AddonsScreen::onColumnClicked(int column_id, bool sort_desc, bool sort_default)
{
    switch(column_id)
    {
    case 0:
        Addon::setSortOrder(sort_default ? Addon::SO_DEFAULT : Addon::SO_NAME);
        break;
    case 1:
        Addon::setSortOrder(sort_default ? Addon::SO_DEFAULT : Addon::SO_DATE);
        break;
    default: assert(0); break;
    }   // switch
    /** \brief Toggle the sort order after column click **/
    m_sort_desc = sort_desc && !sort_default;
    loadList();
}   // onColumnClicked

// ----------------------------------------------------------------------------
void AddonsScreen::eventCallback(GUIEngine::Widget* widget,
                                 const std::string& name, const int playerID)
{
#ifndef SERVER_ONLY
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    else if (name == "reload")
    {
        if (!m_reloading)
        {
            m_reloading = true;
            NewsManager::get()->init(true);

            GUIEngine::ListWidget* w_list =
                       getWidget<GUIEngine::ListWidget>("list_addons");
            w_list->clear();

            w_list->addItem("spacer", L"");
            w_list->addItem("loading",_("Please wait while addons are updated"), m_icon_loading);
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
            new AddonsLoading(id);
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
    else if (name == "filter_search" || name == "filter_installation")
    {
        loadList();
    }
#endif
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
        list->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        list->setSelectionID(m_selected_index);
    }
}   // setLastSelected

// ----------------------------------------------------------------------------

void AddonsScreen::onUpdate(float dt)
{
#ifndef SERVER_ONLY
    NewsManager::get()->joinDownloadThreadIfExit();

    if (m_reloading)
    {
        if(UserConfigParams::m_internet_status!=RequestManager::IPERM_ALLOWED)
        {
            // not allowed to access the net. how did you get to this menu in
            // the first place??
            loadList();
            m_reloading = false;
        }
        else if (addons_manager->wasError())
        {
            m_reloading = false;
            new MessageDialog( _("Sorry, an error occurred while contacting "
                                 "the add-ons website. Make sure you are "
                                 "connected to the Internet and that "
                                 "SuperTuxKart is not blocked by a firewall"));
            loadList();
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
#endif
}   // onUpdate
