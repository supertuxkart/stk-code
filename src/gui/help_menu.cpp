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
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "user_config.hpp"
#include "player.hpp"
#include "collectable_manager.hpp"
#include "material.hpp"
#include "translation.hpp"

enum WidgetTokens {
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
    widgetSet -> delete_widget(m_menu_id) ;

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
    m_menu_id = widgetSet->vstack(0);

    //FIXME: if an hstack has no items, it segfaults
    const int HS1 = widgetSet->hstack(m_menu_id);
    widgetSet -> filler(HS1);
    widgetSet -> label(HS1, _("Force your rivals bite *your* dust!"), GUI_SML);
    widgetSet -> filler(HS1);

    const int HS2 = widgetSet->harray(m_menu_id);
    widgetSet->label(HS2, _("Avoid bananas"), GUI_SML);
    widgetSet->label(HS2, _("Grab blue boxes and coins"), GUI_SML);

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

    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);

    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);
    widgetSet -> filler(m_menu_id);

    widgetSet->multi(m_menu_id,
//Next line starts at column 0 to avoid spaces in the GUI
_("At high speeds wheelies drive you faster, but you can't steer. If you\n\
get stuck or fall too far, use the rescue button to get back on track."),
        GUI_SML);

    widgetSet -> filler(m_menu_id);

    widgetSet->label(m_menu_id,
                     _("Check the current keys bindings for the first player:"),
                     GUI_SML);

    const int HS3       = widgetSet->hstack(m_menu_id);
    widgetSet -> filler(HS3);
    const int CHANGE_ID = widgetSet->varray(HS3);
    const int LABEL_ID  = widgetSet->varray(HS3);
    widgetSet -> filler(HS3);

    for(int i = KC_LEFT; i <= KC_LAST; i++)
    {
        //FIXME: this is temporal, just while the jumping is disabled.
        if(i == KC_JUMP) continue;

        // *sigh* widget set stores only pointer to strings, so
        // to make sure that all key-strings are permanent, they
        // are assigned to an array m_all_keys within this object.
        m_all_keys[i]=user_config->getInputAsString(0, (KartActions)i);
        widgetSet->label(LABEL_ID,  sKartAction2String[i], GUI_SML, GUI_LFT);
        widgetSet->label(CHANGE_ID, m_all_keys[i].c_str(), GUI_SML, GUI_RGT);
    }
    widgetSet->start(m_menu_id,_("Next screen"), GUI_SML, WTOK_SECOND_PAGE);
    widgetSet->state(m_menu_id,_("Go back to the main menu"), GUI_SML, WTOK_QUIT);
    widgetSet->layout(m_menu_id, 0, 0);

}

//-----------------------------------------------------------------------------
void HelpMenu::switch_to_second_screen()
{

    ssgDeRefDelete(m_box); m_box = 0;
    ssgDeRefDelete(m_silver_coin); m_silver_coin = 0;
    ssgDeRefDelete(m_gold_coin); m_gold_coin = 0;
    ssgDeRefDelete(m_banana); m_banana = 0;

    m_menu_id = widgetSet->vstack(0);

    widgetSet->label(m_menu_id,
                     _("To help you win, there are certain collectables you can grab:"),
        GUI_SML);

    const int HA        = widgetSet->hstack(m_menu_id);
    const int LABEL_ID  = widgetSet->varray(HA);
    const int IMAGE_ID  = widgetSet->vstack(HA);

    const int ICON_SIZE = 64;

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_MISSILE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Missile - fast stopper in a straight line"), GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_HOMING_MISSILE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Homing missile - follows rivals, but is slower than the missile"), GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_SPARK)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Fuzzy blob/Spark - very slow, but bounces from walls"), GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_ZIPPER)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Zipper - speed boost"), GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Parachute - slows down all karts in a better position!"), GUI_SML);

    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_ANVIL)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Anvil - slows down greatly the kart in the first position"), GUI_SML);

#ifdef USE_MAGNETS
    widgetSet->image(IMAGE_ID, collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle(),
        ICON_SIZE, ICON_SIZE, GUI_NONE);
    widgetSet->label(LABEL_ID, _("Missile - fast stopper in a straight line"), GUI_SML);
#endif

    widgetSet->start(m_menu_id,_("Previous screen"), GUI_SML, WTOK_FIRST_PAGE);
    widgetSet->state(m_menu_id,_("Go back to the main menu"), GUI_SML, WTOK_QUIT);
    widgetSet->layout(m_menu_id, 0, 0);
}

//-----------------------------------------------------------------------------
void HelpMenu::select()
{
    switch( widgetSet->get_token (widgetSet->click()))
    {
        case WTOK_FIRST_PAGE:
            widgetSet -> delete_widget(m_menu_id) ;
            switch_to_first_screen();
            break;

        case WTOK_SECOND_PAGE:
            widgetSet -> delete_widget(m_menu_id) ;
            switch_to_second_screen();
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
