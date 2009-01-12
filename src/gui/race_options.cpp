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

#include "race_manager.hpp"
#include "race_options.hpp"
#include "widget_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "material_manager.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_DIFFICULTY_TITLE,
    WTOK_DIFFICULTY_UP,
    WTOK_DIFFICULTY,
    WTOK_DIFFICULTY_DOWN,

    WTOK_KARTS_TITLE,
    WTOK_KARTS_UP,
    WTOK_KARTS,
    WTOK_KARTS_DOWN,

    WTOK_LAPS_TITLE,
    WTOK_LAPS_UP,
    WTOK_LAPS,
    WTOK_LAPS_DOWN,

    WTOK_START,

    WTOK_QUIT
};

RaceOptions::RaceOptions() 
{

    m_difficulty=user_config->getDefaultDifficulty();
    m_num_laps=user_config->getDefaultNumLaps();
    // Determine the minimum number of karts
    m_min_karts = (int)race_manager->getNumPlayers();
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {
        // if playing follow the leader single mode, there should be at
        // least one opponent in addition to the leader
        m_min_karts += (race_manager->getNumPlayers()==1 ? 2 : 1);
    }
    m_num_karts=std::max(user_config->getDefaultNumKarts(), m_min_karts);


    const int DESC_WIDTH=48;
    const int ITEM_WIDTH=35;

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 10, 20);
    widget_manager->breakLine();
    
    // Difficulty
    // ==========
    // if there is no AI, no point asking for its difficulty...
    // There's also no point asking the player for the amount of karts
    // since tt will always be the same as the number of human players
    if(!RaceManager::isBattleMode( race_manager->getMinorMode() ))
    {
        widget_manager->addTextWgt( WTOK_DIFFICULTY_TITLE, DESC_WIDTH, 7, _("Difficulty") );
        widget_manager->hideWgtRect(WTOK_DIFFICULTY_TITLE);
        widget_manager->setWgtTextSize(WTOK_DIFFICULTY_TITLE, WGT_FNT_LRG);
        widget_manager->addTextButtonWgt( WTOK_DIFFICULTY_DOWN, 3, 7, " < " );
        
        widget_manager->addTextWgt( WTOK_DIFFICULTY, ITEM_WIDTH, 7, getDifficultyString(m_difficulty));
        widget_manager->setWgtBorderPercentage( WTOK_DIFFICULTY, 10 );
        widget_manager->showWgtBorder( WTOK_DIFFICULTY );
        widget_manager->hideWgtRect( WTOK_DIFFICULTY );
        
        widget_manager->addTextButtonWgt( WTOK_DIFFICULTY_UP, 3, 7, " > " );
        
        widget_manager->breakLine();
        
        // Number of karts
        // ===============
        widget_manager->addTextWgt( WTOK_KARTS_TITLE, DESC_WIDTH, 7, _("Number of karts") );
        widget_manager->hideWgtRect(WTOK_KARTS_TITLE);
        widget_manager->setWgtTextSize(WTOK_KARTS_TITLE, WGT_FNT_LRG);
        widget_manager->addTextButtonWgt( WTOK_KARTS_DOWN, 3, 7, " < " );
        
        char string_num_karts[MAX_MESSAGE_LENGTH];
        snprintf(string_num_karts, MAX_MESSAGE_LENGTH, "%d", m_num_karts);
        widget_manager->addTextWgt( WTOK_KARTS, ITEM_WIDTH, 7, string_num_karts );
        widget_manager->setWgtBorderPercentage( WTOK_KARTS, 10 );
        widget_manager->showWgtBorder( WTOK_KARTS );
        widget_manager->hideWgtRect( WTOK_KARTS );
        
        widget_manager->addTextButtonWgt( WTOK_KARTS_UP, 3, 7, " > " );
        
        widget_manager->breakLine();
    }
    
    // Number of laps
    // ==============
    if( race_manager->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX   &&
        RaceManager::modeHasLaps( race_manager->getMinorMode() ) )
    {
        widget_manager->addTextWgt( WTOK_LAPS_TITLE, DESC_WIDTH, 7, _("Number of laps") );
        widget_manager->hideWgtRect(WTOK_LAPS_TITLE);
        widget_manager->setWgtTextSize(WTOK_LAPS_TITLE, WGT_FNT_LRG);
        widget_manager->addTextButtonWgt( WTOK_LAPS_DOWN, 3, 7, " < " );

        char string_num_laps[MAX_MESSAGE_LENGTH];
        snprintf(string_num_laps, MAX_MESSAGE_LENGTH, "%d", m_num_laps);
        widget_manager->addTextWgt( WTOK_LAPS, ITEM_WIDTH, 7, string_num_laps );
        widget_manager->setWgtBorderPercentage( WTOK_LAPS, 10 );
        widget_manager->showWgtBorder( WTOK_LAPS );
        widget_manager->hideWgtRect( WTOK_LAPS );

        widget_manager->addTextButtonWgt( WTOK_LAPS_UP, 3, 7, " > " );
    }
    widget_manager->breakLine();
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 10, 10 );
    widget_manager->breakLine();

    // Bottom buttons
    // ==============
    widget_manager->addTextButtonWgt( WTOK_START, 60, 7, _("Start race") );
    widget_manager->setWgtBorderPercentage( WTOK_START, 20 );
    widget_manager->setWgtBorderColor( WTOK_START, WGT_TRANS_BLUE );
    widget_manager->showWgtBorder( WTOK_START );

    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 10, 30);
    widget_manager->breakLine();
    
    widget_manager->addTextButtonWgt( WTOK_QUIT, 60, 7, _("Press <ESC> to go back") );

    widget_manager->layout(WGT_AREA_ALL);
    
    // Select 'start' by default.
    widget_manager->setSelectedWgt( WTOK_START );
    
    // hack. in battle mode this screen is totally useless. so I'm calling 'select'
    // to select the start button, so the screen is entirely skipped (FIXME - find cleaner way)
    if(RaceManager::isBattleMode( race_manager->getMinorMode() )) select();
        
}   // RaceOptions

