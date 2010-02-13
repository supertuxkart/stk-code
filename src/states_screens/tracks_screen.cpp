//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "challenges/unlock_manager.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "states_screens/dialogs/gp_info_dialog.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

const char* ALL_TRACK_GROUPS_ID = "all";

// -----------------------------------------------------------------------------------------------

TracksScreen::TracksScreen() : Screen("tracks.stkgui")
{
    // Dynamically add tabs
    // FIXME: it's not very well documented that RibbonWidgets can have dynamically generated contents
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
    
    tabs->m_children.clearAndDeleteAll();
    
    const std::vector<std::string>& groups = track_manager->getAllGroups();
    
    const int amount = groups.size();
    for (int n=0; n<amount; n++)
    {
        ButtonWidget* item = new ButtonWidget();
        item->m_text = groups[n].c_str(); // FIXME: i18n ?
        item->m_properties[PROP_ID] = groups[n];
        tabs->m_children.push_back(item);
    }
    
    ButtonWidget* item = new ButtonWidget();
    //I18N: name of the tab that will show tracks from all groups
    item->m_text = _("All");
    item->m_properties[PROP_ID] = ALL_TRACK_GROUPS_ID;
    tabs->m_children.push_back(item);
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    // -- track seelction screen
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (w2 != NULL)
        {
            const std::string selection = w2->getSelectionIDString(GUI_PLAYER_ID);
            std::cout << "Clicked on track " << selection.c_str() << std::endl;
            
            if (selection == "random_track")
            {
                // TODO
            }
            else if (selection == "locked")
            {
                unlock_manager->playLockSound();
            }
            else if (selection == NO_ITEM_ID)
            {
            }
            else
            {
                Track* clickedTrack = track_manager->getTrack(selection);
                if (clickedTrack != NULL)
                {
                    ITexture* screenshot = irr_driver->getTexture( clickedTrack->getScreenshotFile().c_str() );
                    
                    new TrackInfoDialog( clickedTrack->getIdent(), clickedTrack->getName().c_str(),
                                         screenshot, 0.8f, 0.7f);
                }
            }
        }
    }
    else if (name == "gps")
    {
        DynamicRibbonWidget* gps_widget = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (gps_widget != NULL)
        {
            std::cout << "Clicked on GrandPrix "
            << gps_widget->getSelectionIDString(GUI_PLAYER_ID).c_str()
            << std::endl;
            
            new GPInfoDialog( gps_widget->getSelectionIDString(GUI_PLAYER_ID), 0.8f, 0.7f );
        }
        else
        {
            assert(false);
        }
    }
    
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::init()
{
    DynamicRibbonWidget* gps_widget = this->getWidget<DynamicRibbonWidget>("gps");
    assert( gps_widget != NULL );
    
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );
    
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("trackgroups");
    assert( tabs != NULL );
       
    // Reset GP list everytime (accounts for locking changes, etc.)
    gps_widget->clearItems();
    
    // Build GP list
    const int gpAmount = grand_prix_manager->getNumberOfGrandPrix();
    for (int n=0; n<gpAmount; n++)
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(n);
        std::cout << "Got GP : " << gp->getId() << std::endl;
        
        std::vector<std::string> tracks = gp->getTracks();
        
        std::vector<std::string> sshot_files;
        for (unsigned int t=0; t<tracks.size(); t++)
        {
            Track* curr = track_manager->getTrack(tracks[t]);
            if (curr == NULL)
            {
                //TODO: show warning to user? don't add GP to GUI?
                std::cerr << "/!\\ WARNING: Grand Prix '" << gp->getId() << "' refers to track '"
                          << tracks[t] << "', which does not exist.\n";
            }
            else
            {
                sshot_files.push_back(curr->getScreenshotFile());
            }
        }
        if (sshot_files.size() == 0)
        {
            //TODO: show warning to user? don't add GP to GUI?
            std::cerr << "/!\\ WARNING: Grand Prix '" << gp->getId() << "' does not contain any valid track.\n";
            sshot_files.push_back("gui/main_help.png");
        }
        
        if (unlock_manager->isLocked(gp->getId()))
        {
            gps_widget->addAnimatedItem( _("Locked : solve active challenges to gain access to more!"),
                                        "locked", sshot_files, 1.5f, TROPHY_BADGE );
        }
        else
        {
            gps_widget->addAnimatedItem( gp->getName(), gp->getId(), sshot_files, 1.5f, TROPHY_BADGE );
        }
    }
    gps_widget->updateItemDisplay();
    
    // Reset track list everytime (accounts for locking changes, etc.)
    tracks_widget->clearItems();
    
    // Build track list
    const int trackAmount = track_manager->getNumberOfTracks();
    for (int n=0; n<trackAmount; n++)
    {
        Track* curr = track_manager->getTrack(n);
        if (curr->isArena()) continue;
        
        if (unlock_manager->isLocked(curr->getIdent()))
        {
            tracks_widget->addItem( _("Locked : solve active challenges to gain access to more!"), "locked", curr->getScreenshotFile(), true );
        }
        else
        {
            tracks_widget->addItem( curr->getName(), curr->getIdent(), curr->getScreenshotFile(), false );
        }
    }
    tracks_widget->addItem(_("Random Track"), "random_track", "/gui/track_random.png");

    tracks_widget->updateItemDisplay();    
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::tearDown()
{
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* tracks_widget = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( tracks_widget != NULL );
    
    // FIXME: don't hardcode player 0?
    tracks_widget->setSelection(trackName, 0, true); 
}

// -----------------------------------------------------------------------------------------------

void TracksScreen::setFocusOnGP(const std::string& gpName)
{
    DynamicRibbonWidget* gps_widget = this->getWidget<DynamicRibbonWidget>("gps");
    assert( gps_widget != NULL );
    
    // FIXME: don't hardcode player 0?
    gps_widget->setSelection(gpName, 0, true); 
}

// -----------------------------------------------------------------------------------------------

