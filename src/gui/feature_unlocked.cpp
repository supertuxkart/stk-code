//  $Id: feature_unlocked.cpp 1305 2007-11-26 14:28:15Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "gui/feature_unlocked.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_CONTINUE,
    WTOK_DESCRIPTION
};

FeatureUnlocked::FeatureUnlocked()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialActivationState(false);
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED, WGT_FONT_GUI, WGT_WHITE, false );

    widget_manager->addWgt( WTOK_TITLE, 60, 10);
    widget_manager->showWgtRect( WTOK_TITLE );
    widget_manager->setWgtText( WTOK_TITLE, _("New Feature Unlocked"));
    widget_manager->setWgtTextSize( WTOK_TITLE, WGT_FNT_LRG);
    widget_manager->showWgtText( WTOK_TITLE );
    widget_manager->breakLine();

    m_new_features=unlock_manager->getUnlockedFeatures();
    assert(m_new_features.size()>0);
    unlock_manager->clearUnlocked();

    widget_manager->addWgt( WTOK_DESCRIPTION, 60, 30);
    widget_manager->showWgtRect( WTOK_DESCRIPTION );
    widget_manager->setWgtText( WTOK_DESCRIPTION, m_new_features[0]->getFeatureDescription());
    widget_manager->showWgtText( WTOK_DESCRIPTION );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_CONTINUE, 50, 7);
    widget_manager->showWgtRect(WTOK_CONTINUE);
    widget_manager->showWgtText(WTOK_CONTINUE);
    widget_manager->setWgtText(WTOK_CONTINUE, _("Continue"));
    widget_manager->activateWgt(WTOK_CONTINUE);

    widget_manager->layout(WGT_AREA_ALL);
}   // FeatureUnlocked

//-----------------------------------------------------------------------------
FeatureUnlocked::~FeatureUnlocked()
{
    widget_manager->reset();
}   // ~FeatureUnlocked

//-----------------------------------------------------------------------------

void FeatureUnlocked::select()
{
    assert(widget_manager->getSelectedWgt()==WTOK_CONTINUE);
    m_new_features.erase(m_new_features.begin());
    if(m_new_features.size()>0)
    {
        widget_manager->setWgtText( WTOK_DESCRIPTION, m_new_features[0]->getFeature());
    }
    else
    {
        menu_manager->popMenu();
    }
}   // select
