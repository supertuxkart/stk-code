//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015  Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2015  Joerg Henrichs
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
#include "config/user_config.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <stdexcept>


// ----------------------------------------------------------------------------
/** Loads a grand prix definition from a file.
 *  \param filename Name of the file to load.
 */
GrandPrixData::GrandPrixData(const std::string& filename, enum GPGroupType group)
{
    setFilename(filename);
    m_id       = StringUtils::getBasename(StringUtils::removeExtension(filename));
    m_editable = (filename.find(file_manager->getGPDir(), 0) == 0);
    m_group    = group;

    reload();
}   // GrandPrixData

// ----------------------------------------------------------------------------
/** Creates a random grand prix from the specified parameters.
 *  \param number_of_tracks How many tracks to select.
 *  \param track_group From which track group to select the tracks.
 *  \param use_reverse How the reverse setting is to be determined.
 *  \param new_tracks If true, new tracks are selected, otherwise existing
 *         tracks will not be changed (used to e.g. increase the number of
 *         tracks in an already existing random grand prix).
 *
 */
void GrandPrixData::createRandomGP(const unsigned int number_of_tracks,
                                   const std::string &track_group,
                                   const GPReverseType use_reverse,
                                   bool new_tracks)
{
    m_filename = "Random GP - Not loaded from a file!";
    m_id       = getRandomGPID();
    m_name     = getRandomGPName();
    m_editable = false;
    m_group    = GP_NONE;
    m_reverse_type = use_reverse;

    if(new_tracks)
    {
        m_tracks.clear();
        m_laps.clear();
        m_reversed.clear();
    }
    m_tracks.reserve(number_of_tracks);
    m_laps.reserve(number_of_tracks);
    m_reversed.reserve(number_of_tracks);

    changeTrackNumber(number_of_tracks, track_group);
    changeReverse(use_reverse);
}   // createRandomGP

// ----------------------------------------------------------------------------
/** Either adds or removes tracks to get the requested numder of tracks in
 *  a random GP.
 *  \param number_of_tracks How many tracks should be in the random list.
 *  \param track_group From which group to select the tracks.
 */
void GrandPrixData::changeTrackNumber(const unsigned int number_of_tracks,
                                      const std::string& track_group)
{
    // The problem with the track groups is that "all" isn't a track group
    // TODO: Add "all" to the track groups and rewrite this more elegant
    std::vector<int> track_indices;
    if (track_group == "all")
    {
        for(unsigned int i=0; i<track_manager->getNumberOfTracks(); i++)
        {
            const Track *track = track_manager->getTrack(i);
            // Ignore no-racing tracks:
            if(!track->isRaceTrack())
                continue;

            if (PlayerManager::getCurrentPlayer()->isLocked(track->getIdent()))
                continue;

            // Only add tracks that are not already picked.
            if(std::find(m_tracks.begin(), m_tracks.end(), track->getIdent())==
                m_tracks.end())
                track_indices.push_back(i);
        }
    }
    else
    {
        track_indices = track_manager->getTracksInGroup(track_group);
    }
    assert(number_of_tracks <= track_indices.size() + m_tracks.size());

    // add or remove the right number of tracks
    if (m_tracks.size() < number_of_tracks)
    {
        while (m_tracks.size() < number_of_tracks)
        {
            int index       = rand() % track_indices.size();
            int track_index = track_indices[index];

            const Track *track = track_manager->getTrack(track_index);
            std::string id = track->getIdent();

            if (PlayerManager::getCurrentPlayer()->isLocked(track->getIdent()))
                continue;

            bool is_already_added = false;
            for (unsigned int i = 0; i < m_tracks.size(); i++)
            {
                if (m_tracks[i] == id)
                {
                    is_already_added = true;
                    break;
                }
            }

            if (!is_already_added)
            {
                m_tracks.push_back(id);
                m_laps.push_back(track->getDefaultNumberOfLaps());
                m_reversed.push_back(false); // This will be changed later in the code
            }

            track_indices.erase(track_indices.begin()+index);
        }
    }
    else if (m_tracks.size() > number_of_tracks)
    {
        while (m_tracks.size() > number_of_tracks)
        {
            m_tracks.pop_back();
            m_laps.pop_back();
            m_reversed.pop_back();
        }
    }

    assert(m_tracks.size() == m_laps.size()    );
    assert(m_laps.size()   == m_reversed.size());
}   // changeTrackNumber

// ----------------------------------------------------------------------------
/** Updates the GP data with newly decided reverse requirements.
 *  \param use_reverse How reverse setting for each track is to be determined.
 */
void GrandPrixData::changeReverse(const GrandPrixData::GPReverseType use_reverse)
{
    m_reverse_type = use_reverse;
    for (unsigned int i = 0; i < m_tracks.size(); i++)
    {
        if (use_reverse == GP_NO_REVERSE)
        {
            m_reversed[i] = false;
        }
        else if (use_reverse == GP_ALL_REVERSE) // all reversed
        {
            m_reversed[i] = track_manager->getTrack(m_tracks[i])->reverseAvailable();
        }
        else if (use_reverse == GP_RANDOM_REVERSE)
        {
            if (track_manager->getTrack(m_tracks[i])->reverseAvailable())
                m_reversed[i] = (rand() % 2 != 0);
            else
                m_reversed[i] = false;
        }
    }   // for i < m_tracks.size()
}   // changeReverse

// ----------------------------------------------------------------------------
/** Sets the id of this grand prix.
 *  \param id The new id.
 */
void GrandPrixData::setId(const std::string& id)
{
    m_id = id;
}   // setId

// ----------------------------------------------------------------------------
/** Sets the name of the grand prix.
 *  \param name New name.
 */