//-----------------------------------------------------------------------------
RaceOptions::~RaceOptions()
{
    widget_manager->reset();
}   // ~RaceOptions

//-----------------------------------------------------------------------------
void RaceOptions::update(float dt)
{
    BaseGUI::update(dt);
}   // update
//-----------------------------------------------------------------------------
void RaceOptions::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
        case WTOK_DIFFICULTY_UP:
            if( m_difficulty == RaceManager::RD_HARD )
            {
                m_difficulty = RaceManager::RD_EASY;
            }
            else if( m_difficulty == RaceManager::RD_MEDIUM )
            {
                m_difficulty = RaceManager::RD_HARD;
            }
            else if( m_difficulty == RaceManager::RD_EASY )
            {
                m_difficulty = RaceManager::RD_MEDIUM;
            }
            widget_manager->setWgtText( WTOK_DIFFICULTY, getDifficultyString(m_difficulty) );
            break;

        case WTOK_DIFFICULTY_DOWN:
            if( m_difficulty == RaceManager::RD_HARD )
            {
                m_difficulty = RaceManager::RD_MEDIUM;
            }
            else if( m_difficulty == RaceManager::RD_MEDIUM )
            {
                m_difficulty = RaceManager::RD_EASY;
            } 
            else if( m_difficulty == RaceManager::RD_EASY )
            {
                m_difficulty = RaceManager::RD_HARD;
            } 
            widget_manager->setWgtText( WTOK_DIFFICULTY, 
                                        getDifficultyString(m_difficulty) );

            break;

        case WTOK_KARTS_UP:
            {
	        m_num_karts = m_num_karts==stk_config->m_max_karts 
                            ? m_min_karts : m_num_karts + 1;

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_karts );

                widget_manager->setWgtText( WTOK_KARTS, label );
            }
            break;

        case WTOK_KARTS_DOWN:
            {
                m_num_karts = m_num_karts==m_min_karts 
                            ? stk_config->m_max_karts : m_num_karts-1;
                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_karts );

                widget_manager->setWgtText( WTOK_KARTS, label );
            }
            break;

        case WTOK_LAPS_UP:
            {
                m_num_laps++;
                if(m_num_laps>10) m_num_laps=1;

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_laps );

                widget_manager->setWgtText( WTOK_LAPS, label);
            }
            break;

        case WTOK_LAPS_DOWN:
            {
                m_num_laps--;
                if(m_num_laps<1) m_num_laps=10;

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_laps );

                widget_manager->setWgtText( WTOK_LAPS, label);
            }
            break;

    case WTOK_START:
            setAllValues();
            menu_manager->pushMenu(MENUID_START_RACE_FEEDBACK);
            break;
    case WTOK_QUIT:
            // Save the current values as a default for later.
            setAllValues();
            menu_manager->popMenu();
        break;
    default: break;
    }   // switch
}   // select

// ----------------------------------------------------------------------------
/** If the user presses a leave key, still save the values as default. */
void RaceOptions::handle(GameAction ga, int value)
{
    // Attempts to close the menu are silently discarded
    // since they do not make sense at this point.
    if(ga == GA_LEAVE)
        setAllValues();  
    BaseGUI::handle(ga, value);
}   // handle

// ----------------------------------------------------------------------------
/** Stores the selected parameters (#laps etc) in the race manager and in
 *  user_config, so that they will be available as default next time.
 */
void RaceOptions::setAllValues()
{
    if( m_difficulty >= RaceManager::RD_EASY &&
        m_difficulty <= RaceManager::RD_HARD)
    {
        race_manager->setDifficulty((RaceManager::Difficulty)m_difficulty);
        user_config->setDefaultNumDifficulty(m_difficulty);
    }
    else // invalid difficulty
    {
        race_manager->setDifficulty( RaceManager::RD_EASY );
        user_config->setDefaultNumDifficulty(RaceManager::RD_EASY);
    }

    // if there is no AI, there's no point asking the player for the amount of karts.
    // It will always be the same as the number of human players
    if(RaceManager::isBattleMode( race_manager->getMinorMode() ))
        race_manager->setNumKarts(race_manager->getNumLocalPlayers());
    else
        race_manager->setNumKarts(m_num_karts);
    user_config->setDefaultNumKarts(race_manager->getNumKarts());

    if( race_manager->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX    &&
        RaceManager::modeHasLaps( race_manager->getMinorMode() )    )
    {
        race_manager->setNumLaps( m_num_laps );
        user_config->setDefaultNumLaps(m_num_laps);
    }
}   // setAllValues

// ----------------------------------------------------------------------------
const char *RaceOptions::getDifficultyString(int difficulty) const
{
    switch(difficulty)
    {
    case RaceManager::RD_EASY:     return _("Novice");
    case RaceManager::RD_MEDIUM:   return _("Driver");
    case RaceManager::RD_HARD:     return _("Racer" );
    default:                       return _("Novice");

    }   // switch
}   // getDifficultyString
