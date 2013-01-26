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
    
    // Compute some dimensions
    const core::dimension2d<u32>    vs_size = GUIEngine::getTitleFont()->getDimension( L"VS" );
    const int vs_width = (int)vs_size.Width;
    const int vs_height = (int)vs_size.Height;
    const int nb_columns = 2;   // two karts maximum per column
    const int kart_area_width = (central_div->m_w - vs_width) / 2; // size of one half of the screen
    const int kart_view_size = kart_area_width/nb_columns;  // Size (width and height) of a kart view
    const int center_x = central_div->m_x + central_div->m_w/2;
    const int center_y = central_div->m_y + central_div->m_h/2;
    
    // Choose kart teams + count
    int nb_players = race_manager->getNumLocalPlayers();
    int nb_karts_per_team[2] = {0,0};
    for(int i=0 ; i < nb_players ; i++)
    {
        SoccerTeam  team = i&1 ? SOCCER_TEAM_BLUE : SOCCER_TEAM_RED;
        nb_karts_per_team[team]++;
        race_manager->setLocalKartSoccerTeam(i, team);
    }
    
    // Compute some other dimensions/positions
    // - number of rows displayed for each team = ceil(nb_karts_per_team[i] / nb_columns)
    const int nb_rows_per_team[2] = { (nb_karts_per_team[0] + nb_columns) / nb_columns,
                                      (nb_karts_per_team[1] + nb_columns) / nb_columns};
    // - where to start vertically
    const int start_y[2] = {center_y - nb_rows_per_team[0] * kart_view_size / 2,
                            center_y - nb_rows_per_team[1] * kart_view_size / 2};
    // - center of each half-screen
    const int center_x_per_team[2] = {  ( central_div->m_x                  + (center_x - vs_width) ) / 2,
                                        ( central_div->m_x+central_div->m_w + (center_x + vs_width) ) / 2,
                                     };
    
    // Add "VS" label at the center of the rounded box
    m_label_vs = new LabelWidget(true, false);
    m_label_vs->m_x = center_x - vs_width/2;
    m_label_vs->m_y = center_y - vs_height/2;
    m_label_vs->m_w = vs_width;
    m_label_vs->m_h = vs_height;
    
    central_div->getChildren().push_back(m_label_vs);
    
    // Add the 3D views for the karts
    int cur_kart_per_team[2] = {0,0};   // counters
    for(int i=0 ; i < nb_players ; i++)
    {
        const RemoteKartInfo&   kart_info   = race_manager->getLocalKartInfo(i);
        const SoccerTeam        team        = kart_info.getSoccerTeam();
        const std::string&      kart_name   = kart_info.getKartName();
        
        const KartProperties*   props       = kart_properties_manager->getKart(kart_name);
        const KartModel&        kart_model  = props->getMasterKartModel();
        
        // Compute the position
        const int cur_row = cur_kart_per_team[team] / nb_columns;
        const int pos_y = start_y[team] + cur_row*kart_view_size;
        
        const int cur_col = cur_kart_per_team[team] % nb_columns;
        const int nb_karts_in_this_row = (nb_karts_per_team[team] - cur_row*nb_columns) % nb_columns;
        const int pos_x = center_x_per_team[team] + cur_col*kart_view_size - nb_karts_in_this_row*kart_view_size/2;
        cur_kart_per_team[team]++;
        
        // Add the view
        ModelViewWidget*    kart_view = new ModelViewWidget();
        kart_view->m_x = pos_x;
        kart_view->m_y = pos_y;
        kart_view->m_w = kart_view_size;
        kart_view->m_h = kart_view_size;
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
    }
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
}
