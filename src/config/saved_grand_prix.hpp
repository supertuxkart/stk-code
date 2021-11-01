//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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

#ifndef HEADER_SAVED_GRAND_PRIX_HPP
#define HEADER_SAVED_GRAND_PRIX_HPP

#include "config/user_config.hpp"
#include "race/race_manager.hpp"
#include "utils/ptr_vector.hpp"

#include <algorithm>
#include <string>

class RaceManager;

// ============================================================================

/**
  * \brief Class for managing saved Grand-Prix's
  * A list of all possible resumable GP's is stored in the user config.
  * \ingroup config
  */
class SavedGrandPrix
{
private:
    class SavedGPKart
    {
        friend class SavedGrandPrix;
        GroupUserConfigParam m_group;
        StringUserConfigParam m_ident;
        IntUserConfigParam m_score, m_local_player_id, m_global_player_id;
        FloatUserConfigParam m_overall_time;
    public:
        SavedGPKart(GroupUserConfigParam * group, const XMLNode* node);
        SavedGPKart(GroupUserConfigParam * group,
                    const std::string &ident,
                    int score,
                    int local_player_id,
                    int global_player_id,
                    float overall_time);
    };   // SavedGPKart

protected:

    /**
     * For saving to config file.
     * WARNING : m_savedgp_group has to be declared before the other userconfigparams!
     */
    GroupUserConfigParam        m_savedgp_group;
    IntUserConfigParam          m_player_id;

    /** Identifier of this GP. */
    StringUserConfigParam       m_gp_id;
    
    /** Race type at which this GP was run. */
    IntUserConfigParam          m_race_type;

    /** Difficulty at which this GP was run. */
    IntUserConfigParam          m_difficulty;

    /** Number of player karts used in this GP. */
    IntUserConfigParam          m_player_karts;

    /** Index of the next to run track. */
    IntUserConfigParam          m_next_track;

    /** GPReverseType of the GP as int */
    IntUserConfigParam          m_reverse_type;

    /** Count of tracks that player skipped */
    IntUserConfigParam          m_skipped_tracks;

    /** Time target used in Lap Trial mode */
    FloatUserConfigParam        m_time_target;

    /** Total laps count, used in Lap Trial mode */
    IntUserConfigParam          m_player_total_laps;

    PtrVector<SavedGPKart> m_karts;

public:

    /**
      * Constructor to create a new entry.
      */
    SavedGrandPrix(unsigned int player_id,
                   const std::string &gp_id,
                   RaceManager::MinorRaceModeType race_type,
                   RaceManager::Difficulty difficulty,
                   int player_karts,
                   int last_track,
                   int reverse_type,
                   int skipped_tracks,
                   float time_target,
                   int player_total_laps,
                   const std::vector<RaceManager::KartStatus> &kart_list);

    /**
      * Constructor to deserialize a entry that was saved to a XML file
      */
    SavedGrandPrix(const XMLNode* node);
    void setKarts(const std::vector<RaceManager::KartStatus> &kart_list);
    void clearKarts();
    void loadKarts(std::vector<RaceManager::KartStatus> & kart_list);

    // ------------------------------------------------------------------------
    /** Returns the player id for this saved GP. */
    unsigned int getPlayerID() const { return m_player_id; }

    // ------------------------------------------------------------------------
    /** Returns the grand prix id. */
    std::string getGPID() const { return m_gp_id; }

    // ------------------------------------------------------------------------
    /** Returns the race type of this GP. */
    int getRaceType() const { return m_race_type; }

    // ------------------------------------------------------------------------
    /** Returns the difficulty of this GP. */
    int getDifficulty() const { return m_difficulty; }

    // ------------------------------------------------------------------------
    /** Returns the total number of karts of this GP. */
    int getTotalKarts() const { return m_karts.size(); }

    // ------------------------------------------------------------------------
    /** Returns the number of player karts in this GP. */
    int getPlayerKarts() const { return m_player_karts; }

    // ------------------------------------------------------------------------
    /** Returns the index of the last track finished when this GP was saved. */
    int getNextTrack() const { return m_next_track; }

    // ------------------------------------------------------------------------
    /** Returns the reverse Type. */
    int getReverseType() const { return m_reverse_type; }

    // ------------------------------------------------------------------------
    /** Returns skipped tracks count */
    int getSkippedTracks() const { return m_skipped_tracks; }

    // ------------------------------------------------------------------------
    /** Returns time target used in Lap Trial mode */
    float getTimeTarget() const { return m_time_target; }

    // ------------------------------------------------------------------------
    /** Returns total laps in GP */
    int getPlayerTotalLaps() const { return m_player_total_laps; }

    // ------------------------------------------------------------------------
    /** Sets the index of the last track finished. */
    void setNextTrack(int next_track) { m_next_track = next_track; }

    // ------------------------------------------------------------------------
    /** Removed this SavedGrandPrix from the list of all SavedGrandPrix, and
     *  deletes it. */
    void remove()
    {
        UserConfigParams::m_saved_grand_prix_list.remove(this);
        delete this;
    }   // remove

    // ------------------------------------------------------------------------
    /** Finds the right SavedGrandPrix given the specified data, or
     *  NULL if no matching GP was found. */
    static SavedGrandPrix* getSavedGP(unsigned int player,
                                      const std::string &gpid,
                                      RaceManager::MinorRaceModeType race_type,
                                      const unsigned int number_of_players)
    {
        for (unsigned int n=0; n<UserConfigParams::m_saved_grand_prix_list.size(); n++)
        {
            SavedGrandPrix* gp = &UserConfigParams::m_saved_grand_prix_list[n];
            if (gp->getGPID()        == gpid   &&
                gp->getRaceType()    == race_type &&
                gp->getPlayerID()    == player &&
                gp->getPlayerKarts() == (int)number_of_players)
                return gp;
        }
        return NULL;
    }   // getSavedGP
    // ------------------------------------------------------------------------
};   // class SavedGrandPrix

#endif

/*EOF*/
