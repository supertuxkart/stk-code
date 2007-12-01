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

#include "help_menu.hpp"
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
/* For the first screen */
    WTOK_MSG1,
    WTOK_MSG2,
    WTOK_MSG3,
    WTOK_MSG4,
    WTOK_MSG5,

    WTOK_EMPTY,

    //Change this so it's not a static amount of keys that is supported
    WTOK_LEFT,
    WTOK_RIGHT,
    WTOK_ACCEL,
    WTOK_BRAKE,
    WTOK_WHEELIE,
    WTOK_RESCUE,
    WTOK_FIRE,
    WTOK_JUMP,

    WTOK_LEFTKEY,
    WTOK_RIGHTKEY,
    WTOK_ACCELKEY,
    WTOK_BRAKEKEY,
    WTOK_WHEELIEKEY,
    WTOK_RESCUEKEY,
    WTOK_FIREKEY,
    WTOK_JUMPKEY,

/* For the second screen */
    WTOK_MSG6,

    WTOK_ITEMIMG1, WTOK_ITEMTXT1,
    WTOK_ITEMIMG2, WTOK_ITEMTXT2,
    WTOK_ITEMIMG3, WTOK_ITEMTXT3,
    WTOK_ITEMIMG4, WTOK_ITEMTXT4,
    WTOK_ITEMIMG5, WTOK_ITEMTXT5,
    WTOK_ITEMIMG6, WTOK_ITEMTXT6,

    WTOK_FIRST_PAGE,
    WTOK_SECOND_PAGE,
    WTOK_QUIT
};

HelpMenu::HelpMenu()
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

    switch_to_first_screen();
}   // HelpMenu

//-----------------------------------------------------------------------------
HelpMenu::~HelpMenu()
{
    widget_manager->reset() ;

	if (m_box != NULL && m_silver_coin != NULL && m_gold_coin != NULL
        && m_banana != NULL )
	{
		ssgDeRefDelete(m_box);
		ssgDeRefDelete(m_silver_coin);
		ssgDeRefDelete(m_gold_coin);
		ssgDeRefDelete(m_banana);
	}

    delete m_context;

}   // ~HelpMenu

