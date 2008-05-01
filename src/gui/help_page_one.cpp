//  $Id: help_menu.cpp 812 2006-10-07 11:43:57Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "help_page_one.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "user_config.hpp"
#include "player.hpp"
#include "collectable_manager.hpp"
#include "material.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_MSG1,
    WTOK_MSG2,
    WTOK_MSG3,
    WTOK_MSG4,
    WTOK_MSG5,

    WTOK_EMPTY,

    WTOK_FIRST_KEYNAME,
    WTOK_LAST_KEYNAME = WTOK_FIRST_KEYNAME + KA_LAST,

    WTOK_FIRST_KEYBINDING,
    WTOK_LAST_KEYBINDING = WTOK_FIRST_KEYBINDING + KA_LAST,

    WTOK_SECOND_PAGE,
    WTOK_QUIT
};

HelpPageOne::HelpPageOne()
{
    //The ssgContext constructor calls makeCurrent(), it has to be restored
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    m_box = 0;
    m_silver_coin = 0;
    m_gold_coin = 0;
    m_banana = 0;

	m_clock = 0;

    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    const WidgetFontSize TEXT_SIZE = WGT_FNT_SML;

    widget_manager->setInitialRectState( SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK );
    widget_manager->setInitialTextState( SHOW_TEXT, "", TEXT_SIZE,
        WGT_FONT_GUI, WGT_WHITE, false );

    /*Help header*/
    widget_manager->addWgt(WTOK_MSG1, 50, 7);
    widget_manager->setWgtText( WTOK_MSG1, _("Force your rivals bite *your* dust!") );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_MSG2, 60, 7);
    widget_manager->setWgtText( WTOK_MSG2, _("Grab blue boxes and coins") );

    widget_manager->addWgt(WTOK_MSG3, 30, 7);
    widget_manager->setWgtText( WTOK_MSG3, _("Avoid bananas") );
    widget_manager->breakLine();

    /*Rotating 3D models*/
	ssgEntity* hm = herring_manager->getHerringModel(HE_RED);
    ssgDeRefDelete(m_box);
    m_box = new ssgTransform;
    m_box->ref();
    m_box->addKid(hm);

    hm = herring_manager->getHerringModel(HE_SILVER);
    ssgDeRefDelete(m_silver_coin);
    m_silver_coin = new ssgTransform;
    m_silver_coin->ref();
    m_silver_coin->addKid(hm);

    hm = herring_manager->getHerringModel(HE_GOLD);
    ssgDeRefDelete(m_gold_coin);
    m_gold_coin = new ssgTransform;
    m_gold_coin->ref();
    m_gold_coin->addKid(hm);

    hm = herring_manager->getHerringModel(HE_GREEN);
    ssgDeRefDelete(m_banana);
    m_banana = new ssgTransform;
    m_banana->ref();
    m_banana->addKid(hm);

    /*Empty widget to cover the space for the 3D models*/
    widget_manager->addWgt(WTOK_EMPTY, 100, 15);
    widget_manager->hideWgtRect(WTOK_EMPTY);
    widget_manager->hideWgtText(WTOK_EMPTY);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_MSG4, 100, 10);
    widget_manager->setWgtText( WTOK_MSG4,
//Next line starts at column 0 to avoid spaces in the GUI
_("At high speeds wheelies drive you faster, but you can't steer. If you\n\
get stuck or fall too far, use the rescue button to get back on track."));
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_MSG5, 70, 7);
    widget_manager->setWgtText( WTOK_MSG5,
        _("Check the current key bindings for the first player"));
    widget_manager->breakLine();

    widget_manager->insertColumn();
    /*The keybindings are placed with loops because it allows to change the
     * number of kart actions without changing this screen. */
    for(int i = WTOK_FIRST_KEYNAME; i <= WTOK_LAST_KEYNAME; ++i)
    {
        widget_manager->addWgt( i, 20, 4 );
        widget_manager->setWgtRoundCorners( i, WGT_AREA_LFT );
        widget_manager->setWgtText( i,
            sKartAction2String[i - WTOK_FIRST_KEYNAME] );
    }
    widget_manager->breakLine();

    widget_manager->insertColumn();
    for(int i = WTOK_FIRST_KEYBINDING; i <= WTOK_LAST_KEYBINDING; ++i)
    {
        widget_manager->addWgt( i, 20, 4 );
        widget_manager->setWgtRoundCorners( i, WGT_AREA_RGT );
        widget_manager->setWgtText( i,
            user_config->getMappingAsString( 0,
            (KartAction)(i - WTOK_FIRST_KEYBINDING)).c_str());
    }
    widget_manager->breakLine();
    widget_manager->breakLine();

    /*Buttons at the bottom*/
    widget_manager->addWgt(WTOK_SECOND_PAGE, 20, 7);
    widget_manager->setWgtText(WTOK_SECOND_PAGE, _("Next screen"));
    widget_manager->activateWgt(WTOK_SECOND_PAGE);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_QUIT, 40, 7);
    widget_manager->setWgtText(WTOK_QUIT, _("Go back to the main menu"));
    widget_manager->activateWgt(WTOK_QUIT);

    widget_manager->layout( WGT_AREA_TOP );
}   // HelpPageOne

