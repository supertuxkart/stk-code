//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "race/highscores.hpp"

#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "race/race_manager.hpp"

#include <stdexcept>
#include <fstream>

// -----------------------------------------------------------------------------
Highscores::Highscores(const HighscoreType &highscore_type,
                       int num_karts,
                       const RaceManager::Difficulty &difficulty,
                       const std::string &track_name,
                       const int number_of_laps,
                       const bool reverse)
{
    m_track           = track_name;
    m_highscore_type  = highscore_type;
    m_number_of_karts = num_karts;
    m_difficulty      = difficulty;
    m_number_of_laps  = number_of_laps;
    m_reverse         = reverse;

    for(int i=0; i<HIGHSCORE_LEN; i++)
    {
        m_name[i]      = "";
        m_kart_name[i] = "";
        m_time[i]      = -9.9f;
    }
}
// -----------------------------------------------------------------------------
Highscores::Highscores(const XMLNode &node)
{
    m_track           = "";
    m_highscore_type  = "HST_UNDEFINED";
    m_number_of_karts = -1;
    m_difficulty      = -1;
    m_number_of_laps  = -1;
    m_reverse         = false;

    for(int i=0; i<HIGHSCORE_LEN; i++)
    {
        m_name[i]      = "";
        m_kart_name[i] = "";
        m_time[i]      = -9.9f;
    }

    readEntry(node);
}   // Highscores

// -----------------------------------------------------------------------------
void Highscores::readEntry(const XMLNode &node)
{
    node.get("track-name",     &m_track               );
    node.get("number-karts",   &m_number_of_karts     );
    std::string hst="HST_UNDEFINED";
    node.get("hscore-type",    &hst                   );
    m_highscore_type = (HighscoreType)hst;
    node.get("difficulty",     &m_difficulty          );
    node.get("number-of-laps", &m_number_of_laps      );
    node.get("reverse",        &m_reverse             );

    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        const XMLNode *entry = node.getNode(i);
        entry->get("time",     &m_time[i]            );
        entry->get("name",     &m_name[i]            );
        entry->get("kartname", &m_kart_name[i]       );

        // a non-empty entry needs a non-empty kart name.
        if (!(m_time[i] <= 0.0f || m_kart_name[i].size() > 0))
        {
            throw std::logic_error("Invalid highscore entry : empty kart name");
        }
        if (!(m_time[i] <= 0.0f || m_name[i].size() > 0))
        {
            throw std::logic_error("Invalid highscore entry : empty kart name");
        }
    }
}   // readEntry

// -----------------------------------------------------------------------------
/** Writes the highscores in this entry to the writer. It will only write
 *  anything if there is actually a highscore recored (i.e. time >=0). Empty
 *  entries are created e.g. when changing the number of laps in the GUI,
 *  resulting in empty entries here.
 *  \param writer The file stream to write the data to.
 */
void Highscores::writeEntry(UTFWriter &writer)
{
    // Only
    bool one_is_set = false;
    for(unsigned int i=0; i<HIGHSCORE_LEN; i++)
        one_is_set |= m_time[i]>=0;
    if(!one_is_set) return;

    writer << L"  <highscore track-name    =\"" << m_track.c_str()           << "\"\n";
    writer << L"             number-karts  =\"" << m_number_of_karts         << "\"\n";
    writer << L"             difficulty    =\"" << m_difficulty              << "\"\n";
    writer << L"             hscore-type   =\"" << m_highscore_type.c_str()  << "\"\n";
    writer << L"             number-of-laps=\"" << m_number_of_laps          << "\"\n";
    writer << L"             reverse       =\"" << m_reverse                 << "\">\n";

    for(int i=0; i<HIGHSCORE_LEN; i++)
    {
        if (m_time[i] > 0.0f)
        {
            assert(m_kart_name[i].size() > 0);
            writer << L"             <entry time    =\"" << m_time[i] << L"\"\n";
            writer << L"                    name    =\"" << m_name[i] << L"\"\n";
            writer << L"                    kartname=\"" << m_kart_name[i].c_str()
                   << L"\"/>\n";
        }
    }   // for i
    writer << L"  </highscore>\n";
}   // writeEntry

// -----------------------------------------------------------------------------
int Highscores::matches(const HighscoreType &highscore_type,
                        int num_karts, const RaceManager::Difficulty &difficulty,
                        const std::string &track, const int number_of_laps,
                        const bool reverse)
{
    return (m_highscore_type  == highscore_type   &&
            m_track           == track            &&
            m_difficulty      == difficulty       &&
            m_number_of_laps  == number_of_laps   &&
            m_number_of_karts == num_karts        &&
            m_reverse         == reverse            );
}   // matches

// -----------------------------------------------------------------------------
/** Inserts the data into the highscore list.
 *  If the new entry is fast enough to
 *  be in the highscore list, the new position (1-HIGHSCORE_LEN) is returned,
 *  otherwise a 0.
 */
int Highscores::addData(const std::string& kart_name,
                        const core::stringw& name, const float time)
{
    int position=-1;
    for(int i=0; i<HIGHSCORE_LEN; i++)
    {
        // Check for unused entry. If so, just insert the new record
        if(m_time[i]<0.0f)
        {
            position=i;
            break;
        }
        // Check if new entry is faster than than in slot 'i', if so
        // move times etc and insert new entry
        if(time < m_time[i])
        {
            for(int j=HIGHSCORE_LEN-2;j>=i;j--)
            {
                m_name[j+1]      = m_name[j];
                m_kart_name[j+1] = m_kart_name[j];
                m_time[j+1]      = m_time[j];
            }
            position = i;
            break;
        }
    }//next score slot

    if(position>=0)
    {
        m_track               = race_manager->getTrackName();
        m_number_of_karts     = race_manager->getNumberOfKarts();
        m_difficulty          = race_manager->getDifficulty();
        m_number_of_laps      = race_manager->getNumLaps();
        m_reverse             = race_manager->getReverseTrack();
        m_name[position]      = name;
        m_time[position]      = time;
        m_kart_name[position] = kart_name;
    }

    return position+1;
}   // addData

// -----------------------------------------------------------------------------
int Highscores::getNumberEntries() const
{
    for(int i=HIGHSCORE_LEN-1; i>=0; i--)
    {
        if(m_time[i]>0) return i+1;
    }
    return 0;
}   // getNumberEntries

// -----------------------------------------------------------------------------
void Highscores::getEntry(int number, std::string &kart_name,
                          core::stringw &name, float *const time) const
{
    if(number<0 || number>getNumberEntries())
    {
        Log::warn("Highscores", "Accessing undefined highscore entry:");
        Log::warn("Highscores", "Number %d, but %d entries are defined.", number,
                  getNumberEntries());
        Log::warn("Highscores", "This error can be ignored, but no highscores are available.");
        return;
    }
    kart_name = m_kart_name[number];
    name      = m_name[number];
    *time     = m_time[number];

}   // getEntry

// -----------------------------------------------------------------------------
