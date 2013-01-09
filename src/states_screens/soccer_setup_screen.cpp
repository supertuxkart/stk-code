//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Lionel Fuentes
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

#include "states_screens/soccer_setup_screen.hpp"

#include "states_screens/state_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_model.hpp"

using namespace GUIEngine;
DEFINE_SCREEN_SINGLETON( SoccerSetupScreen );

// -----------------------------------------------------------------------------

SoccerSetupScreen::SoccerSetupScreen() : Screen("soccer_setup.stkgui")
{
}

// -----------------------------------------------------------------------------

void SoccerSetupScreen::loadedFromFile()
{
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if(name == "continue")
    {
        StateManager::get()->pushScreen( ArenasScreen::getInstance() );
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::beforeAddingWidget()
{
    Widget* central_div = getWidget<Widget>("central_div");
    
    /*
    const unsigned int kart_amount = m_karts.size();
    
    int team_karts_amount[NB_SOCCER_TEAMS];
    memset(team_karts_amount, 0, sizeof(team_karts_amount));
    
    {
        // Set the kart teams if they haven't been already set by the setup screen
        // (happens when the setup screen is skipped, with 1 player)
        SoccerTeam    round_robin_team = SOCCER_TEAM_RED;
        for(unsigned int n=0; n<kart_amount; n++)
        {
            if(m_karts[n]->getSoccerTeam() == SOCCER_TEAM_NONE)
                m_karts[n]->setSoccerTeam(round_robin_team);
            
            team_karts_amount[m_karts[n]->getSoccerTeam()]++;
            
            round_robin_team = (round_robin_team==SOCCER_TEAM_RED ?
                                SOCCER_TEAM_BLUE : SOCCER_TEAM_RED);
        }// next kart
    }
    
    // Compute start positions for each team
    int team_cur_position[NB_SOCCER_TEAMS];
    team_cur_position[0] = 1;
    for(int i=1 ; i < (int)NB_SOCCER_TEAMS ; i++)
        team_cur_position[i] = team_karts_amount[i-1] + team_cur_position[i-1];
    
    // Set kart positions, ordering them by team
    for(unsigned int n=0; n<kart_amount; n++)
    {
        SoccerTeam  team = m_karts[n]->getSoccerTeam();
        m_karts[n]->setPosition(team_cur_position[team]);
        team_cur_position[team]++;
    }// next kart    
    */
    
    // BEGIN TEST
    ModelViewWidget*    kart_view = new ModelViewWidget();
    kart_view->m_x = central_div->m_x + 10;
    kart_view->m_y = central_div->m_y + 10;
    kart_view->m_w = 200;
    kart_view->m_h = 200;
    kart_view->clearModels();
    
    const std::string default_kart = UserConfigParams::m_default_kart;
    const KartProperties* props = 
        kart_properties_manager->getKart(default_kart);
    const KartModel &kart_model = props->getMasterKartModel();
    
    kart_view->addModel( kart_model.getModel(), Vec3(0,0,0),
                            Vec3(35.0f, 35.0f, 35.0f),
                            kart_model.getBaseFrame() );
    kart_view->addModel( kart_model.getWheelModel(0), 
                            kart_model.getWheelGraphicsPosition(0) );
    kart_view->addModel( kart_model.getWheelModel(1), 
                            kart_model.getWheelGraphicsPosition(1) );
    kart_view->addModel( kart_model.getWheelModel(2),
                            kart_model.getWheelGraphicsPosition(2) );
    kart_view->addModel( kart_model.getWheelModel(3),
                            kart_model.getWheelGraphicsPosition(3) );
    kart_view->setRotateContinuously( 35.0f );
    
    kart_view->update(0);
    
    central_div->getChildren().push_back(kart_view);
    // END TEST
    
    // Add "VS" label at the center of the rounded box
    m_label_vs = new LabelWidget(true, false);
    
    core::dimension2d<u32>  vs_size = GUIEngine::getTitleFont()->getDimension( L"VS" );
    m_label_vs->m_x = central_div->m_x + central_div->m_w/2 - vs_size.Width/2;
    m_label_vs->m_y = central_div->m_y + central_div->m_h/2 - vs_size.Height/2;
    m_label_vs->m_w = vs_size.Width;
    m_label_vs->m_h = vs_size.Height;
    
    central_div->getChildren().push_back(m_label_vs);
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::init()
{
    Screen::init();
    
    // TODO: remember in config.xml the last number of goals
    SpinnerWidget*  goalamount = getWidget<SpinnerWidget>("goalamount");
    goalamount->setValue(3);
    
    // Set focus on "continue"
    ButtonWidget*   bt_continue = getWidget<ButtonWidget>("continue");
    bt_continue->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    
    // "VS" (needs to be done here for the Irrlicht element to be here)
    m_label_vs->setText("VS", true);
    /*
    Widget* players_table = getWidget<Widget>("players-table");
    assert(players_table != NULL);
    
    manualAddWidget(new TeamViewWidget(this));
    */
    
    /*// ----- Kart model view
        m_model_view = new ModelViewWidget();
        
        m_model_view->m_x = model_x;
        m_model_view->m_y = model_y;
        m_model_view->m_w = model_w;
        m_model_view->m_h = model_h;
        m_model_view->m_properties[PROP_ID] = 
            StringUtils::insertValues("@p%i_model", m_playerID);
        //m_model_view->setParent(this);
        m_children.push_back(m_model_view);
        
        // Init kart model
        const std::string default_kart = UserConfigParams::m_default_kart;
        const KartProperties* props = 
            kart_properties_manager->getKart(default_kart);
                
        if(!props)
        {            
            // If the default kart can't be found (e.g. previously a addon 
            // kart was used, but the addon package was removed), use the
            // first kart as a default. This way we don't have to hardcode
            // any kart names.
            int id = kart_properties_manager->getKartByGroup(kartGroup, 0);
            if (id == -1)
            {
                props = kart_properties_manager->getKartById(0);
            }
            else
            {
                props = kart_properties_manager->getKartById(id);
            }
            
            if(!props)
            {
                fprintf(stderr, 
                        "[KartSelectionScreen] WARNING: Can't find default "
                        "kart '%s' nor any other kart.\n",
                        default_kart.c_str());
                exit(-1);
            }
        }
        m_kartInternalName = props->getIdent();

        const KartModel &kart_model = props->getMasterKartModel();
        
        m_model_view->addModel( kart_model.getModel(), Vec3(0,0,0),
                                Vec3(35.0f, 35.0f, 35.0f),
                                kart_model.getBaseFrame() );
        m_model_view->addModel( kart_model.getWheelModel(0), 
                                kart_model.getWheelGraphicsPosition(0) );
        m_model_view->addModel( kart_model.getWheelModel(1), 
                                kart_model.getWheelGraphicsPosition(1) );
        m_model_view->addModel( kart_model.getWheelModel(2),
                                kart_model.getWheelGraphicsPosition(2) );
        m_model_view->addModel( kart_model.getWheelModel(3),
                                kart_model.getWheelGraphicsPosition(3) );
        m_model_view->setRotateContinuously( 35.0f );
        
        // ---- Kart name label
        m_kart_name = new LabelWidget();
        m_kart_name->setText(props->getName(), false);
        m_kart_name->m_properties[PROP_TEXT_ALIGN] = "center";
        m_kart_name->m_properties[PROP_ID] = 
            StringUtils::insertValues("@p%i_kartname", m_playerID);
        m_kart_name->m_x = kart_name_x;
        m_kart_name->m_y = kart_name_y;
        m_kart_name->m_w = kart_name_w;
        m_kart_name->m_h = kart_name_h;
        //m_kart_name->setParent(this);
        m_children.push_back(m_kart_name);*/
}
