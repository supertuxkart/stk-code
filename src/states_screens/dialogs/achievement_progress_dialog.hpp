//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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


#ifndef HEADER_ACHIEVEMENT_PROGRESS_DIALOG_HPP
#define HEADER_ACHIEVEMENT_PROGRESS_DIALOG_HPP

#include "achievements/achievement.hpp"
#include "guiengine/modaldialog.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <vector>

namespace GUIEngine
{
    class IconButtonWidget;
    class LabelWidget;
    class ListWidget;
    class RibbonWidget;
}

/**
 * \brief Dialog that shows an achievement description and progress
 * \ingroup states_screens
 */
class AchievementProgressDialog : public GUIEngine::ModalDialog
{
private:
    Achievement *m_achievement;

    bool m_self_destroy;
    int m_depth;
    int m_row_counter;//Used in the recurisve table filling

    GUIEngine::ListWidget* m_progress_table;
    GUIEngine::LabelWidget* m_main_goal_description;
    GUIEngine::LabelWidget* m_main_goal_progress;

    GUIEngine::RibbonWidget* m_options_widget;

    GUIEngine::IconButtonWidget* m_ok_widget;

    void recursiveFillTable(AchievementInfo::goalTree &progress,
                            AchievementInfo::goalTree &reference, int depth);
    core::stringw niceGoalName(std::string internal_name);
public:
    AchievementProgressDialog(Achievement *achievement);
    // ------------------------------------------------------------------------
    ~AchievementProgressDialog() {}
    // ------------------------------------------------------------------------
    virtual void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    virtual void init();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal()                    { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    virtual bool onEscapePressed()
    {
        m_self_destroy = true;
        return false;
    }
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt);
};

#endif