void GrandPrixData::setName(const irr::core::stringw& name)
{
    m_name = name;
}   // setName

// ----------------------------------------------------------------------------
/** Sets the filename of this grand prix.
 *  \param filename New filename.
 */
void GrandPrixData::setFilename(const std::string& filename)
{
    m_filename = filename;
}   // setFilename

// ----------------------------------------------------------------------------
/** Sets if this grand prix can be edited.
 *  \param editable New value.
 */
void GrandPrixData::setEditable(const bool editable)
{
    m_editable = editable;
}   // setEditable

// ----------------------------------------------------------------------------
/** Sets the group of this grand prix.
 *  \param editable New value.
 */
void GrandPrixData::setGroup(const enum GPGroupType group)
{
    m_group = group;
}   // setGroup

// ----------------------------------------------------------------------------
/** Reloads grand prix from file.
 */
void GrandPrixData::reload()
{
    m_tracks.clear();
    m_laps.clear();
    m_reversed.clear();

    std::unique_ptr<XMLNode> root(file_manager->createXMLTree(m_filename));
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
                    "Missing 'name' attribute", m_filename.c_str());
        throw std::runtime_error("Missing name attribute");
    }

    const int amount = root->getNumNodes();
    if (amount == 0)
    {
         Log::warn("GrandPrixData",
                   "Grandprix file '%s': There is no track defined",
                   m_filename.c_str());
    }

    // Every iteration means parsing one track entry
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

        if (number_of_laps < 1 && !UserConfigParams::m_artist_debug_mode)
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
/** Saves the grand prix data to a file.
 */
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
}   // writeToFile

// ----------------------------------------------------------------------------
/** Checks if the grand prix data are consistent.
 *  \param log_error: If errors should be sent to the logger.
 */
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
}   // checkConsistency

// ----------------------------------------------------------------------------
/** Returns true if the track is available. This is used to test if Fort Magma
 *  is available (this way FortMagma is not used in the last Grand Prix in
 *  story mode, but will be available once all challenges are done and nolok
 *  is unlocked). It also prevents people from using the grand prix editor as
 *  a way to play tracks that still haven't been unlocked
 *  \param id Name of the track to test.
 *  \param include_locked If set to true, all tracks (including locked tracks)
 *         are considered to be available.
 */
bool GrandPrixData::isTrackAvailable(const std::string &id,
                                     bool include_locked     ) const
{
    if (include_locked)
        return true;
    else if (id == "fortmagma")
        return !PlayerManager::getCurrentPlayer()->isLocked("fortmagma");
    else
        return (!m_editable ||
                !PlayerManager::get()->getCurrentPlayer()->isLocked(id));
}   // isTrackAvailable

// ----------------------------------------------------------------------------
/** Returns the list of tracks that is available (i.e. unlocked) of this
 *   grand prix.
 *  \param include_locked If data for locked tracks should be included or not.
 *  \return A copy of the list of available tracks in this grand prix.
 */
std::vector<std::string> GrandPrixData::getTrackNames(bool include_locked) const
{
    std::vector<std::string> names;
    for (unsigned int i = 0; i < m_tracks.size(); i++)
    {
        if(isTrackAvailable(m_tracks[i], include_locked))
            names.push_back(m_tracks[i]);
    }
    return names;
}   // getTrackNames

// ----------------------------------------------------------------------------
/** Returns the laps for each available track of the grand prix.
 *  \param include_locked If data for locked tracks should be included or not.
 *  \return a std::vector containing the laps for each grand prix.
 */
std::vector<int> GrandPrixData::getLaps(bool include_locked) const
{
    std::vector<int> laps;
    for (unsigned int i = 0; i< m_tracks.size(); i++)
        if(isTrackAvailable(m_tracks[i], include_locked))
            laps.push_back(m_laps[i]);

    return laps;
}   // getLaps

// ----------------------------------------------------------------------------
/** Returns the reverse setting for each available grand prix.
 *  \param include_locked If data for locked tracks should be included or not.
 *  \return A copy of alist with the reverse status for each track.
 */
std::vector<bool> GrandPrixData::getReverse(bool include_locked) const
{
    std::vector<bool> reverse;
    for (unsigned int i = 0; i< m_tracks.size(); i++)
        if(isTrackAvailable(m_tracks[i], include_locked))
            reverse.push_back(m_reversed[i]);

    return reverse;
}   // getReverse

// ----------------------------------------------------------------------------
/** Returns true if this grand prix can be edited.
 */
bool GrandPrixData::isEditable() const
{
    return m_editable;
}   // isEditable

// ----------------------------------------------------------------------------
/** Returns the number of tracks in this grand prix.
 *  \param include_locked If data for locked tracks should be included or not.
 */
unsigned int GrandPrixData::getNumberOfTracks(bool includeLocked) const
{
    if (includeLocked)
        return (unsigned int)m_tracks.size();
    else
        return (unsigned int)getTrackNames(false).size();
}   // getNumberOfTracks

// ----------------------------------------------------------------------------
/** Returns the (translated) name of the track with the specified index.
 */
irr::core::stringw GrandPrixData::getTrackName(const unsigned int track) const
{
    assert(track < getNumberOfTracks(true));
    Track* t = track_manager->getTrack(m_tracks[track]);
    assert(t != NULL);
    return t->getName();
}   // getTrackName

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
    assert (-1 <= position && position < n);

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
    assert (0 <= track && track < getNumberOfTracks(true));

    m_tracks.erase(m_tracks.begin() + track);
    m_laps.erase(m_laps.begin() + track);
    m_reversed.erase(m_reversed.begin() + track);
}

/* EOF */
