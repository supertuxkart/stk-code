//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2013 Lionel Fuentes
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

#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/state_manager.hpp"


using namespace GUIEngine;
DEFINE_SCREEN_SINGLETON( SoccerSetupScreen );

#define KART_CONTINUOUS_ROTATION_SPEED      35.f
#define KART_CONFIRMATION_ROTATION_SPEED    4.f
#define KART_CONFIRMATION_TARGET_ANGLE      10.f

// -----------------------------------------------------------------------------

SoccerSetupScreen::SoccerSetupScreen() : Screen("soccer_setup.stkgui")
{
}

// -----------------------------------------------------------------------------

void SoccerSetupScreen::loadedFromFile()
{
}

// ----------------------------------------------------------------------------
void SoccerSetupScreen::eventCallback(Widget* widget, const std::string& name,
                                      const int playerID)
{
    if(m_schedule_continue)
        return;

    if(name == "continue")
    {
        int nb_players = (int)m_kart_view_info.size();

        if (getNumKartsInTeam(SOCCER_TEAM_RED) == 0 ||
            getNumKartsInTeam(SOCCER_TEAM_BLUE) == 0)
        {
            for(int i=0 ; i < nb_players ; i++)
            {
                if (!m_kart_view_info[i].confirmed)
                {
                    m_kart_view_info[i].view->setBadge(BAD_BADGE);
                }
            }
            SFXManager::get()->quickSound( "anvil" );
            return;
        }
        else if(!areAllKartsConfirmed())
        {
            for(int i=0 ; i < nb_players ; i++)
            {
                if (!m_kart_view_info[i].confirmed)
                {
                    m_kart_view_info[i].confirmed = true;
                    m_kart_view_info[i].view->setRotateTo( KART_CONFIRMATION_TARGET_ANGLE, KART_CONFIRMATION_ROTATION_SPEED );
                    m_kart_view_info[i].view->setBadge(OK_BADGE);
                }
            }
            SFXManager::get()->quickSound( "wee" );
            m_schedule_continue = true;
        }
        else
        {
            m_schedule_continue = true;
        }

        if(getWidget<SpinnerWidget>("goalamount")->isActivated())
            race_manager->setMaxGoal(getWidget<SpinnerWidget>("goalamount")->getValue());
        else
            race_manager->setTimeTarget((float)getWidget<SpinnerWidget>("timeamount")->getValue()*60);

        input_manager->setMasterPlayerOnly(true);
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "time_enabled")
    {
        CheckBoxWidget* timeEnabled = dynamic_cast<CheckBoxWidget*>(widget);
        if(timeEnabled->getState())
        {
            getWidget<SpinnerWidget>("goalamount")->setDeactivated();
            getWidget<SpinnerWidget>("timeamount")->setActivated();
        }
        else
        {
            getWidget<SpinnerWidget>("timeamount")->setDeactivated();
            getWidget<SpinnerWidget>("goalamount")->setActivated();
        }
    }
}   // eventCallback

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
    LabelWidget*    label_vs = getWidget<LabelWidget>("vs");
    label_vs->m_x = center_x - vs_width/2;
    label_vs->m_y = center_y - vs_height/2;
    label_vs->m_w = vs_width;
    label_vs->m_h = vs_height;

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
        kart_view->setRotateContinuously( KART_CONTINUOUS_ROTATION_SPEED );

        kart_view->update(0);

        central_div->getChildren().push_back(kart_view);

        // Record info about it for further update
        KartViewInfo    info;
        info.view            = kart_view;
        info.confirmed       = false;
        info.local_player_id = i;
        info.team            = i&1 ? SOCCER_TEAM_BLUE : SOCCER_TEAM_RED;
        m_kart_view_info.push_back(info);
        race_manager->setLocalKartSoccerTeam(i, info.team);
    }

    // Update layout
    updateKartViewsLayout();
}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void SoccerSetupScreen::init()
{
    m_schedule_continue = false;

    Screen::init();
    
    if (UserConfigParams::m_num_goals <= 0)
        UserConfigParams::m_num_goals = 3;
        
    if (UserConfigParams::m_soccer_time_limit <= 0)
        UserConfigParams::m_soccer_time_limit = 3;

    SpinnerWidget*  goalamount = getWidget<SpinnerWidget>("goalamount");
    goalamount->setValue(UserConfigParams::m_num_goals);
    goalamount->setActivated();

    SpinnerWidget* timeAmount = getWidget<SpinnerWidget>("timeamount");
    timeAmount->setValue(UserConfigParams::m_soccer_time_limit);
    timeAmount->setDeactivated();

    CheckBoxWidget* timeEnabled = getWidget<CheckBoxWidget>("time_enabled");
    timeEnabled->setState(false);

    // Set focus on "continue"
    ButtonWidget* bt_continue = getWidget<ButtonWidget>("continue");
    bt_continue->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    // We need players to be able to choose their teams
    input_manager->setMasterPlayerOnly(false);

    // This flag will cause that a 'fire' event will be mapped to 'select' (if
    // 'fire' is not assigned to a GUI event). This is done to support the old
    // way of player joining by pressing 'fire' instead of 'select'.
    input_manager->getDeviceManager()->mapFireToSelect(true);
}   // init

