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
GrandPrixData::GrandPrixData(const std::string& filename) throw(std::logic_error)
{
    m_filename = filename;
    m_id       = StringUtils::getBasename(StringUtils::removeExtension(filename));
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
        Log::error("GrandPrixData","Error while trying to read grandprix file '%s'",
                    m_filename.c_str());
        throw std::logic_error("File not found");
    }

    bool foundName = false;

    if (root->getName() == "supertuxkart_grand_prix")
    {
        if (root->get("name", &m_name) == 0)
        {
             Log::error("GrandPrixData", "Error while trying to read grandprix file '%s' : "
                    "missing 'name' attribute\n", m_filename.c_str());
            throw std::logic_error("File contents are incomplete or corrupt");
        }
        foundName = true;
    }
    else
    {
        Log::error("GrandPrixData", "Error while trying to read grandprix file '%s' : "
                "Root node has an unexpected name\n", m_filename.c_str());
        throw std::logic_error("File contents are incomplete or corrupt");
    }


    const int amount = root->getNumNodes();
    for (int i=0; i<amount; i++)
    {
        const XMLNode* node = root->getNode(i);

        // read a track entry
        if (node->getName() == "track")
        {
            std::string trackID;
            int numLaps;
            bool reversed = false;

            const int idFound      = node->get("id",      &trackID  );
            const int lapFound     = node->get("laps",    &numLaps  );
            // Will stay false if not found
            node->get("reverse", &reversed );

            if (!idFound || !lapFound)
            {
                Log::error("GrandPrixData", "Error while trying to read grandprix file '%s' : "
                                "<track> tag does not have idi and laps reverse attributes. \n",
                                m_filename.c_str());
                throw std::logic_error("File contents are incomplete or corrupt");
            }

            // Make sure the track really is reversible
            Track* t = track_manager->getTrack(trackID);
            if (t != NULL && reversed)
            {
                reversed = t->reverseAvailable();
            }

            m_tracks.push_back(trackID);
            m_laps.push_back(numLaps);
            m_reversed.push_back(reversed);

            assert(m_tracks.size() == m_laps.size()    );
            assert(m_laps.size()   == m_reversed.size());
        }
        else
        {
            std::cerr << "Unknown node in Grand Prix XML file : " << node->getName().c_str() << std::endl;
            throw std::runtime_error("Unknown node in sfx XML file");
        }
    }// nend for

    // sanity checks
    if  (!foundName)
    {
        Log::error("GrandPrixData", "Error while trying to read grandprix file '%s' : "
                "missing 'name' attribute\n", m_filename.c_str());
        throw std::logic_error("File contents are incomplete or corrupt");
    }
}

// ----------------------------------------------------------------------------
bool GrandPrixData::writeToFile()
{
    try
    {
        UTFWriter file(m_filename.c_str());
        if (file.is_open())
        {
            file << L"\n<supertuxkart_grand_prix name=\"" << m_name << L"\">\n\n";
            for (unsigned int i = 0; i < m_tracks.size(); i++)
            {
                file <<
                    L"\t<track id=\""  << m_tracks[i] <<
                    L"\" laps=\""      << m_laps[i] <<
                    L"\" reverse=\""   << (m_reversed[i] ? L"true" : L"false") <<
                L"\" />\n";
            }
            file << L"\n</supertuxkart_grand_prix>\n";

            file.close();

            return true;
        }

        return false;
    }
    catch (std::runtime_error& e)
    {
        Log::error("GrandPrixData", "Failed to write '%s'; cause: %s\n",
            m_filename.c_str(), e.what());
        return false;
    }
}

// ----------------------------------------------------------------------------
bool GrandPrixData::checkConsistency(bool chatty) const
{
    for (unsigned int i = 0; i<m_tracks.size(); i++)
    {
        Track* t = track_manager->getTrack(m_tracks[i]);

        if (t == NULL)
        {
            if (chatty)
            {
                Log::error("GrandPrixData", "Grand Prix '%ls': Track '%s' does not exist!\n",
                                m_name.c_str(), m_tracks[i].c_str());
                Log::error("GrandPrixData", "This Grand Prix will not be available.\n");
            }
            return false;
        }

    }   // for i
    return true;
}   // checkConsistency


// ----------------------------------------------------------------------------
/** Returns true if the track is available. This is used to test if Fort Magma
 *  is available (this way FortMagma is not used in the last Grand Prix in
 *  story mode, but will be available once all challenges are done and nolok
 *  is unlocked).
 */
bool GrandPrixData::isTrackAvailable(const std::string &id) const
{
    return id!="fortmagma" ||
           !PlayerManager::get()->getCurrentPlayer()->isLocked("fortmagma");
}   // isTrackAvailable

