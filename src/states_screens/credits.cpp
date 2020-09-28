//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/credits.hpp"

#include <fstream>
#include <algorithm>

#include "irrString.h"
using irr::core::stringw;
using irr::core::stringc;

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "online/link_helper.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/constants.hpp"
#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
const float TIME_SECTION_FADE = 0.8f;
const float ENTRIES_FADE_TIME = 0.3f;

// ----------------------------------------------------------------------------

class CreditsEntry
{
public:
    stringw              m_name;
    std::vector<stringw> m_subentries;

    CreditsEntry(stringw& name)
    {
        m_name = name;
    }
};   // CreditsEntry

// ----------------------------------------------------------------------------

class CreditsSection
{
public:
    // read-only
    std::vector<CreditsEntry> m_entries;
    stringw                   m_name;

    CreditsSection(stringw name)       { m_name = name;             }
    // ------------------------------------------------------------------------
    void addEntry(CreditsEntry& entry) {m_entries.push_back(entry); }
    // ------------------------------------------------------------------------
    void addSubEntry(stringw& subEntryString)
    {
        m_entries[m_entries.size()-1].m_subentries.push_back(subEntryString);
    }
};   // CreditsSection

// ----------------------------------------------------------------------------

CreditsSection* CreditsScreen::getCurrentSection()
{
    return m_sections.get(m_sections.size()-1);
}   // getCurrentSection

// ----------------------------------------------------------------------------

bool CreditsScreen::getLineAsWide(std::ifstream& file, core::stringw* out)
{
    if (!file.good())
    {
        Log::error("CreditsScreen", "getLineAsWide: File is not good!");
        return false;
    }

    std::string line;
    std::getline(file, line);

    // Replace "STKVERSION" with the actual version number
    line = StringUtils::findAndReplace(line, "$STKVERSION$", STK_VERSION);

    *out = StringUtils::utf8ToWide(line);
    return file.good();

}   // getLineAsWide

// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark CreditsScreen
#endif

CreditsScreen::CreditsScreen() : Screen("credits.stkgui")
{
    m_is_victory_music = false;
}   // CreditsScreen

// ----------------------------------------------------------------------------

void CreditsScreen::loadedFromFile()
{
    reset();

    std::string creditsfile = file_manager->getAsset("CREDITS");

    std::ifstream file(
        FileUtils::getPortableReadingPath(creditsfile), std::ios::binary);

    if (file.fail() || !file.is_open() || file.eof())
    {
        Log::error("CreditsScreen", "Failed to open file at '%s'.",
                   creditsfile.c_str());
        return;
    }

    stringw line;
    int lineCount = 0;
#undef DEBUG_TRANSLATIONS    // Enable to only see the translator credits
#ifdef DEBUG_TRANSLATIONS
    int my_counter = 0;
#endif
    // Read file into wide strings (converted from utf-8 on the fly)
    while (getLineAsWide( file, &line ))
    {
#ifdef DEBUG_TRANSLATIONS
        if (my_counter > 0)
            break;
        my_counter++;
#endif
        stringc cversion = line.c_str();
        //printf("CREDITS line : %s\n", cversion.c_str());

        line = line.trim();

        if (line.size() < 1) continue; // empty line

        lineCount++;

        if ((line[0] & 0xFF) == '=' && (line[line.size()-1] & 0xFF) == '=')
        {
            line = stringw( line.subString(1, line.size()-2).c_str() );
            line = line.trim();

            m_sections.push_back( new CreditsSection(line)  );
        }
        else if ((line[0] & 0xFF) == '-')
        {
            line = stringw( line.subString(1, line.size()-1).c_str() );
            getCurrentSection()->addSubEntry( line );
        }
        else
        {
            CreditsEntry entry(line);
            getCurrentSection()->addEntry( entry );
        }
    } // end while


    if (lineCount == 0)
    {
        Log::error("CreditsScreen", "Could not read anything from CREDITS file!");
        return;
    }


    irr::core::stringw translators_credits = _("translator-credits");
    const unsigned MAX_PER_SCREEN = 6;

    if (translators_credits != L"translator-credits")
    {
        std::vector<irr::core::stringw> translator =
            StringUtils::split(translators_credits, '\n');

        m_sections.push_back( new CreditsSection("Translations"));
        for (unsigned int i = 1; i < translator.size(); i = i + MAX_PER_SCREEN)
        {
#ifndef SERVER_ONLY
            line = stringw(translations->getCurrentLanguageName().c_str());
#endif
            CreditsEntry entry(line);
            getCurrentSection()->addEntry( entry );

            for (unsigned int j = 0; i + j < translator.size() && j < MAX_PER_SCREEN; j ++)
            {
                getCurrentSection()->addSubEntry(translator[i + j]);
            }
        }
        assert(m_sections.size() > 0);

        // translations should be just before the last screen
        m_sections.swap( m_sections.size() - 1, m_sections.size() - 2 );
    }
}   // loadedFromFile

// ----------------------------------------------------------------------------

void CreditsScreen::init()
{
    Screen::init();
    Widget* w = getWidget<Widget>("animated_area");
    assert(w != NULL);

    reset();
    
    setArea(w->m_x + GUIEngine::getFontHeight(),
             w->m_y + GUIEngine::getFontHeight() / 2,
             w->m_w - GUIEngine::getFontHeight() * 2,
             w->m_h - GUIEngine::getFontHeight());
}   // init

// ----------------------------------------------------------------------------