// -----------------------------------------------------------------------------
void SoccerSetupScreen::tearDown()
{
    Widget* central_div = getWidget<Widget>("central_div");

    // Reset the 'map fire to select' option of the device manager
    input_manager->getDeviceManager()->mapFireToSelect(false);
    
    UserConfigParams::m_num_goals = getWidget<SpinnerWidget>("goalamount")->getValue();
    UserConfigParams::m_soccer_time_limit = getWidget<SpinnerWidget>("timeamount")->getValue();

    // Remove all ModelViewWidgets we created manually
    PtrVector<Widget>&  children = central_div->getChildren();
    for(int i = children.size()-1 ; i >= 0 ; i--)
    {
        if(children[i].getType() == WTYPE_MODEL_VIEW)
            children.erase(i);
    }
    m_kart_view_info.clear();

    Screen::tearDown();
}   // tearDown

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation SoccerSetupScreen::filterActions(PlayerAction action,
                                                             int deviceID,
                                                             const unsigned int value,
                                                             Input::InputType type,
                                                             int playerId)
{
    if(m_schedule_continue)
        return EVENT_BLOCK;

    ButtonWidget*   bt_continue = getWidget<ButtonWidget>("continue");
    GUIEngine::EventPropagation result = EVENT_LET;
    SoccerTeam  team_switch = SOCCER_TEAM_NONE;
    int nb_players = (int)m_kart_view_info.size();

    switch(action)
    {
    case PA_MENU_LEFT:
        if (bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            m_kart_view_info[playerId].confirmed == false)
        {
            team_switch = SOCCER_TEAM_RED;

            for(int i=0 ; i < nb_players ; i++)
            {
                m_kart_view_info[i].view->unsetBadge(BAD_BADGE);
            }
        }
        break;
    case PA_MENU_RIGHT:
        if (bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            m_kart_view_info[playerId].confirmed == false)
        {
            team_switch = SOCCER_TEAM_BLUE;

            for(int i=0 ; i < nb_players ; i++)
            {
                m_kart_view_info[i].view->unsetBadge(BAD_BADGE);
            }
        }
        break;
    case PA_MENU_UP:
        if (playerId != PLAYER_ID_GAME_MASTER)
            return EVENT_BLOCK;
        break;
    case PA_MENU_DOWN:
        if (playerId != PLAYER_ID_GAME_MASTER)
            return EVENT_BLOCK;
        break;
    case PA_MENU_SELECT:
    {
        if (!bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) ||
            areAllKartsConfirmed())
        {
            return result;
        }

        if (bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            m_kart_view_info[playerId].confirmed)
        {
            return EVENT_BLOCK;
        }

        if (getNumConfirmedKarts() > nb_players-2 &&
           (getNumKartsInTeam(SOCCER_TEAM_RED) == 0 ||
            getNumKartsInTeam(SOCCER_TEAM_BLUE) == 0))
        {
            SFXManager::get()->quickSound( "anvil" );
            m_kart_view_info[playerId].view->setBadge(BAD_BADGE);
        }
        else
        {
            // Confirm team selection
            m_kart_view_info[playerId].confirmed = true;
            m_kart_view_info[playerId].view->setRotateTo( KART_CONFIRMATION_TARGET_ANGLE, KART_CONFIRMATION_ROTATION_SPEED );
            m_kart_view_info[playerId].view->setBadge(OK_BADGE);
            m_kart_view_info[playerId].view->unsetBadge(BAD_BADGE);
            SFXManager::get()->quickSound( "wee" );
        }
        return EVENT_BLOCK;
    }
    case PA_MENU_CANCEL:
    {
        if (!bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            playerId == PLAYER_ID_GAME_MASTER)
        {
            return result;
        }

        // Un-confirm team selection
        m_kart_view_info[playerId].confirmed = false;
        m_kart_view_info[playerId].view->setRotateContinuously( KART_CONTINUOUS_ROTATION_SPEED );
        m_kart_view_info[playerId].view->unsetBadge(OK_BADGE);

        for(int i=0 ; i < nb_players ; i++)
        {
            m_kart_view_info[i].view->unsetBadge(BAD_BADGE);
        }

        return EVENT_BLOCK;
    }
    default:
        break;
    }
     
    if(team_switch != SOCCER_TEAM_NONE) // A player wants to change his team?
    {
        race_manager->setLocalKartSoccerTeam(playerId, team_switch);
        m_kart_view_info[playerId].team = team_switch;
        updateKartViewsLayout();
    }
    


    return result;
}   // filterActions

