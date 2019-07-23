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

#include "states_screens/dialogs/achievement_progress_dialog.hpp"

#include "achievements/achievement_info.hpp"
#include "config/player_manager.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// ----------------------------------------------------------------------------
AchievementProgressDialog::AchievementProgressDialog(Achievement *achievement)
                    : ModalDialog(0.95f,0.92f), m_achievement(achievement),
                      m_self_destroy(false)
{
    loadFromFile("online/achievement_progress_dialog.stkgui");
    
    m_depth = m_achievement->getInfo()->getDepth();
    assert (m_depth < 3);

    m_progress_table = getWidget<ListWidget>("progress-tree");
    assert(m_progress_table != NULL);
    m_progress_table->clear();
    
    m_main_goal_description = getWidget<LabelWidget>("main-goal-description");
    assert(m_main_goal_description != NULL);
    
    m_main_goal_progress = getWidget<LabelWidget>("main-goal-progress");
    assert(m_main_goal_progress != NULL);
     
    if (m_depth > 1)
    {
        std::vector<ListWidget::ListCell> row;
        for (int i = 1; i < m_depth; i++)
        {
            row.push_back(ListWidget::ListCell
                (_C("achievement_info", "Subgoals"), -1, 2, true));
            row.push_back(ListWidget::ListCell
                (_C("achievement_info", "Progress"), -1, 1, true));
        }
    
        m_progress_table->addItem(StringUtils::toString(0), row);        
    }
    
    m_row_counter = 1;
    recursiveFillTable(m_achievement->m_progress_goal_tree, 
                       m_achievement->m_achievement_info->m_goal_tree, 0);
}   // AchievementProgressDialog

// -----------------------------------------------------------------------------
/* Recursively fill the table with the goals */
void AchievementProgressDialog::recursiveFillTable(AchievementInfo::goalTree &progress,
                                                   AchievementInfo::goalTree &reference, int depth)
{
    if (progress.children.size() != 1)
    {
        int goal = -1; // Will be filled with goals or progress
        int target = -1;

        if (progress.type == "AND" ||
            progress.type == "AND-AT-ONCE" || 
            progress.type == "OR")
        {
            goal = m_achievement->computeFullfiledGoals(progress, reference);
            target = m_achievement->computeFullfiledGoals(reference, reference);
        }
        else
        {
            goal = m_achievement->computeGoalProgress(progress, reference);
            target = m_achievement->computeGoalProgress(reference, reference, true);
        }

        if (m_achievement->isAchieved() || goal > target)
        {
            goal = target;
        }
            
        if (depth == 0)
        {
            std::string temp = StringUtils::toString(goal) + "/" +
                               StringUtils::toString(target);
            core::stringw progress_string(temp.c_str());
            core::stringw goal_name = niceGoalName(progress.type);
                
            m_main_goal_description->setText(goal_name, false);
            m_main_goal_progress->setText(progress_string, false);
        }
        else
        {
            std::vector<ListWidget::ListCell> row;
            for (int i = 1; i < m_depth; i++)
            {
                //TODO : for sum, indicate if a subgoal counts towards or against it
                if (i == depth)
                {
                    std::string temp = StringUtils::toString(goal) + "/" +
                                       StringUtils::toString(target);
                    core::stringw progress_string(temp.c_str());
                    core::stringw goal_name = niceGoalName(progress.type);
                    row.push_back(ListWidget::ListCell
                        (goal_name, -1, 2, true));
                    row.push_back(ListWidget::ListCell
                        (progress_string, -1, 1, true));
                }
                else
                {
                    row.push_back(ListWidget::ListCell
                        (" ", -1, 2, true));
                    row.push_back(ListWidget::ListCell
                        (" ", -1, 1, true));
                }
            }
            m_progress_table->addItem(StringUtils::toString(m_row_counter), row);
            m_row_counter++;
        }

        for (unsigned int i = 0; i < progress.children.size(); i++)
        {
            recursiveFillTable(progress.children[i],reference.children[i],depth+1);
        }
    }
    // Skip the current node as it has no effect
    else
    {
        return recursiveFillTable(progress.children[0], reference.children[0], depth);
    }
} // recursiveFillTable