//-----------------------------------------------------------------------------
void HelpMenu::update(float dt)
{
    m_clock += dt * 40.0f;
    BaseGUI::update(dt);

    if (m_box != NULL && m_silver_coin != NULL && m_gold_coin != NULL
        && m_banana != NULL )
    {
        ssgContext* oldContext = ssgGetCurrentContext();
        m_context -> makeCurrent();

        glClear(GL_DEPTH_BUFFER_BIT);

#if 0
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glViewport ( 0, 0, viewport[2], viewport[3]);
        m_context -> setFOV ( 45.0f, 45.0f * viewport[2]/viewport[3] ) ;
        m_context -> setNearFar ( 0.05f, 1000.0f ) ;
#endif

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
void HelpMenu::switch_to_first_screen()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    const WidgetFontSize TEXT_SIZE = WGT_FNT_SML;

    widget_manager->set_initial_rect_state( SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK );
    widget_manager->set_initial_text_state( SHOW_TEXT, "", TEXT_SIZE, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    /*Help header*/
    widget_manager->add_wgt(WTOK_MSG1, 50, 7);
    widget_manager->set_wgt_text( WTOK_MSG1, _("Force your rivals bite *your* dust!") );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_MSG2, 60, 7);
    widget_manager->set_wgt_text( WTOK_MSG2, _("Grab blue boxes and coins") );

    widget_manager->add_wgt(WTOK_MSG3, 30, 7);
    widget_manager->set_wgt_text( WTOK_MSG3, _("Avoid bananas") );
    widget_manager->break_line();

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
    widget_manager->add_wgt(WTOK_EMPTY, 100, 15);
    widget_manager->hide_wgt_rect(WTOK_EMPTY);
    widget_manager->hide_wgt_text(WTOK_EMPTY);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_MSG4, 100, 10);
    widget_manager->set_wgt_text( WTOK_MSG4,
//Next line starts at column 0 to avoid spaces in the GUI
_("At high speeds wheelies drive you faster, but you can't steer. If you\n\
get stuck or fall too far, use the rescue button to get back on track."));
    widget_manager->break_line();

    /*Current key bindings*/
    widget_manager->add_wgt(WTOK_MSG5, 70, 7);
    widget_manager->set_wgt_text( WTOK_MSG5,
        _("Check the current key bindings for the first player"));
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_LEFT, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_LEFT, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_LEFT, sKartAction2String[KA_LEFT]);
    widget_manager->add_wgt(WTOK_LEFTKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_LEFTKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_LEFTKEY,
        user_config->getMappingAsString(0, (KartAction)KA_LEFT).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_RIGHT, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_RIGHT, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_RIGHT, sKartAction2String[KA_RIGHT]);
    widget_manager->add_wgt(WTOK_RIGHTKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_RIGHTKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_RIGHTKEY,
        user_config->getMappingAsString(0, (KartAction)KA_RIGHT).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_ACCEL, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_ACCEL, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_ACCEL, sKartAction2String[KA_ACCEL]);
    widget_manager->add_wgt(WTOK_ACCELKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_ACCELKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_ACCELKEY,
        user_config->getMappingAsString(0, (KartAction)KA_ACCEL).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_BRAKE, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_BRAKE, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_BRAKE, sKartAction2String[KA_BRAKE]);
    widget_manager->add_wgt(WTOK_BRAKEKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_BRAKEKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_BRAKEKEY,
        user_config->getMappingAsString(0, (KartAction)KA_BRAKE).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_WHEELIE, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_WHEELIE, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_WHEELIE, sKartAction2String[KA_WHEELIE]);
    widget_manager->add_wgt(WTOK_WHEELIEKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_WHEELIEKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_WHEELIEKEY,
        user_config->getMappingAsString(0, (KartAction)KA_WHEELIE).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_RESCUE, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_RESCUE, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_RESCUE, sKartAction2String[KA_RESCUE]);
    widget_manager->add_wgt(WTOK_RESCUEKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_RESCUEKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_RESCUEKEY,
        user_config->getMappingAsString(0, (KartAction)KA_RESCUE).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_FIRE, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_FIRE, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_FIRE, sKartAction2String[KA_FIRE]);
    widget_manager->add_wgt(WTOK_FIREKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_FIREKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_FIREKEY,
        user_config->getMappingAsString(0, (KartAction)KA_FIRE).c_str());
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_JUMP, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_JUMP, WGT_AREA_LFT );
    widget_manager->set_wgt_text(WTOK_JUMP, sKartAction2String[KA_JUMP]);
    widget_manager->add_wgt(WTOK_JUMPKEY, 20, 5);
    widget_manager->set_wgt_round_corners(WTOK_JUMPKEY, WGT_AREA_RGT );
    widget_manager->set_wgt_text(WTOK_JUMPKEY,
        user_config->getMappingAsString(0, (KartAction)KA_JUMP).c_str());
    widget_manager->break_line();

    /*Buttons at the bottom*/
    widget_manager->add_wgt(WTOK_SECOND_PAGE, 20, 7);
    widget_manager->set_wgt_text(WTOK_SECOND_PAGE, _("Next screen"));
    widget_manager->activate_wgt(WTOK_SECOND_PAGE);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_QUIT, 40, 7);
    widget_manager->set_wgt_text(WTOK_QUIT, _("Go back to the main menu"));
    widget_manager->activate_wgt(WTOK_QUIT);

    widget_manager->layout( WGT_AREA_TOP );
}

