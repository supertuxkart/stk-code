//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <sstream>
#include <string>

#include "char_sel.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/network_manager.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_EMPTY_DOWN,
    WTOK_EMPTY_UP,
    WTOK_TITLE,
    WTOK_UP,
    WTOK_DOWN,
    WTOK_MESSAGE,
    WTOK_QUIT,
    WTOK_EMPTY0 = 10,
    WTOK_NAME0  = 2000,
    WTOK_RACER0 = 3000
};

CharSel::CharSel(int whichPlayer)
        : m_kart(0), m_player_index(whichPlayer)
{
    // First time this is called --> switch client and server
    // to character barrier mode
    // initCharacter ... must be called to clean the kart_info
    // in the server (even when NW_NONE), otherwise ghost karts
    // etc. can appear.
    m_first_frame = true;
    if(network_manager->getState()==NetworkManager::NS_MAIN_MENU ||
        network_manager->getMode()==NetworkManager::NW_NONE)
        network_manager->initCharacterDataStructures();

    // For some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    // If m_player_index is 0 then this is a single player game or the first
    // player of a multiplayer game so we need to ensure that all karts are available.
    // If m_player_index is less than the number of elements in selected_karts then
    // the user is moving back through the menus and the last value in the vector
    // needs to be made available again.
    if (m_player_index == 0)
        kart_properties_manager->clearAllSelectedKarts();

    // Determine the list of all groups to display. A call to
    // kart_properties_manager->getAllGroups() will return even groups without
    // karts in it (e.g. because all karts are locked), so in this case we
    // don't want to display this list.
    m_all_groups = kart_properties_manager->getAllGroups();
    std::vector<std::string>::iterator it = m_all_groups.begin();
    while(it!=m_all_groups.end())
    {
        const std::vector<int> &kig=kart_properties_manager->getKartsInGroup(*it);
        bool can_be_deleted=true;
        for(unsigned int i=0; i<kig.size(); i++)
        {
            const KartProperties *k=kart_properties_manager->getKartById(kig[i]);
            if(!unlock_manager->isLocked(k->getIdent()))
            {
                can_be_deleted=false;
                break;
            }   // if isLocked
        }   // for i<kig.size
        if(can_be_deleted)
            it=m_all_groups.erase(it);
        else
            it++;
    }


    if (m_player_index < (int)kart_properties_manager->getNumSelectedKarts())
        kart_properties_manager->removeLastSelectedKart();

    char heading[MAX_MESSAGE_LENGTH];
    snprintf(heading, sizeof(heading), _("Player %d, choose a driver"),
             m_player_index + 1);
    widget_manager->addTitleWgt( WTOK_TITLE, 100, 10, heading );
    widget_manager->hideWgtRect(WTOK_TITLE);
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 2);
    widget_manager->breakLine();

    const int HEIGHT = 10;
    widget_manager->addEmptyWgt(WTOK_EMPTY_UP, computeIndent(0), HEIGHT/2);
    widget_manager->addTextButtonWgt(WTOK_UP, 20, HEIGHT/2, "^");
    widget_manager->breakLine();

    for (unsigned int i = 0; i < m_max_entries; i++)
    {
        // Depending on previously selected kart indx here might be a group,
        // i.e. indx is negative, and kp is not defined. To avoid this,
        // all widgets are _initialised_ with kart 0 (which surely exists),
        // before later calling switchCharacter, which will define the 
        // proper icons and texts.
        const KartProperties* kp = kart_properties_manager->getKartById(0);
        widget_manager->addEmptyWgt(WTOK_EMPTY0+i, computeIndent(i), HEIGHT );
        widget_manager->addImgButtonWgt(WTOK_RACER0 + i, 8, HEIGHT,
                                        kp->getIconFile() );
        widget_manager->addTextButtonWgt(WTOK_NAME0+i, 30, HEIGHT, "");
        widget_manager->setWgtTextSize(WTOK_NAME0+i, WGT_FNT_SML);
        widget_manager->breakLine();
    }

    widget_manager->addEmptyWgt(WTOK_EMPTY_DOWN, computeIndent(m_max_entries), HEIGHT/2);
    widget_manager->addTextButtonWgt(WTOK_DOWN, 20, HEIGHT/2, "v");

    switchGroup();  // select all karts from the currently selected group

    Widget *w=widget_manager->addTextWgt(WTOK_MESSAGE, 30, 7, "");
    w->setPosition(WGT_DIR_CENTER, 0, WGT_DIR_CENTER, 0);
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
        widget_manager->setWgtText(WTOK_MESSAGE, _("Waiting for server"));
    else
        widget_manager->setWgtText(WTOK_MESSAGE, _("Waiting for clients"));

    widget_manager->layout(WGT_AREA_RGT);

    m_current_kart = -1;

    const int LAST_KART = user_config->m_player[m_player_index].getLastKartId();
    if( LAST_KART != -1 && kart_properties_manager->kartAvailable(LAST_KART))// is LAST_KART not in vector of selected karts
    {
        int local_index = computeOffset();
        widget_manager->setSelectedWgt(WTOK_NAME0 +(m_max_entries-1)/2);
        switchCharacter(local_index);
    }
    else
    {
        m_offset = 0;
        switchCharacter(0);
    }
    if(network_manager->getState()==NetworkManager::NS_WAIT_FOR_AVAILABLE_CHARACTERS)
    {
        widget_manager->hideWgt(WTOK_TITLE, WTOK_DOWN);
        // Hide all widgets except the message widget
        for (unsigned int i = 0; i < m_max_entries; i++)
        {
            widget_manager->hideWgt(WTOK_NAME0+i);
            widget_manager->hideWgt(WTOK_RACER0+i);
        }
    }
    else
    {
        widget_manager->hideWgt(WTOK_MESSAGE);
        updateScrollPosition();
    }

    m_clock  = 0;

}   // CharSel

