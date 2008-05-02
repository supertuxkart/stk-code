//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

RaceOptions::RaceOptions() :
    m_difficulty(RaceManager::RD_HARD), m_num_karts(4), m_num_laps(3)
{
    widget_manager->addTextWgt( WTOK_TITLE, 60, 7, _("Choose the race options") );
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 1);
    widget_manager->breakLine();

    if( race_manager->getRaceMode() != RaceManager::RM_TIME_TRIAL )
    {
        widget_manager->insertColumn();
        widget_manager->addTextWgt( WTOK_DIFFICULTY_TITLE, 30, 7, _("Difficulty") );
        widget_manager->addTextButtonWgt( WTOK_DIFFICULTY_UP, 12, 12, _("More") );

        widget_manager->addTextWgt( WTOK_DIFFICULTY, 12, 7, _("Racer") );
        widget_manager->setWgtBorderPercentage( WTOK_DIFFICULTY, 10 );
        widget_manager->showWgtBorder( WTOK_DIFFICULTY );
        widget_manager->hideWgtRect( WTOK_DIFFICULTY );

        widget_manager->addTextButtonWgt( WTOK_DIFFICULTY_DOWN, 12, 12, _("Less") );

        widget_manager->breakLine();
        widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 1);
    }

    if( race_manager->getRaceMode() != RaceManager::RM_TIME_TRIAL )
    {
        widget_manager->insertColumn();
        widget_manager->addTextWgt( WTOK_KARTS_TITLE, 30, 7, _("Number of karts") );
        widget_manager->addTextButtonWgt( WTOK_KARTS_UP, 12, 12, _("More") );

        widget_manager->addTextWgt( WTOK_KARTS, 12, 7, _("4") );
        widget_manager->setWgtBorderPercentage( WTOK_KARTS, 10 );
        widget_manager->showWgtBorder( WTOK_KARTS );
        widget_manager->hideWgtRect( WTOK_KARTS );

        widget_manager->addTextButtonWgt( WTOK_KARTS_DOWN, 12, 12, _("Less") );

        widget_manager->breakLine();
        widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 1);
    }

    if( race_manager->getRaceMode() != RaceManager::RM_GRAND_PRIX   &&
        race_manager->getRaceMode() != RaceManager::RM_FOLLOW_LEADER   )
    {
        widget_manager->insertColumn();
        widget_manager->addTextWgt( WTOK_LAPS_TITLE, 30, 7, _("Number of laps") );
        widget_manager->addTextButtonWgt( WTOK_LAPS_UP, 12, 12, _("More") );

        widget_manager->addTextWgt( WTOK_LAPS, 12, 7, _("3") );
        widget_manager->setWgtBorderPercentage( WTOK_LAPS, 10 );
        widget_manager->showWgtBorder( WTOK_LAPS );
        widget_manager->hideWgtRect( WTOK_LAPS );

        widget_manager->addTextButtonWgt( WTOK_LAPS_DOWN, 12, 12, _("Less") );
        widget_manager->breakLine();
    }

    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 1);
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt( WTOK_START, 60, 7, _("Start race") );
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt( WTOK_QUIT, 60, 7, _("Press <ESC> to go back") );

    widget_manager->layout(WGT_AREA_ALL);
}   // RaceOptions

//-----------------------------------------------------------------------------
RaceOptions::~RaceOptions()
{
    widget_manager->reset();
}   // ~RaceOptions

//-----------------------------------------------------------------------------
void RaceOptions::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
        case WTOK_DIFFICULTY_UP:
            if( m_difficulty == RaceManager::RD_HARD && !unlock_manager->isLocked("skidding"))
            {
                m_difficulty = RaceManager::RD_SKIDDING;
                widget_manager->setWgtText( WTOK_DIFFICULTY, _("Skidding Preview") );
            }
            else if( m_difficulty == RaceManager::RD_MEDIUM )
            {
                m_difficulty = RaceManager::RD_HARD;
                widget_manager->setWgtText( WTOK_DIFFICULTY, _("Racer") );
            }
            else if( m_difficulty == RaceManager::RD_EASY )
            {
                m_difficulty = RaceManager::RD_MEDIUM;
                widget_manager->setWgtText( WTOK_DIFFICULTY, _("Driver") );
            }
            break;

        case WTOK_DIFFICULTY_DOWN:
            if( m_difficulty == RaceManager::RD_SKIDDING )
            {
                m_difficulty = RaceManager::RD_HARD;
                widget_manager->setWgtText( WTOK_DIFFICULTY, _("Racer") );
            }

            else if( m_difficulty == RaceManager::RD_HARD )
            {
                m_difficulty = RaceManager::RD_MEDIUM;
                widget_manager->setWgtText( WTOK_DIFFICULTY, _("Driver") );
            }
            else if( m_difficulty == RaceManager::RD_MEDIUM )
            {
                m_difficulty = RaceManager::RD_EASY;
                widget_manager->setWgtText( WTOK_DIFFICULTY, _("Novice") );
            }
            break;

        case WTOK_KARTS_UP:
            {
                m_num_karts = std::min( stk_config->m_max_karts, m_num_karts + 1 );

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_karts );

                widget_manager->setWgtText( WTOK_KARTS, label );
            }
            break;

        case WTOK_KARTS_DOWN:
            {
                // Follow the leader needs at least three karts
                if(race_manager->getRaceMode() == RaceManager::RM_FOLLOW_LEADER)
                {
                    m_num_karts = std::max( 3, m_num_karts - 1 );
                }
                else
                {
                    m_num_karts = std::max( (int)race_manager->getNumPlayers(),
                        m_num_karts - 1 );
                }

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_karts );

                widget_manager->setWgtText( WTOK_KARTS, label );
            }
            break;

        case WTOK_LAPS_UP:
            {
                m_num_laps = std::min( 10, m_num_laps + 1 );

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_laps );

                widget_manager->setWgtText( WTOK_LAPS, label);
            }
            break;

        case WTOK_LAPS_DOWN:
            {
                m_num_laps = std::max( 1, m_num_laps - 1 );

                char label[ MAX_MESSAGE_LENGTH ];
                snprintf( label, MAX_MESSAGE_LENGTH, "%d", m_num_laps );

                widget_manager->setWgtText( WTOK_LAPS, label);
            }
            break;

    case WTOK_START:
        if( race_manager->getRaceMode() != RaceManager::RM_TIME_TRIAL )
        {
            if( m_difficulty == RaceManager::RD_SKIDDING )
            {
                race_manager->setDifficulty( RaceManager::RD_SKIDDING );
            }
            else if( m_difficulty == RaceManager::RD_HARD )
            {
                race_manager->setDifficulty( RaceManager::RD_HARD );
            }
            else if( m_difficulty == RaceManager::RD_MEDIUM )
            {
                race_manager->setDifficulty( RaceManager::RD_MEDIUM );
            }
            else //if( m_difficulty == RaceManager::RD_EASY )
            {
                race_manager->setDifficulty( RaceManager::RD_EASY );
            }
        }

        if( race_manager->getRaceMode() != RaceManager::RM_TIME_TRIAL )
        {
            race_manager->setNumKarts(m_num_karts);
        }
        else race_manager->setNumKarts( 1 );

        if( race_manager->getRaceMode() != RaceManager::RM_GRAND_PRIX    &&
            race_manager->getRaceMode() != RaceManager::RM_FOLLOW_LEADER    )
        {
            race_manager->setNumLaps( m_num_laps );
        }

        menu_manager->pushMenu(MENUID_START_RACE_FEEDBACK);
        break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }   // switch
}   // select


