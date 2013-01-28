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

#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
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
    
    // Compute some dimensions
    const core::dimension2d<u32>    vs_size = GUIEngine::getTitleFont()->getDimension( L"VS" );
    const int vs_width = (int)vs_size.Width;
    const int vs_height = (int)vs_size.Height;
    const int center_x = central_div->m_x + central_div->m_w/2;
    const int center_y = central_div->m_y + central_div->m_h/2;
    
    // Add "VS" label at the center of the rounded box
    m_label_vs = new LabelWidget(true, false);
    m_label_vs->m_x = center_x - vs_width/2;
    m_label_vs->m_y = center_y - vs_height/2;
    m_label_vs->m_w = vs_width;
    m_label_vs->m_h = vs_height;
    
    central_div->getChildren().push_back(m_label_vs);
    
    // Add the 3D views for the karts
    int nb_players = race_manager->getNumLocalPlayers();
    for(int i=0 ; i < nb_players ; i++)
    {
        const RemoteKartInfo&   kart_info   = race_manager->getLocalKartInfo(i);
        const std::string&      kart_name   = kart_info.getKartName();
        
        const KartProperties*   props       = kart_properties_manager->getKart(kart_name);
        const KartModel&        kart_model  = props->getMasterKartModel();
        
        // Add the view
        ModelViewWidget*    kart_view = new ModelViewWidget();
        kart_view->m_x = 0;
        kart_view->m_y = 0;
        kart_view->m_w = 200;
        kart_view->m_h = 200;   // these values will be overriden by updateKartViewsLayout() anyway
        kart_view->clearModels();
        
        // Add the kart model
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
        
        // Record info about it for further update
        KartViewInfo    info;
        info.view            = kart_view;
        info.confirmed       = false;
        info.local_player_id = i;
        info.team            = i&1 ? SOCCER_TEAM_BLUE : SOCCER_TEAM_RED;
        m_kart_view_info.push_back(info);
    }
    
    // Update layout
    updateKartViewsLayout();
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
    bt_continue->setDeactivated();
    
    // "VS" (needs to be done here for the Irrlicht element to be here)
    m_label_vs->setText("VS", true);
    
    // We need players to be able to choose their teams
    input_manager->getDeviceList()->setAssignMode(ASSIGN);
    input_manager->setMasterPlayerOnly(false);
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation SoccerSetupScreen::filterActions(  PlayerAction action,
                                                               int deviceID,
                                                               const unsigned int value,
                                                               Input::InputType type,
                                                               int playerId)
{
    if(areAllKartsConfirmed())
        return EVENT_LET;
    
    SoccerTeam  team_switch = SOCCER_TEAM_NONE;
    switch(action)
    {
    case PA_MENU_LEFT:
        team_switch = SOCCER_TEAM_RED;
        break;
    case PA_MENU_RIGHT:
        team_switch = SOCCER_TEAM_BLUE;
        break;
    case PA_MENU_SELECT:
    {
        int nb_players = m_kart_view_info.size();
        for(int i=0 ; i < nb_players ; i++)
        {
            if(m_kart_view_info[i].local_player_id == playerId)
            {
                m_kart_view_info[i].confirmed = true;
                m_kart_view_info[i].view->setRotateContinuously( 0.0f );
                
                if(areAllKartsConfirmed())
                {
                    ButtonWidget*   bt_continue = getWidget<ButtonWidget>("continue");
                    bt_continue->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
                    bt_continue->setActivated();
                    
                    for(int i=0 ; i < nb_players ; i++)
                        race_manager->setLocalKartSoccerTeam(m_kart_view_info[i].local_player_id,
                                                             m_kart_view_info[i].team);
                }
                break;
            }
        }
    }
        break;
    default:
        break;
    }
    
    if(team_switch != SOCCER_TEAM_NONE)
    {
        // Find the corresponding kart view, update its team and update the layout
        int nb_players = m_kart_view_info.size();
        for(int i=0 ; i < nb_players ; i++)
        {
            if(m_kart_view_info[i].local_player_id == playerId)
            {
                m_kart_view_info[i].team = team_switch;
                updateKartViewsLayout();
                break;
            }
        }
    }
    
    return EVENT_BLOCK;
}

bool SoccerSetupScreen::areAllKartsConfirmed() const
{
    bool all_confirmed = true;
    int nb_players = m_kart_view_info.size();
    for(int i=0 ; i < nb_players ; i++)
    {
        if(!m_kart_view_info[i].confirmed)
        {
            all_confirmed = false;
            break;
        }
    }
    return all_confirmed;
}

void SoccerSetupScreen::updateKartViewsLayout()
{
    Widget* central_div = getWidget<Widget>("central_div");
    
    // Compute/get some dimensions
    const int vs_width = m_label_vs->m_w;
    const int nb_columns = 2;   // two karts maximum per column
    const int kart_area_width = (central_div->m_w - vs_width) / 2; // size of one half of the screen
    const int kart_view_size = kart_area_width/nb_columns;  // Size (width and height) of a kart view
    const int center_x = central_div->m_x + central_div->m_w/2;
    const int center_y = central_div->m_y + central_div->m_h/2;
    
    // Count the number of karts per team
    int nb_players = m_kart_view_info.size();
    int nb_karts_per_team[2] = {0,0};
    for(int i=0 ; i < nb_players ; i++)
        nb_karts_per_team[m_kart_view_info[i].team]++;
    
    // - number of rows displayed for each team = ceil(nb_karts_per_team[i] / nb_columns)
    const int nb_rows_per_team[2] = { (nb_karts_per_team[0] + nb_columns - 1) / nb_columns,
                                      (nb_karts_per_team[1] + nb_columns - 1) / nb_columns};
    // - where to start vertically
    const int start_y[2] = {center_y - nb_rows_per_team[0] * kart_view_size / 2,
                            center_y - nb_rows_per_team[1] * kart_view_size / 2};
    // - center of each half-screen
    const int center_x_per_team[2] = {  ( central_div->m_x                  + (center_x - vs_width) ) / 2,
                                        ( central_div->m_x+central_div->m_w + (center_x + vs_width) ) / 2,
                                     };
    
    // Update the layout of the 3D views for the karts
    int cur_kart_per_team[2] = {0,0};   // counters
    for(int i=0 ; i < nb_players ; i++)
    {
        const KartViewInfo& view_info = m_kart_view_info[i];
        const SoccerTeam    team = view_info.team;
        
        // Compute the position
        const int cur_row = cur_kart_per_team[team] / nb_columns;
        const int pos_y = start_y[team] + cur_row*kart_view_size;
        
        const int cur_col = cur_kart_per_team[team] % nb_columns;
        int nb_karts_in_this_row = (nb_karts_per_team[team] - cur_row*nb_columns) % nb_columns;
        if(nb_karts_in_this_row == 0)
            nb_karts_in_this_row = nb_columns;  // TODO: not sure of the computation here...
        const int pos_x = center_x_per_team[team] + cur_col*kart_view_size - nb_karts_in_this_row*kart_view_size/2;
        cur_kart_per_team[team]++;
        
        // Move the view
        view_info.view->move(pos_x, pos_y, kart_view_size, kart_view_size);
    }
}