//-----------------------------------------------------------------------------
CharSel::~CharSel()
{
    widget_manager->reset();
    ssgDeRefDelete(m_kart);

    delete m_context;
}   // ~CharSel

//-----------------------------------------------------------------------------
void CharSel::updateScrollPosition()
{
    // Handle the special case of less karts (plus groups) than widgets.
    // Some of the widgets then need to be disabled.
    unsigned int start = 0, end=m_max_entries;
    if(m_index_avail_karts.size()<m_max_entries)
    {
        start = (unsigned int)(m_max_entries-m_index_avail_karts.size()+1)/2;
        end   = start+m_index_avail_karts.size()-1;
    }

    for(unsigned int i=0; i<m_max_entries; i++)
    {
        if(i<start || i>end)
        {
            widget_manager->hideWgt       (WTOK_NAME0 +i);
            widget_manager->hideWgtRect   (WTOK_RACER0+i);
            widget_manager->hideWgtTexture(WTOK_RACER0+i);
            continue;
        }

        // Otherwise enable the widgets again (just in case that they
        // had been disabled before)
        widget_manager->showWgt(WTOK_NAME0 +i);

        int indx = (i+m_offset)%m_index_avail_karts.size();
        indx     = m_index_avail_karts[indx];
        if(indx>=0)   // It's a kart, not a group
        {
            const KartProperties* kp= kart_properties_manager->getKartById(indx);

            if(unlock_manager->isLocked(kp->getIdent())) continue;
            widget_manager->setWgtText(WTOK_NAME0 + i, kp->getName());
            widget_manager->setWgtTexture(WTOK_RACER0 + i, kp->getIconFile() );
            widget_manager->showWgtTexture(WTOK_RACER0 + i);
            widget_manager->showWgtRect(WTOK_RACER0 + i);
        }
        else
        {
            widget_manager->setWgtText(WTOK_NAME0+i, m_all_groups[-indx-1]);
            widget_manager->hideWgtTexture(WTOK_RACER0 + i);
            widget_manager->hideWgtRect(WTOK_RACER0 + i);
        }
    }   // for i
    // set the 'selection changed' flag in the widget_manager, since update 
    // scroll position (when called on action up/down) will change the kart
    // to display, even though it's the same widget
    widget_manager->setSelectionChanged();
}   // updateScrollPosition

//-----------------------------------------------------------------------------
void CharSel::switchGroup()
{
    m_index_avail_karts.clear();
    const std::vector<int> &karts =
        kart_properties_manager->getKartsInGroup(user_config->m_kart_group);
    for(unsigned int i=0; i<karts.size(); i++)
    {   
        if(kart_properties_manager->kartAvailable(karts[i]))
        {
            m_index_avail_karts.push_back(karts[i]);
            kart_properties_manager->getKartById(karts[i])->getKartModel()->resetWheels();
        }
    }

    // Now add the groups, indicated by a negative number as kart index
    // ----------------------------------------------------------------
    for(int i =0; i<(int)m_all_groups.size(); i++)
    {
        // Only add groups other than the current one
        if(m_all_groups[i]!=user_config->m_kart_group) m_index_avail_karts.push_back(-i-1);
    }
    if(m_index_avail_karts.size()>=m_max_entries) 
    {
        m_offset          = 0;
        widget_manager->showWgtRect(WTOK_DOWN);
        widget_manager->showWgtText(WTOK_DOWN);
        widget_manager->showWgtRect(WTOK_UP);
        widget_manager->showWgtText(WTOK_UP);
    }
    else
    {
        // Less entries than maximum -> set m_offset to a negative number, so
        // that the actual existing entries are displayed 
        m_offset          = - (int)(m_max_entries-m_index_avail_karts.size())/2-1;
        widget_manager->hideWgtRect(WTOK_DOWN);
        widget_manager->hideWgtText(WTOK_DOWN);
        widget_manager->hideWgtRect(WTOK_UP);
        widget_manager->hideWgtText(WTOK_UP);
    }

}   // switchGroup

