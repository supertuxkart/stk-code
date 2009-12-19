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
#include "states_screens/state_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::video;

ArenasScreen::ArenasScreen() : Screen("arenas.stkgui")
{
}


void ArenasScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if (w2 != NULL)
        {
            const std::string selection = w2->getSelectionIDString(GUI_PLAYER_ID);
            std::cout << "Clicked on arena " << selection.c_str() << std::endl;
            
            if (selection == "random_track")
            {
                // TODO
            }
            else
            {
                Track* clickedTrack = track_manager->getTrack(selection);
                if (clickedTrack != NULL)
                {
                    ITexture* screenshot = GUIEngine::getDriver()->getTexture( clickedTrack->getScreenshotFile().c_str() );
                    
                    new TrackInfoDialog( clickedTrack->getIdent(), clickedTrack->getName().c_str(), screenshot, 0.8f, 0.7f);
                }
            }
        }
    }
    
}

void ArenasScreen::init()
{
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( w != NULL );
    
    // Re-build track list everytime (accounts for locking changes, etc.)
    w->clearItems();
    
    const int trackAmount = track_manager->getNumberOfTracks();
    //bool hasLockedTracks = false;
    for (int n=0; n<trackAmount; n++)
    {
        Track* curr = track_manager->getTrack(n);
        if (!curr->isArena()) continue;

        if (unlock_manager->isLocked(curr->getIdent()))
        {
            w->addItem( _("Locked : solve active challenges to gain access to more!"), "locked", curr->getScreenshotFile(), true );
        }
        else
        {
             w->addItem( curr->getName(), curr->getIdent(), curr->getScreenshotFile(), false );
        }
    }
    w->addItem(_("Random Arena"), "random_track", "/gui/track_random.png");
    w->updateItemDisplay();    
}

void ArenasScreen::tearDown()
{
}

void ArenasScreen::setFocusOnTrack(const std::string& trackName)
{
    DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("tracks");
    assert( w != NULL );
    
    // FIXME: don't hardcode player 0?
    w->setSelection(trackName, 0, true); 
}
