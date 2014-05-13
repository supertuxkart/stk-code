//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013  Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2013  Joerg Henrichs
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

#include "race/grand_prix_data.hpp"

#include "config/player_profile.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

#include <iostream>
#include <memory>
#include <algorithm>
#include <stdexcept>


// ----------------------------------------------------------------------------
GrandPrixData::GrandPrixData(const std::string& filename)
{
    m_filename = filename;
    m_id       = StringUtils::getBasename(
                                        StringUtils::removeExtension(filename));
    m_editable = (filename.find(file_manager->getGPDir(), 0) == 0);
    reload();
}

// ----------------------------------------------------------------------------
void GrandPrixData::setId(const std::string& id)
{
    m_id = id;
}

// ----------------------------------------------------------------------------
void GrandPrixData::setName(const irr::core::stringw& name)
{
    m_name = name;
}

// ----------------------------------------------------------------------------
void GrandPrixData::setFilename(const std::string& filename)
{
    m_filename = filename;
}

// ----------------------------------------------------------------------------
void GrandPrixData::setEditable(const bool editable)
{
    m_editable = editable;
}

// ----------------------------------------------------------------------------
void GrandPrixData::reload()
{
    m_tracks.clear();
    m_laps.clear();
    m_reversed.clear();

    std::auto_ptr<XMLNode> root(file_manager->createXMLTree(m_filename));
    if (root.get() == NULL)
    {
        Log::error("GrandPrixData",
                   "Error while trying to read xml Grand Prix from file '%s'. "
                   "Is the file readable for supertuxkart?",
                   m_filename.c_str());
        throw std::runtime_error("File couldn't be read");
    }

    if (root->getName() != "supertuxkart_grand_prix")
    {
        Log::error("GrandPrixData",
                   "Error while trying to read Grand Prix file '%s': "
                   "Root node has the wrong name %s", m_filename.c_str(),
                   root->getName().c_str());
        throw std::runtime_error("Wrong root node name");
    }

    if (!root->get("name", &m_name))
    {
         Log::error("GrandPrixData",
                    "Error while trying to read grandprix file '%s': "
                    "missing 'name' attribute", m_filename.c_str());
        throw std::runtime_error("Missing name attribute");
    }

    // Every iteration means parsing one track entry
    const int amount = root->getNumNodes();
    for (int i = 0; i < amount; i++)
    {
        const XMLNode* node = root->getNode(i);

        if (node->getName() != "track")
        {
            Log::error("GrandPrixData"
                       "Unknown node in Grand Prix XML file '%s': %s",
                       m_filename.c_str(), node->getName().c_str());
            throw std::runtime_error("Unknown node in the XML file");
        }

        // 1. Parsing the id atttribute
        std::string track_id;
        if (!node->get("id", &track_id))
        {
            Log::error("GrandPrixData",
                       "The id attribute is missing in the %d. track entry of "
                       "the Grand Prix file '%s'.", i, m_filename.c_str());
            throw std::runtime_error("Missing track id");
        }

        // 1.1 Checking if the track exists
        Track* t = track_manager->getTrack(track_id);
        if (t == NULL)
        {
            Log::error("GrandPrixData",
                       "The Grand Prix file '%s' contains a track '%s' that "
                       "does not exist", m_filename.c_str(), track_id.c_str());
            throw std::runtime_error("Unknown track");
        }

        // 2. Parsing the number of laps
        int number_of_laps;
        if (!node->get("laps", &number_of_laps))
        {
            Log::error("GrandPrixData",
                       "The laps attribute is missing in the %d. track entry "
                       "of the Grand Prix file '%s'.", i, m_filename.c_str());
            throw std::runtime_error("Missing track id");
        }

        if (number_of_laps < 1)
        {
            Log::error("GrandPrixData",
                       "Track '%s' in the Grand Prix file '%s' should be raced "
                       "with %d laps, which isn't possible.", track_id.c_str(),
                       m_filename.c_str());
            throw std::runtime_error("Lap count lower than 1");
        }

        // 3. Parsing the reversed attribute
        bool reversed = false; // Stays false if not found
        node->get("reverse", &reversed );
        if (!t->reverseAvailable())
            reversed = false;

        // Adding parsed data
        m_tracks.push_back(track_id);
        m_laps.push_back(number_of_laps);
        m_reversed.push_back(reversed);

        assert(m_tracks.size() == m_laps.size()    );
        assert(m_laps.size()   == m_reversed.size());
    }   // end for all root nodes
}   // reload()

// ----------------------------------------------------------------------------
bool GrandPrixData::writeToFile()
{
    try
    {
        UTFWriter file(m_filename.c_str());
        if (file.is_open())
        {
            file << L"\n<supertuxkart_grand_prix name=\"" << m_name
                 << L"\">\n\n";
            for (unsigned int i = 0; i < m_tracks.size(); i++)
            {
                file <<
                    L"\t<track id=\"" << m_tracks[i] <<
                    L"\" laps=\""     << m_laps[i] <<
                    L"\" reverse=\""  << (m_reversed[i] ? L"true" : L"false")
                                      <<  L"\" />\n";
            }
            file << L"\n</supertuxkart_grand_prix>\n";

            file.close();

            return true;
        }

        return false;
    }
    catch (std::runtime_error& e)
    {
        Log::error("GrandPrixData",
                   "Failed to write grand prix to '%s'; cause: %s",
                   m_filename.c_str(), e.what());
        return false;
    }
}

// ----------------------------------------------------------------------------
bool GrandPrixData::checkConsistency(bool log_error) const
{
    for (unsigned int i = 0; i < m_tracks.size(); i++)
    {
        if (track_manager->getTrack(m_tracks[i]) == NULL)
        {
            if (log_error)
            {
                Log::error("GrandPrixData",
                           "The grand prix '%ls' won't be available because "
                           "the track '%s' does not exist!", m_name.c_str(),
                           m_tracks[i].c_str());
            }
            return false;
        }
    }
    return true;
}