//-----------------------------------------------------------------------------
/** This forces a re-display of the available characters. It is used from the
 *  network manager when a character confirm message is received from the 
 *  server.
 */
void CharSel::updateAvailableCharacters()
{
    // This call computes the available characters (even though in this case
    // the group hasn't changed. 
    switchGroup();

    // This re-displays the characters (even though the scroll position has
    // not changed, one character might have been deleted).
    updateScrollPosition();
    // Forces a redraw of the model.
    m_current_kart = -1;
}   // updateAvailableCharacters

//-----------------------------------------------------------------------------

void CharSel::switchCharacter(int n)
{
    int indx=m_index_avail_karts[n];
        // if a group is hovered about, don't do anything
    if(indx<0) return;
    
    const KartProperties* kp= kart_properties_manager->getKartById(indx);
    if (m_current_kart != n && kp != NULL)
    {
        m_current_kart = n;
        ssgDeRefDelete(m_kart);
        m_kart = new ssgTransform;
        m_kart->ref();
        KartModel* kartentity = kp->getKartModel();

        m_kart->addKid(kartentity->getRoot());
    }
}   // switchCharacter

//-----------------------------------------------------------------------------
void CharSel::update(float dt)
{
    // If we are still waiting in the barrier, don't do anything
    if(network_manager->getState()==NetworkManager::NS_WAIT_FOR_AVAILABLE_CHARACTERS) 
    {
        widget_manager->update(dt);
        return;
    }

    // Are we still waiting for a confirmation?
    if(network_manager->getState()==NetworkManager::NS_WAIT_FOR_KART_CONFIRMATION)
    {
        widget_manager->update(dt);
        return;
    }

    // The selection was confirmed, proceed:
    if(network_manager->getState()==NetworkManager::NS_KART_CONFIRMED)
    {
        nextMenu();
        return;
    }

    if(m_first_frame)
    {
        // Switch group will update the list of selected karts, i.e. use the
        // information about kart availability just received from the server
        switchGroup();

        // Now try to select the previously selected kart again (in network
        // karts might be removed if they are not available on all computers,
        // so we have to recompute offset)
        computeOffset();

        // Now hide the message window and display the widgets:
        widget_manager->hideWgt(WTOK_MESSAGE);
        widget_manager->showWgt(WTOK_TITLE, WTOK_DOWN);
        // Hide all widgets except the message widget
        for (unsigned int i = 0; i < m_max_entries; i++)
        {
            widget_manager->showWgt(WTOK_NAME0+i);
            widget_manager->showWgt(WTOK_RACER0+i);
        }
        m_first_frame = false;
        updateScrollPosition();
        return;
    }

    m_clock += dt * 40.0f;

    if( widget_manager->selectionChanged() )
    {
        int token = widget_manager->getSelectedWgt() - WTOK_RACER0;
        if(token<0 || token>(int)m_index_avail_karts.size())
            token = widget_manager->getSelectedWgt() - WTOK_NAME0;
                switchCharacter((token+m_offset)%m_index_avail_karts.size());
    }

    if (m_kart != NULL)
    {
        ssgContext* oldContext = ssgGetCurrentContext();
        m_context -> makeCurrent();

        glClear(GL_DEPTH_BUFFER_BIT);

        // Puts the character in the center. Scaling is done by
        // applying a big camera FOV.
        int w = user_config->m_width;
        int h = user_config->m_height;
        glViewport ( 0, h*1/4, (int)(0.7f*w), (int)(0.7f*h));

        m_context -> setFOV ( 65.0f, 65.0f * h/w ) ;
        m_context -> setNearFar ( 0.05f, 1000.0f ) ;

        sgCoord cam_pos;
        sgSetCoord(&cam_pos, 0.5f, 0, 0.4f, 0, 0, 0);
        m_context -> setCamera ( &cam_pos ) ;

        glEnable (GL_DEPTH_TEST);
        sgCoord trans;
        sgSetCoord(&trans, 0, 3, -.4f, m_clock, 0, 0);
        m_kart->setTransform (&trans) ;
        //glShadeModel(GL_SMOOTH);
        ssgCullAndDraw ( m_kart ) ;
        glViewport ( 0, 0, user_config->m_width, user_config->m_height ) ;

        glDisable (GL_DEPTH_TEST);
        oldContext->makeCurrent();
    }

    widget_manager->update(dt);
}   // update