//-----------------------------------------------------------------------------
void HelpMenu::switch_to_second_screen()
{
    /* Delete 3D models from the first screen */
    ssgDeRefDelete(m_box); m_box = 0;
    ssgDeRefDelete(m_silver_coin); m_silver_coin = 0;
    ssgDeRefDelete(m_gold_coin); m_gold_coin = 0;
    ssgDeRefDelete(m_banana); m_banana = 0;

    /* Add the widgets */
    const bool SHOW_RECT = true;
    const WidgetFontSize TEXT_SIZE = WGT_FNT_SML;
    widget_manager->set_initial_rect_state( SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK );
    widget_manager->set_initial_text_state( false, "", TEXT_SIZE, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    widget_manager->add_wgt(WTOK_MSG6, 100, 8);
    widget_manager->set_wgt_text(WTOK_MSG6,
        _("To help you win, there are certain collectables you can grab:"));
    widget_manager->show_wgt_text( WTOK_MSG6 );
    widget_manager->break_line();

    /* Collectable images and descriptions */
    widget_manager->add_wgt(WTOK_ITEMIMG1, 10, 13);
    widget_manager->set_wgt_texture(WTOK_ITEMIMG1,
        collectable_manager->getIcon(COLLECT_MISSILE)->getState()->getTextureHandle());
    widget_manager->set_wgt_color(WTOK_ITEMIMG1, WGT_WHITE);
    widget_manager->show_wgt_texture(WTOK_ITEMIMG1);
    widget_manager->set_wgt_round_corners(WTOK_ITEMIMG1, WGT_AREA_NONE);

    widget_manager->add_wgt(WTOK_ITEMTXT1, 90, 13);
    widget_manager->set_wgt_text( WTOK_ITEMTXT1,
        _("Missile - fast stopper in a straight line"));
    widget_manager->show_wgt_text( WTOK_ITEMTXT1 );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_ITEMIMG2, 10, 13);
    widget_manager->set_wgt_texture(WTOK_ITEMIMG2,
        collectable_manager->getIcon(COLLECT_HOMING_MISSILE)->getState()->getTextureHandle());
    widget_manager->set_wgt_color(WTOK_ITEMIMG2, WGT_WHITE);
    widget_manager->show_wgt_texture( WTOK_ITEMIMG2 );
    widget_manager->set_wgt_round_corners(WTOK_ITEMIMG2, WGT_AREA_NONE);

    widget_manager->add_wgt(WTOK_ITEMTXT2, 90, 13);
    widget_manager->set_wgt_text( WTOK_ITEMTXT2,
        _("Homing missile - follows rivals, but is slower than the missile"));
    widget_manager->show_wgt_text( WTOK_ITEMTXT2 );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_ITEMIMG3, 10, 13);
    widget_manager->set_wgt_texture(WTOK_ITEMIMG3,
        collectable_manager->getIcon(COLLECT_SPARK)->getState()->getTextureHandle());
    widget_manager->set_wgt_color(WTOK_ITEMIMG3, WGT_WHITE);
    widget_manager->show_wgt_texture( WTOK_ITEMIMG3 );
    widget_manager->set_wgt_round_corners(WTOK_ITEMIMG3, WGT_AREA_NONE);

    widget_manager->add_wgt(WTOK_ITEMTXT3, 90, 13);
    widget_manager->set_wgt_text( WTOK_ITEMTXT3,
        _("Fuzzy blob/Spark - very slow, but bounces from walls"));
    widget_manager->show_wgt_text( WTOK_ITEMTXT3 );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_ITEMIMG4, 10, 13);
    widget_manager->set_wgt_texture(WTOK_ITEMIMG4,
        collectable_manager->getIcon(COLLECT_ZIPPER)->getState()->getTextureHandle());
    widget_manager->set_wgt_color(WTOK_ITEMIMG4, WGT_WHITE);
    widget_manager->show_wgt_texture( WTOK_ITEMIMG4 );
    widget_manager->set_wgt_round_corners(WTOK_ITEMIMG4, WGT_AREA_NONE);

    widget_manager->add_wgt(WTOK_ITEMTXT4, 90, 13);
    widget_manager->set_wgt_text( WTOK_ITEMTXT4,
        _("Zipper - speed boost"));
    widget_manager->show_wgt_text( WTOK_ITEMTXT4 );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_ITEMIMG5, 10, 13);
    widget_manager->set_wgt_texture(WTOK_ITEMIMG5,
        collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle());
    widget_manager->set_wgt_color(WTOK_ITEMIMG5, WGT_WHITE);
    widget_manager->show_wgt_texture( WTOK_ITEMIMG5 );
    widget_manager->set_wgt_round_corners(WTOK_ITEMIMG5, WGT_AREA_NONE);

    widget_manager->add_wgt(WTOK_ITEMTXT5, 90, 13);
    widget_manager->set_wgt_text( WTOK_ITEMTXT5,
        _("Parachute - slows down all karts in a better position!"));
    widget_manager->show_wgt_text( WTOK_ITEMTXT5 );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_ITEMIMG6, 10, 13);
    widget_manager->set_wgt_texture(WTOK_ITEMIMG6,
        collectable_manager->getIcon(COLLECT_ANVIL)->getState()->getTextureHandle());
    widget_manager->set_wgt_color(WTOK_ITEMIMG6, WGT_WHITE);
    widget_manager->show_wgt_texture( WTOK_ITEMIMG6 );
    widget_manager->set_wgt_round_corners(WTOK_ITEMIMG6, WGT_AREA_NONE);

    widget_manager->add_wgt(WTOK_ITEMTXT6, 90, 13);
    widget_manager->set_wgt_text( WTOK_ITEMTXT6,
        _("Anvil - slows down greatly the kart in the first position"));
    widget_manager->show_wgt_text( WTOK_ITEMTXT6 );
    widget_manager->break_line();

#ifdef USE_MAGNETS
    //Magnets are currently disabled.
#endif

    /*Buttons at the bottom*/
    widget_manager->add_wgt(WTOK_FIRST_PAGE, 25, 7);
    widget_manager->set_wgt_text(WTOK_FIRST_PAGE, _("Previous screen"));
    widget_manager->show_wgt_text( WTOK_FIRST_PAGE );
    widget_manager->activate_wgt(WTOK_FIRST_PAGE);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_QUIT, 40, 7);
    widget_manager->set_wgt_text(WTOK_QUIT, _("Go back to the main menu"));
    widget_manager->show_wgt_text( WTOK_QUIT );
    widget_manager->activate_wgt(WTOK_QUIT);

    widget_manager->layout( WGT_AREA_TOP );
}

//-----------------------------------------------------------------------------
void HelpMenu::select()
{
    switch ( widget_manager->get_selected_wgt() )
    {
        case WTOK_FIRST_PAGE:
            widget_manager->reset();
            switch_to_first_screen();
            break;

        case WTOK_SECOND_PAGE:
            widget_manager->reset();
            switch_to_second_screen();
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
