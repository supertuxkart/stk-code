 //  $Id: highscores.hpp 921 2007-02-28 05:43:34Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "highscores.hpp"

#include <stdexcept>
#include <sstream>

#include "race_manager.hpp"

// -----------------------------------------------------------------------------
HighscoreEntry::HighscoreEntry(const HighscoreEntry::HighscoreType highscore_type,
                               int num_karts, const RaceManager::Difficulty difficulty, 
                               const std::string trackName, const int number_of_laps)
{
    m_track           = trackName; 
    m_highscore_type  = highscore_type;
    m_number_of_karts = num_karts;
    m_difficulty      = difficulty;
    m_number_of_laps  = number_of_laps;
    
    for(int i=0; i<HIGHSCORE_LEN; i++) 
    {
        m_name[i]      = "";
        m_kart_name[i] = "";
        m_time[i]      = -9.9f;
    }
}
// -----------------------------------------------------------------------------
HighscoreEntry::HighscoreEntry(const lisp::Lisp* const node)
{
    m_track           = ""; 
    m_highscore_type  = "HST_UNDEFINED";
    m_number_of_karts = -1;
    m_difficulty      = -1;
    m_number_of_laps  = -1;
    
    for(int i=0; i<HIGHSCORE_LEN; i++) 
    {
        m_name[i]      = "";
        m_kart_name[i] = "";
        m_time[i]      = -9.9f;
    }
    
    Read(node);
}
// -----------------------------------------------------------------------------
void HighscoreEntry::Read(const lisp::Lisp* const node)
{
    node->get("track-name",     m_track               );
    node->get("number-karts",   m_number_of_karts     );
    std::string hst="HST_UNDEFINED";
    node->get("hscore-type",    hst                   );
    m_highscore_type = (HighscoreType)hst;
    node->get("difficulty",     m_difficulty          );
    node->get("number-of-laps", m_number_of_laps      );

    for(int i=0; i<HIGHSCORE_LEN; i++) 
    {
        std::ostringstream s;
        s << "time-" << i;
        node->get(s.str(),m_time[i]                  );
        s.str(""); s << "name-" << i;
        node->get(s.str(),m_name[i]                  );
        s.str(""); s << "kartname-" << i;
        node->get(s.str(), m_kart_name[i]            );
    }
}   // Read

// -----------------------------------------------------------------------------
void HighscoreEntry::Write(lisp::Writer *writer)
{
    writer->write("track-name\t",     m_track            );
    writer->write("number-karts\t",   m_number_of_karts  );
    writer->write("difficulty\t\t",   m_difficulty       );
    writer->write("hscore-type\t\t",  m_highscore_type   );
    writer->write("number-of-laps\t", m_number_of_laps   );
    for(int j=0; j<HIGHSCORE_LEN; j++) 
    {
        std::ostringstream s;
        s << "time-" << j << "\t\t";
        writer->write(s.str(), m_time[j]                 );
        s.str(""); s << "name-" << j << "\t\t";
        writer->write(s.str(), m_name[j]                 );
        s.str(""); s << "kartname-" << j << "\t\t";
        writer->write(s.str(), m_kart_name[j]            );
    }   // for j
    
}   // Write

// -----------------------------------------------------------------------------
int HighscoreEntry::matches(HighscoreType highscore_type,
                            int num_karts, RaceManager::Difficulty difficulty,
                            const std::string track, const int number_of_laps)
{
    return (m_highscore_type  == highscore_type   &&
            m_track           == track            &&
            m_difficulty      == difficulty       &&
            m_number_of_laps  == number_of_laps   &&
            m_number_of_karts == num_karts          );
}   // matches
// -----------------------------------------------------------------------------
/** Inserts the data into the highscore list. 
 *  If the new entry is fast enough to
 *  be in the highscore list, the new position (1-HIGHSCORE_LEN) is returned,
 *  otherwise a 0.
 */
int HighscoreEntry::addData(const std::string& kart_name,
                            const std::string& name, const float time)
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
        m_number_of_karts     = race_manager->getNumKarts();
        m_difficulty          = race_manager->getDifficulty();
        m_number_of_laps      = race_manager->getNumLaps();
        m_name[position]      = name;
        m_time[position]      = time;
        m_kart_name[position] = kart_name;
    }

    return position+1;

}   // addData
// -----------------------------------------------------------------------------
int HighscoreEntry::getNumberEntries() const
{
    for(int i=HIGHSCORE_LEN-1; i>=0; i--)
    {
        if(m_time[i]>0) return i+1;
    }
    return 0;
}   // getNumberEntries

// -----------------------------------------------------------------------------
void HighscoreEntry::getEntry(int number, std::string &kart_name,
                          std::string &name, float *const time) const
{
    if(number<0 || number>getNumberEntries())
    {
        fprintf(stderr, "Error, accessing undefined highscore entry:\n");
        fprintf(stderr,"number %d, but %d entries are defined\n",number,
                getNumberEntries());
        fprintf(stderr, "This error can be ignored, but no highscores are available\n");
        return;
    }
    kart_name = m_kart_name[number];
    name      = m_name[number];
    *time     = m_time[number];
    
}   // getEntry

// -----------------------------------------------------------------------------