void CreditsScreen::setArea(const int x, const int y, const int w, const int h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    m_section_rect = core::rect< s32 >( x, y, x + w, y + h/6 );
}   // setArea

// ----------------------------------------------------------------------------

void CreditsScreen::reset()
{
    m_curr_section = 0;
    m_curr_element = -1;
    time_before_next_step = TIME_SECTION_FADE;
    m_time_element = 2.5f;
}   // reset

// ----------------------------------------------------------------------------

void CreditsScreen::onDraw(float elapsed_time)
{
    // multiply by 0.8 to slow it down a bit as a whole
    time_before_next_step -= elapsed_time*0.8f;

    const bool before_first_elem = (m_curr_element == -1);
    const bool after_last_elem   =
        (m_curr_element >= (int)m_sections[m_curr_section].m_entries.size());


    // ---- section name
    video::SColor color =  GUIEngine::getSkin()->getColor("credits_text::neutral");
    video::SColor white_color( 255, 255, 255, 255 );

    // manage fade-in
    if (before_first_elem)
    {
        // I use 425 instead of 255 so that there is a little pause after
        int alpha = 425 - (int)(time_before_next_step/TIME_SECTION_FADE * 425);
        if      (alpha < 0) alpha = 0;
        else if (alpha > 255) alpha = 255;
        white_color.setAlpha( alpha );
    }
    // manage fade-out
    else if (after_last_elem)
    {
        // I use 425 instead of 255 so that there is a little pause after
        int alpha =
            (int)(time_before_next_step/TIME_SECTION_FADE * 425) - (425-255);
        if      (alpha < 0)   alpha = 0;
        else if (alpha > 255) alpha = 255;
        white_color.setAlpha( alpha );
    }

    GUIEngine::getTitleFont()->draw(m_sections[m_curr_section].m_name.c_str(),
                                    m_section_rect, white_color,
                                    true /* center h */, true /* center v */ );

    // draw entries
    if (!before_first_elem && !after_last_elem)
    {
        int text_offset  = 0;

        // fade in
        if (time_before_next_step < ENTRIES_FADE_TIME)
        {
            const float fade_in = time_before_next_step / ENTRIES_FADE_TIME;

            int alpha =  (int)(fade_in * 255);

            if      (alpha < 0) alpha = 0;
            else if (alpha > 255) alpha = 255;

            color.setAlpha( alpha );

            text_offset = (int)((1.0f - fade_in) * GUIEngine::getFontHeight());
        }
        // fade out
        else if (time_before_next_step >= m_time_element - ENTRIES_FADE_TIME)
        {
            const float fade_out =
                 (time_before_next_step - (m_time_element - ENTRIES_FADE_TIME))
                / ENTRIES_FADE_TIME;

            int alpha = 255 - (int)(fade_out * 255);
            if(alpha < 0) alpha = 0;
            else if(alpha > 255) alpha = 255;
            color.setAlpha( alpha );

            text_offset = -(int)(fade_out * GUIEngine::getFontHeight());
        }


        GUIEngine::getFont()->draw(
            m_sections[m_curr_section].m_entries[m_curr_element]
                                      .m_name.c_str(),
            core::recti( m_x + text_offset, m_y + m_h/6,
                         m_x + m_w + text_offset, m_y + m_h/3 ),
            color, false /* center h */, true /* center v */, NULL,
            true /* ignore RTL */                                   );

        const int subamount = (int)m_sections[m_curr_section]
                             .m_entries[m_curr_element].m_subentries.size();
        int suby = m_y + m_h/3;
        const int inc = subamount == 0 ? m_h/8
                                       : std::min(m_h/8,
                                                  (m_h - m_h/3)/(subamount+1));
        for(int i=0; i<subamount; i++)
        {
            GUIEngine::getFont()->draw(m_sections[m_curr_section]
                                       .m_entries[m_curr_element]
                                       .m_subentries[i].c_str(),
                                       core::recti( m_x + GUIEngine::getFontHeight()/2,
                                                    suby + text_offset/(1+1),
                                                    m_x + m_w + GUIEngine::getFontHeight()/2,
                                                    suby + m_h/8
                                                         + text_offset/(1+1) ),
                                       color, false/* center h */,
                                       true /* center v */, NULL,
                                       true /* ignore RTL */ );
            suby += inc;
        }

    }

    // is it time to move on?
    if (time_before_next_step < 0)
    {
        if (after_last_elem)
        {
            // switch to next element
            m_curr_section++;
            m_curr_element = -1;
            time_before_next_step = TIME_SECTION_FADE;

            if (m_curr_section >= (int)m_sections.size()) reset();
        }
        else
        {
            // move on
            m_curr_element++;

            if (m_curr_element >=
                (int)m_sections[m_curr_section].m_entries.size())
            {
                time_before_next_step = TIME_SECTION_FADE;
            }
            else
            {
                const int count =
                    (int)m_sections[m_curr_section].m_entries[m_curr_element]
                                                   .m_subentries.size();
                m_time_element = 2.0f + count*0.6f;
                time_before_next_step = m_time_element;
            }
        }
    }

}   // onUpdate

// ----------------------------------------------------------------------------

void CreditsScreen::eventCallback(GUIEngine::Widget* widget,
                                  const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    if (name == "donate")
    {
        // Open donation page
        Online::LinkHelper::openURL(stk_config->m_donate_url);
    }
}

// ----------------------------------------------------------------------------