//----------------------------------------------------------------------------
/** This computes m_offset so that the previously selected kart is selected
 *  by default.
 */
int CharSel::computeOffset()
{
    int local_index = 0;
    int last_kart = user_config->m_player[m_player_index].getLastKartId();
    for(unsigned int i=0; i<m_index_avail_karts.size(); i++)
    {
        if(m_index_avail_karts[i]==last_kart)
        {
            local_index = i;
            break;
        }
    }
    m_offset = local_index - m_max_entries/2;
    if(m_offset<0) m_offset+=(int)m_index_avail_karts.size();
    return local_index;
}   // recmoputeOffset

//----------------------------------------------------------------------------
/** Pushes the next menu onto the stack. In case of split screen that might
 *  be another instance of the character selection menu.
 */
void CharSel::nextMenu()
{
    if(race_manager->getNumLocalPlayers() > 1 && 
       menu_manager->isCurrentMenu(MENUID_CHARSEL_P1))
    {
        menu_manager->pushMenu(MENUID_CHARSEL_P2);
        return;
    }

    if(race_manager->getNumLocalPlayers() > 2 &&
        menu_manager->isCurrentMenu(MENUID_CHARSEL_P2))
    {
        menu_manager->pushMenu(MENUID_CHARSEL_P3);
        return;
    }

    if (race_manager->getNumLocalPlayers() > 3 &&
        menu_manager->isCurrentMenu(MENUID_CHARSEL_P3))
    {
        menu_manager->pushMenu(MENUID_CHARSEL_P4);
        return;
    }

    // Last character selected
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
    {
        // Switch state to wait for race information
        network_manager->waitForRaceInformation();
        menu_manager->pushMenu(MENUID_START_RACE_FEEDBACK);
    }
    else
    {

        // The state of the server does not change now (so that it can keep
        // on handling client selections). Waiting for all client infos
        // happens in the start_race_feedback menu (which then triggers
        // sending the race info).
        if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
            menu_manager->pushMenu(MENUID_GRANDPRIXSELECT);
        else
            menu_manager->pushMenu(MENUID_TRACKSEL);
    }
}   // nextMenu

// ----------------------------------------------------------------------------
/** Handles widget selection.
 */
void CharSel::select()
{
    int wgt = widget_manager->getSelectedWgt();
    if(wgt==WTOK_UP)
    {
        m_offset--;
        if(m_offset < 0) m_offset = (int)m_index_avail_karts.size() - 1;
        updateScrollPosition();
        return;
    }
    if(wgt==WTOK_DOWN)
    {
        m_offset++;
        if( m_offset >= (int)m_index_avail_karts.size() ) m_offset=0;
        updateScrollPosition();
        return;
    }

    // Now it must be a character selection:
    // -------------------------------------
    int token = widget_manager->getSelectedWgt() - WTOK_RACER0;
    if(token<0 || token>(int)m_index_avail_karts.size())
    {
        token = widget_manager->getSelectedWgt() - WTOK_NAME0;
    }

    token = (token+m_offset) % (int)m_index_avail_karts.size();
    int kart_id = m_index_avail_karts[token];
    if(kart_id < 0) // group selected
    {
        user_config->m_kart_group = m_all_groups[-kart_id-1];
        switchGroup();
        // forces redraw of the model, otherwise (if m_current_kart=0) the new
        // model would not be displayed.
        m_current_kart = -1;
        switchCharacter(0);
        updateScrollPosition();
        return;
    }
    const KartProperties* KP = kart_properties_manager->getKartById(kart_id);
    if (!KP) return;

    race_manager->setLocalKartInfo(m_player_index, KP->getIdent());
    user_config->m_player[m_player_index].setLastKartId(kart_id);
    // Send the confirmation message to all clients.
    network_manager->sendCharacterSelected(m_player_index,
                                           KP->getIdent());
    // In non-network more or on the server add selected kart (token) to 
    // selected karts vector so it cannot be selected again
    if(network_manager->getMode()!=NetworkManager::NW_CLIENT)
    {
        kart_properties_manager->selectKart(kart_id);
        nextMenu();
    }
}   // select

//----------------------------------------------------------------------------
void CharSel::handle(GameAction action, int value)
{
    // Forward keypresses to basegui
    if(value) return BaseGUI::handle(action, value);

    if(action==GA_CURSOR_UP)
    {
        m_offset--;
        if(m_offset < 0) m_offset = (int)m_index_avail_karts.size() - 1;
        updateScrollPosition();
        return;

    }   // if cursor up
    if(action ==GA_CURSOR_DOWN)
    {
        m_offset++;
        if( m_offset >= (int)m_index_avail_karts.size() ) m_offset=0;
        updateScrollPosition();
        return;
    }   // if cursor down
    BaseGUI::handle(action, value);
}   // handle