// -----------------------------------------------------------------------------
core::stringw AchievementProgressDialog::niceGoalName(std::string internal_name)
{
    core::stringw nice_name;
    // I18N: For achievements, a parent goal linking logically several subgoals
    if(internal_name=="AND") nice_name = _("Fulfill all the subgoals");
    // I18N: For achievements, a parent goal linking logically several subgoals
    if(internal_name=="AND-AT-ONCE") nice_name = _("Fulfill all the subgoals at the same time");
    // I18N: For achievements, a parent goal linking logically several subgoals
    if(internal_name=="OR") nice_name = _("Fulfill at least one subgoal");
    // I18N: For achievements, a parent goal linking logically several subgoals
    if(internal_name=="SUM") nice_name = _("The sum of the subgoals must reach the indicated value");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="won-races") nice_name = _("Races won");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="won-normal-races") nice_name = _("Normal races won");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="won-tt-races") nice_name = _("Time-trial races won");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="won-ftl-races") nice_name = _("Follow-the-Leader races won");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="cons-won-races") nice_name = _("Consecutive won races");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="cons-won-races-hard") nice_name = _("Consecutive won races in Expert or SuperTux");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="easy-started") nice_name = _("Novice races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="easy-finished") nice_name = _("Novice races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="medium-started") nice_name = _("Intermediate races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="medium-finished") nice_name = _("Intermediate races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="hard-started") nice_name = _("Expert races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="hard-finished") nice_name = _("Expert races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="best-started") nice_name = _("SuperTux races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="best-finished") nice_name = _("SuperTux races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="normal-started") nice_name = _("Normal races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="normal-finished") nice_name = _("Normal races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="tt-started") nice_name = _("Time-trial races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="tt-finished") nice_name = _("Time-trial races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="ftl-started") nice_name = _("Follow-the-Leader races started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="ftl-finished") nice_name = _("Follow-the-Leader races finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="three-strikes-started") nice_name = _("3 Strikes battles started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="three-strikes-finished") nice_name = _("3 Strikes battles finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="soccer-started") nice_name = _("Soccer matches started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="soccer-finished") nice_name = _("Soccer matches finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="egg-hunt-started") nice_name = _("Egg Hunts started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="egg-hunt-finished") nice_name = _("Egg Hunts finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="with-ghost-started") nice_name = _("Races started with a ghost replay");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="with-ghost-finished") nice_name = _("Races finished with a ghost replay");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="ctf-started") nice_name = _("Capture-the-Flag matches started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="ctf-finished") nice_name = _("Capture-the-Flag matches finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="ffa-started") nice_name = _("Free-for-All matches started");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="ffa-finished") nice_name = _("Free-for-All matches finished");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="powerup-used") nice_name = _("Powerups used");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="powerup-used-1race") { nice_name = _("Powerups used"); nice_name += _(" (1 race)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="bowling-hit") nice_name = _("Bowling ball hits");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="bowling-hit-1race") { nice_name = _("Bowling ball hits"); nice_name += _(" (1 race)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="swatter-hit") nice_name = _("Swatter hits");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="swatter-hit-1race") { nice_name = _("Swatter hits"); nice_name += _(" (1 race)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="all-hits") nice_name = _("All hits");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="all-hits-1race") { nice_name = _("All hits"); nice_name += _(" (1 race)"); }
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="hit-same-kart-1race") { nice_name = _("Hits against the same kart"); nice_name += _(" (1 race)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="banana") nice_name = _("Bananas collected");
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="banana-1race") { nice_name = _("Bananas collected"); nice_name += _(" (1 race)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="skidding") nice_name = _("Skidding");
    if (internal_name=="skidding-1race")
    {
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name = _("Skidding");
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name += _(" (1 race)");
    }
    if (internal_name=="skidding-1lap")
    {
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name = _("Skidding");
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name += _(" (1 lap)");
    }
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-started") { nice_name =_("Races started"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-finished") { nice_name =_("Races finished"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-won") { nice_name =_("Races won"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-finished-reverse") { nice_name =_("Reverse direction races finished"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-finished-alone") { nice_name =_("Races finished alone"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="less-laps") { nice_name =_("Races with less than the default lap number"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="more-laps") { nice_name =_("Races with more than the default lap number"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="twice-laps") { nice_name =_("Races with at least twice as much as the default lap number"); nice_name += _(" (maximum on one official track)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="egg-hunt-started") { nice_name =_("Egg hunts started"); nice_name += _(" (maximum on one official track)"); } 
    if (internal_name=="egg-hunt-finished")
    {
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name =_("Egg hunts finished");
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name += _(" (maximum on one official track)");
    }
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-started-all") { nice_name =_("Races started"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-finished-all") { nice_name =_("Races finished"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-won-all") { nice_name =_("Races won"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-finished-reverse-all") { nice_name =_("Reverse direction races finished"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="race-finished-alone-all") { nice_name =_("Races finished alone"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="less-laps-all") { nice_name =_("Races with less than the default lap number"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="more-laps-all") { nice_name =_("Races with more than the default lap number"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="twice-laps-all") { nice_name =_("Races with at least twice as much as the default lap number"); nice_name += _(" (official tracks matching the goal)"); } 
    // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
    if(internal_name=="egg-hunt-started-all") { nice_name =_("Egg hunts started"); nice_name += _(" (official tracks matching the goal)"); } 
    if(internal_name=="egg-hunt-finished-all")
    {
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name =_("Egg hunts finished");
        // I18N: A goal for achievements. If this text is in (), it's a precision added to multiple different goals.
        nice_name += _(" (official tracks matching the goal)");
    }

    return nice_name;
} // niceGoalName

// -----------------------------------------------------------------------------
void AchievementProgressDialog::beforeAddingWidgets()
{
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_ok_widget = getWidget<IconButtonWidget>("ok");
    assert(m_ok_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // beforeAddingWidgets

// -----------------------------------------------------------------------------
void AchievementProgressDialog::init()
{
    LabelWidget* header = getWidget<LabelWidget>("title");
    assert(header != NULL);
    core::stringw name = m_achievement->getInfo()->getName();
    header->setText(name, true /* expand as needed */);

    LabelWidget* description = getWidget<LabelWidget>("description");
    assert(description != NULL);
    core::stringw description_text = m_achievement->getInfo()->getDescription();
    description->setText(description_text, false);
}   // init

// -----------------------------------------------------------------------------
void AchievementProgressDialog::onUpdate(float dt)
{
    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    AchievementProgressDialog::processEvent(const std::string& source)
{

    if (source == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection =
            m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_ok_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
