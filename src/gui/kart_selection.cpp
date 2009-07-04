//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 
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

#include "config/player.hpp"
#include "config/user_config.hpp"
#include "kart_selection.hpp"
#include "gui/widget.hpp"
#include "gui/engine.hpp"
#include "gui/screen.hpp"
#include "gui/state_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "karts/kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "utils/translation.hpp"

#include <string>

InputDevice* player_1_device = NULL;

using namespace GUIEngine;

namespace StateManager
{
    class PlayerKart : public Widget
    {
    public:
        LabelWidget* playerID;
        SpinnerWidget* playerName;
        ModelViewWidget* modelView;
        LabelWidget* kartName;
        
        PlayerKart(Widget* area) : Widget()
        {
            this->m_properties[PROP_ID] = "@p1";
            this->x = area->x;
            this->y = area->y;
            this->w = area->w;
            this->h = area->h;
            
            playerID = new LabelWidget();
            playerID->m_properties[PROP_TEXT] = _("Player 1 (keyboard)");
            playerID->m_properties[PROP_TEXT_ALIGN] = "center";
            playerID->m_properties[PROP_ID] = "@p1_label";
            playerID->x = area->x;
            playerID->y = area->y;
            playerID->w = area->w;
            playerID->h = 25;
            //playerID->setParent(this);
            m_children.push_back(playerID);
            
            const int playerAmount = UserConfigParams::m_player.size();
            const int spinnerWidth = std::min(400, area->w/3 /* FIXME : replace by number of players */);
            playerName = new SpinnerWidget();
            playerName->x = area->x + area->w/2 - spinnerWidth/2;
            playerName->y = area->y + 25;
            playerName->w = spinnerWidth;
            playerName->h = 40;
            
            playerName->m_properties[PROP_MIN_VALUE] = "0";
            playerName->m_properties[PROP_MAX_VALUE] = (playerAmount-1);
            playerName->m_properties[PROP_ID] = "@p1_spinner";
            //playerName->setParent(this);
            m_children.push_back(playerName);
            
            
            modelView = new ModelViewWidget();
            const int modelY = area->y + 65;
            const int modelMaxHeight =  area->h - 25 - 65;
            const int modelMaxWidth =  area->w;
            const int bestSize = std::min(modelMaxWidth, modelMaxHeight);
                        
            modelView->x = area->x + area->w/2 - bestSize*1.2/2;
            modelView->y = modelY + modelMaxHeight/2 - bestSize/2;
            modelView->w = bestSize*1.2; // FIXME : for some reason, it looks better this way, though full square should be ok
            modelView->h = bestSize;
            modelView->m_properties[PROP_ID] = "@p1_model";
            //modelView->setParent(this);
            m_children.push_back(modelView);
            
            
            kartName = new LabelWidget();
            kartName->m_properties[PROP_TEXT] = _("Tux");
            kartName->m_properties[PROP_TEXT_ALIGN] = "center";
            kartName->m_properties[PROP_ID] = "@p1_kartname";
            kartName->x = area->x;
            kartName->y = area->y + area->h - 25;
            kartName->w = area->w;
            kartName->h = 25;
            //kartName->setParent(this);
            m_children.push_back(kartName);
        }
        virtual void add()
        {
            playerID->add();
            playerName->add();
            modelView->add();
            kartName->add();
        } 
        
    };
    
