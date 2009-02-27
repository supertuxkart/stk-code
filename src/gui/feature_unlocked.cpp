//  $Id: feature_unlocked.cpp 1305 2007-11-26 14:28:15Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "gui/feature_unlocked.hpp"

#include "challenges/unlock_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "utils/translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_CONTINUE,
    WTOK_DESCRIPTION
};

FeatureUnlocked::FeatureUnlocked()
{
    widget_manager->switchOrder();
    widget_manager->addTitleWgt( WTOK_TITLE, 60, 10,
        _("New Feature Unlocked"));
    widget_manager->hideWgtRect(WTOK_TITLE);
    
    m_new_features=unlock_manager->getUnlockedFeatures();
    assert(m_new_features.size()>0);
    unlock_manager->clearUnlocked();

    widget_manager->addTextWgt( WTOK_DESCRIPTION, 60, 30,
        m_new_features[0]->getUnlockedMessage());

    widget_manager->addTextButtonWgt(WTOK_CONTINUE, 50, 7,
        _("Continue"));

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
        widget_manager->setWgtText( WTOK_DESCRIPTION, m_new_features[0]->getUnlockedMessage() );
    }
    else
    {
        menu_manager->popMenu();
    }
}   // select