// ----------------------------------------------------------------------------
/** Returns true if the track is available. This is used to test if Fort Magma
 *  is available (this way FortMagma is not used in the last Grand Prix in
 *  story mode, but will be available once all challenges are done and nolok
 *  is unlocked). It also prevents people from using the grand prix editor as
 *  a way to play tracks that still haven't been unlocked
 */
bool GrandPrixData::isTrackAvailable(const std::string &id, 
                                     bool includeLocked     ) const
{
    if (includeLocked)
        return true;
    else if (id == "fortmagma")
        return !PlayerManager::getCurrentPlayer()->isLocked("fortmagma");
    else
        return (!m_editable ||
                !PlayerManager::get()->getCurrentPlayer()->isLocked(id));
}

// ----------------------------------------------------------------------------
std::vector<std::string> GrandPrixData::getTrackNames(bool includeLocked) const
{
    std::vector<std::string> names;
    for (unsigned int i = 0; i < m_tracks.size(); i++)
    {
        if(isTrackAvailable(m_tracks[i], includeLocked))
            names.push_back(m_tracks[i]);
    }
    return names;
}

// ----------------------------------------------------------------------------
std::vector<int> GrandPrixData::getLaps(bool includeLocked) const
{
    std::vector<int> laps;
    for (unsigned int i = 0; i< m_tracks.size(); i++)
        if(isTrackAvailable(m_tracks[i], includeLocked))
            laps.push_back(m_laps[i]);

    return laps;
}

// ----------------------------------------------------------------------------
std::vector<bool> GrandPrixData::getReverse(bool includeLocked) const
{
    std::vector<bool> reverse;
    for (unsigned int i = 0; i< m_tracks.size(); i++)
        if(isTrackAvailable(m_tracks[i], includeLocked))
            reverse.push_back(m_reversed[i]);

    return reverse;
}

// ----------------------------------------------------------------------------
bool GrandPrixData::isEditable() const
{
    return m_editable;
}

// ----------------------------------------------------------------------------
unsigned int GrandPrixData::getNumberOfTracks(bool includeLocked) const
{
    if (includeLocked)
        return m_tracks.size();
    else
        return getTrackNames(false).size();
}

// ----------------------------------------------------------------------------
irr::core::stringw GrandPrixData::getTrackName(const unsigned int track) const
{
    assert(track < getNumberOfTracks(true));
    Track* t = track_manager->getTrack(m_tracks[track]);
    assert(t != NULL);
    return t->getName();
}

// ----------------------------------------------------------------------------
const std::string& GrandPrixData::getTrackId(const unsigned int track) const
{
    assert(track < getNumberOfTracks(true));
    return m_tracks[track];
}

// ----------------------------------------------------------------------------
unsigned int GrandPrixData::getLaps(const unsigned int track) const
{
    assert(track < getNumberOfTracks(true));
    return m_laps[track];
}

// ----------------------------------------------------------------------------
bool GrandPrixData::getReverse(const unsigned int track) const
{
    assert(track < getNumberOfTracks(true));
    return m_reversed[track];
}

// ----------------------------------------------------------------------------
void GrandPrixData::moveUp(const unsigned int track)
{
    assert (track > 0 && track < getNumberOfTracks(true));

    std::swap(m_tracks[track], m_tracks[track - 1]);
    std::swap(m_laps[track], m_laps[track - 1]);
    bool tmp              = m_reversed[track    ];
    m_reversed[track]     = m_reversed[track - 1];
    m_reversed[track - 1] = tmp;
}

// ----------------------------------------------------------------------------
void GrandPrixData::moveDown(const unsigned int track)
{
    assert (track < (getNumberOfTracks(true) - 1));

    std::swap(m_tracks[track], m_tracks[track + 1]);
    std::swap(m_laps[track], m_laps[track + 1]);
    bool tmp              = m_reversed[track    ];
    m_reversed[track    ] = m_reversed[track + 1];
    m_reversed[track + 1] = tmp;
}

// ----------------------------------------------------------------------------
void GrandPrixData::addTrack(Track* track, unsigned int laps, bool reverse,
                             int position)
{
    int n = getNumberOfTracks(true);
    assert (track != NULL);
    assert (laps > 0);
    assert (-1 < position && position < n);

    if (position < 0 || position == (n - 1) || m_tracks.empty())
    {
        // Append new track to the end of the list
        m_tracks.push_back(track->getIdent());
        m_laps.push_back(laps);
        m_reversed.push_back(reverse);
    }
    else
    {
        // Insert new track right after the specified position. Caution:
        // std::vector inserts elements _before_ the specified position
        m_tracks.  insert(m_tracks.begin()   + position + 1, track->getIdent());
        m_laps.    insert(m_laps.begin()     + position + 1, laps             );
        m_reversed.insert(m_reversed.begin() + position + 1, reverse          );
    }
}

// ----------------------------------------------------------------------------
void GrandPrixData::editTrack(unsigned int index, Track* track,
                              unsigned int laps, bool reverse)
{
    assert (index < getNumberOfTracks(true));
    assert (track != NULL);
    assert (laps > 0);

    m_tracks[index]   = track->getIdent();
    m_laps[index]     = laps;
    m_reversed[index] = reverse;
}

// ----------------------------------------------------------------------------
void GrandPrixData::remove(const unsigned int track)
{
    assert (0 < track && track < getNumberOfTracks(true));

    m_tracks.erase(m_tracks.begin() + track);
    m_laps.erase(m_laps.begin() + track);
    m_reversed.erase(m_reversed.begin() + track);
}

/* EOF */