// -----------------------------------------------------------------------------
void SoccerSetupScreen::onUpdate(float delta)
{
    int nb_players = (int)m_kart_view_info.size();
    
    if(m_schedule_continue)
    {
        for(int i=0 ; i < nb_players ; i++)
        {
            if (m_kart_view_info[i].view->isRotating() == true)
                return;
        }
        m_schedule_continue = false;
        ArenasScreen::getInstance()->push();
    }
}   // onUPdate

// ----------------------------------------------------------------------------
bool SoccerSetupScreen::areAllKartsConfirmed() const
{
    bool all_confirmed = true;
    int nb_players = (int)m_kart_view_info.size();
    for(int i=0 ; i < nb_players ; i++)
    {
        if(!m_kart_view_info[i].confirmed)
        {
            all_confirmed = false;
            break;
        }
    }
    return all_confirmed;
}   // areAllKartsConfirmed

// ----------------------------------------------------------------------------
int SoccerSetupScreen::getNumKartsInTeam(int team)
{
    int karts_in_team = 0;
    int nb_players = (int)m_kart_view_info.size();
    for(int i=0 ; i < nb_players ; i++)
    {
        if(m_kart_view_info[i].team == team)
            karts_in_team++;
    }
    return karts_in_team;
}   // getNumKartsInTeam

// -----------------------------------------------------------------------------
int SoccerSetupScreen::getNumConfirmedKarts()
{
    int confirmed_karts = 0;
    int nb_players = (int)m_kart_view_info.size();
    for(int i=0 ; i < nb_players ; i++)
    {
        if(m_kart_view_info[i].confirmed == true)
            confirmed_karts++;
    }
    return confirmed_karts;
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::updateKartViewsLayout()
{
    Widget* central_div = getWidget<Widget>("central_div");

    // Compute/get some dimensions
    LabelWidget*    label_vs = getWidget<LabelWidget>("vs");
    const int vs_width = label_vs->m_w;
    const int nb_columns = 2;   // two karts maximum per column
    const int kart_area_width = (central_div->m_w - vs_width) / 2; // size of one half of the screen
    const int kart_view_size = kart_area_width/nb_columns;  // Size (width and height) of a kart view
    const int center_x = central_div->m_x + central_div->m_w/2;
    const int center_y = central_div->m_y + central_div->m_h/2;

    // Count the number of karts per team
    int nb_players = (int)m_kart_view_info.size();
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
        if(nb_karts_in_this_row == 0 || nb_karts_per_team[team] > 1)
            nb_karts_in_this_row = nb_columns;  // TODO: not sure of the computation here...
        const int pos_x = center_x_per_team[team] + cur_col*kart_view_size - nb_karts_in_this_row*kart_view_size/2;
        cur_kart_per_team[team]++;

        // Move the view
        view_info.view->move(pos_x, pos_y, kart_view_size, kart_view_size);
    }
}   // updateKartViewsLayout