//-----------------------------------------------------------------------------
HelpPageOne::~HelpPageOne()
{
    widget_manager->reset();

	if (m_box != NULL && m_silver_coin != NULL && m_gold_coin != NULL
        && m_banana != NULL )
	{
		ssgDeRefDelete(m_box);
		ssgDeRefDelete(m_silver_coin);
		ssgDeRefDelete(m_gold_coin);
		ssgDeRefDelete(m_banana);
	}

    delete m_context;

}   // ~HelpPageOne

//-----------------------------------------------------------------------------
void HelpPageOne::update(float dt)
{
    m_clock += dt * 40.0f;
    BaseGUI::update(dt);

    if (m_box != NULL && m_silver_coin != NULL && m_gold_coin != NULL
        && m_banana != NULL )
    {
        ssgContext* oldContext = ssgGetCurrentContext();
        m_context -> makeCurrent();

        glClear(GL_DEPTH_BUFFER_BIT);

        sgCoord cam_pos;
        sgSetCoord(&cam_pos, 0, 0, 0, 0, 0, 0);
        m_context -> setCamera ( &cam_pos ) ;

        glEnable (GL_DEPTH_TEST);
        sgCoord trans;
        sgSetCoord(&trans, -4, 10, 1.85f, m_clock, 0, 0);
        m_box->setTransform (&trans);

        sgSetCoord(&trans, -2, 8, 1.5f, m_clock, 0, 0);
        m_silver_coin->setTransform (&trans);

        sgSetCoord(&trans, -1, 8, 1.5f, m_clock, 0, 0);
        m_gold_coin->setTransform (&trans);

        sgSetCoord(&trans, 5, 15, 3, m_clock, 0, 0);
        m_banana->setTransform (&trans);

        //glShadeModel(GL_SMOOTH);
        ssgCullAndDraw ( m_box ) ;
        ssgCullAndDraw ( m_silver_coin ) ;
        ssgCullAndDraw ( m_gold_coin ) ;
        ssgCullAndDraw ( m_banana ) ;
        glViewport ( 0, 0, user_config->m_width, user_config->m_height ) ;

        glDisable (GL_DEPTH_TEST);
        oldContext->makeCurrent();
    }
}

//-----------------------------------------------------------------------------
void HelpPageOne::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
        case WTOK_SECOND_PAGE:
            //This switches thee first page with the second page, so they
            //are not stacked by the menu manager, and the menu that called
            //this help is the one that gets called back when the next page
            //is popped.
            menu_manager->popMenu();
            menu_manager->pushMenu(MENUID_HELP2);
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