    // ref only since we're adding them to a Screen, and the Screen will take ownership of these widgets
    ptr_vector<PlayerKart, REF> g_player_karts;
    
class KartHoverListener : public RibbonGridHoverListener
    {
    public:
        void onSelectionChanged(RibbonGridWidget* theWidget, const std::string& selectionID)
        {
            //std::cout << "hovered " << selectionID.c_str() << std::endl;
            
            if(selectionID.size() == 0) return;
            
            // TODO : support players other than player 0 (i.e. multiplayer)
            ModelViewWidget* w3 = g_player_karts[0].modelView;
            assert( w3 != NULL );
            
            const KartProperties* kart = kart_properties_manager->getKart(selectionID);
            if(kart == NULL) return;
            KartModel* kartModel = kart->getKartModel();
            
            w3->clearModels();
            w3->addModel( kartModel->getModel() );
            w3->addModel( kartModel->getWheelModel(0), kartModel->getWheelGraphicsPosition(0) );
            w3->addModel( kartModel->getWheelModel(1), kartModel->getWheelGraphicsPosition(1) );
            w3->addModel( kartModel->getWheelModel(2), kartModel->getWheelGraphicsPosition(2) );
            w3->addModel( kartModel->getWheelModel(3), kartModel->getWheelGraphicsPosition(3) );
            w3->update(0);
            
            // TODO : support players other than player 0 (i.e. multiplayer)
            g_player_karts[0].kartName->setText( kart->getName().c_str() );
        }
    };
KartHoverListener* karthoverListener = NULL;

void setPlayer0Device(InputDevice* device)
{
    if(device == NULL)
    {
        std::cout << "I don't know which device to assign to player 0 :'(\n";
        return;
    }
    
    if(device->getType() == DT_KEYBOARD)
    {
        std::cout << "Player 0 is using a keyboard\n";
    }
    if(device->getType() == DT_GAMEPAD)
    {
        std::cout << "Player 0 is using a gamepad\n";
    }
    
    // TODO : support more than 1 player
    StateManager::addActivePlayer( UserConfigParams::m_player.get(0) );
    UserConfigParams::m_player[0].setDevice(device);
    input_manager->getDeviceList()->setNoAssignMode(false);
    
    // TODO : fall back in no-assign mode when aborting a game and going back to the menu
    // how to revert assign mode :
    // StateManager::resetActivePlayers();
    // input_manager->getDeviceList()->setNoAssignMode(true);
}
    
/**
 * Callback handling events from the kart selection menu
 */
void menuEventKarts(Widget* widget, std::string& name)
{
    if(name == "init")
    {
              
        RibbonGridWidget* w = getCurrentScreen()->getWidget<RibbonGridWidget>("karts");
        assert( w != NULL );
        
        if(karthoverListener == NULL)
        {
            karthoverListener = new KartHoverListener();
            w->registerHoverListener(karthoverListener);
        }
        
        if(!getCurrentScreen()->m_inited)
        {

            Widget* area = getCurrentScreen()->getWidget("playerskarts");
            
            PlayerKart* playerKart1 = new PlayerKart(area);
            getCurrentScreen()->manualAddWidget(playerKart1);
            playerKart1->add();
            g_player_karts.push_back(playerKart1);
            
            // Build kart list
            const int kart_amount = kart_properties_manager->getNumberOfKarts();
            for(int n=0; n<kart_amount; n++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(n);
                std::string icon_path = "karts/";
                icon_path += prop->getIdent() + "/" + prop->getIconFile();
                w->addItem(prop->getName().c_str(), prop->getIdent().c_str(), icon_path.c_str());
                
            }
            
            // Build player list
            const int playerAmount = UserConfigParams::m_player.size();
            for(int n=0; n<playerAmount; n++)
            {
                playerKart1->playerName->addLabel( UserConfigParams::m_player[n].getName() );
            }
            
            // Init kart model
            KartModel* kartModel = kart_properties_manager->getKart("tux")->getKartModel();
            
            playerKart1->modelView->addModel( kartModel->getModel() );
            playerKart1->modelView->addModel( kartModel->getWheelModel(0), kartModel->getWheelGraphicsPosition(0) );
            playerKart1->modelView->addModel( kartModel->getWheelModel(1), kartModel->getWheelGraphicsPosition(1) );
            playerKart1->modelView->addModel( kartModel->getWheelModel(2), kartModel->getWheelGraphicsPosition(2) );
            playerKart1->modelView->addModel( kartModel->getWheelModel(3), kartModel->getWheelGraphicsPosition(3) );
            playerKart1->modelView->update(0);
            
            
            getCurrentScreen()->m_inited = true;
        }
        w->updateItemDisplay();
        
        
        getCurrentScreen()->m_inited = true;
    } // end if init
    else if(name == "karts")
    {
        RibbonGridWidget* w = getCurrentScreen()->getWidget<RibbonGridWidget>("karts");
        assert( w != NULL );
        
        race_manager->setLocalKartInfo(0, w->getSelectionIDString());
        
        StateManager::pushMenu("racesetup.stkgui");
    }
}

}