// ----------------------------------------------------------------------------
void GrandPrixData::getLaps(std::vector<int> *laps) const
{
    laps->clear();
    for (unsigned int i = 0; i< m_tracks.size(); i++)
        if(isTrackAvailable(m_tracks[i]))
            laps->push_back(m_laps[i]);
}   // getLaps

// ----------------------------------------------------------------------------
void GrandPrixData::getReverse(std::vector<bool> *reverse) const
{
    reverse->clear();
    for (unsigned int i = 0; i< m_tracks.size(); i++)
        if(isTrackAvailable(m_tracks[i]))
            reverse->push_back(m_reversed[i]);
}   // getReverse

// ----------------------------------------------------------------------------
bool GrandPrixData::isEditable() const
{
    return m_editable;
}   // isEditable

// ----------------------------------------------------------------------------
unsigned int GrandPrixData::getNumberOfTracks() const
{
    return m_tracks.size();
}

// ----------------------------------------------------------------------------
irr::core::stringw GrandPrixData::getTrackName(const unsigned int track) const
{
    assert(track < getNumberOfTracks());
    Track* t = track_manager->getTrack(m_tracks[track]);
    assert(t != NULL);
    return t->getName();
}

// ----------------------------------------------------------------------------
const std::string& GrandPrixData::getTrackId(const unsigned int track) const
{
    assert(track < getNumberOfTracks());
    return m_tracks[track];
}

// ----------------------------------------------------------------------------
unsigned int GrandPrixData::getLaps(const unsigned int track) const
{
    assert(track < getNumberOfTracks());
    return m_laps[track];
}

// ----------------------------------------------------------------------------
bool GrandPrixData::getReverse(const unsigned int track) const
{
    assert(track < getNumberOfTracks());
    return m_reversed[track];
}

// ----------------------------------------------------------------------------
void GrandPrixData::moveUp(const unsigned int track)
{
    assert (track > 0 && track < getNumberOfTracks());

    std::swap(m_tracks[track], m_tracks[track - 1]);
    std::swap(m_laps[track], m_laps[track - 1]);
    m_reversed.swap(m_reversed[track], m_reversed[track - 1]);
}

// ----------------------------------------------------------------------------
void GrandPrixData::moveDown(const unsigned int track)
{
    assert (track < (getNumberOfTracks() - 1));

    std::swap(m_tracks[track], m_tracks[track + 1]);
    std::swap(m_laps[track], m_laps[track + 1]);
    m_reversed.swap(m_reversed[track], m_reversed[track + 1]);
}

// ----------------------------------------------------------------------------
void GrandPrixData::addTrack(Track* track, unsigned int laps, bool reverse,
    int position)
{
    int n;

    n = getNumberOfTracks();
    assert (track != NULL);
    assert (laps > 0);
    assert (position >= -1 && position < n);

    if (position < 0 || position == (n - 1) || m_tracks.empty())
    {
        //Append new track to the end of the list
        m_tracks.push_back(track->getIdent());
        m_laps.push_back(laps);
        m_reversed.push_back(reverse);
    }
    else
    {
        //Insert new track right after the specified position. Caution:
        //std::vector inserts elements _before_ the specified position
        m_tracks.insert(m_tracks.begin() + position + 1, track->getIdent());
        m_laps.insert(m_laps.begin() + position + 1, laps);
        m_reversed.insert(m_reversed.begin() + position + 1, reverse);
    }
}

// ----------------------------------------------------------------------------
void GrandPrixData::editTrack(unsigned int t, Track* track,
    unsigned int laps, bool reverse)
{
    assert (t < getNumberOfTracks());
    assert (track != NULL);
    assert (laps > 0);

    m_tracks[t] = track->getIdent();
    m_laps[t] = laps;
    m_reversed[t] = reverse;
}

// ----------------------------------------------------------------------------
void GrandPrixData::remove(const unsigned int track)
{
    assert (track < getNumberOfTracks());

    m_tracks.erase(m_tracks.begin() + track);
    m_laps.erase(m_laps.begin() + track);
    m_reversed.erase(m_reversed.begin() + track);
}

// ----------------------------------------------------------------------------
const std::vector<std::string>& GrandPrixData::getTrackNames() const
{
    m_really_available_tracks.clear();
    for (unsigned int i = 0; i < m_tracks.size(); i++)
    {
        if(isTrackAvailable(m_tracks[i]))
            m_really_available_tracks.push_back(m_tracks[i]);
    }   // for i
    return m_really_available_tracks;
}   // getTrackNames

/* EOF */
