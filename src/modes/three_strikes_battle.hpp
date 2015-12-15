//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#ifndef THREE_STRIKES_BATTLE_HPP
#define THREE_STRIKES_BATTLE_HPP


#include "modes/world_with_rank.hpp"
#include "tracks/track_object.hpp"
#include "states_screens/race_gui_base.hpp"

#include <IMesh.h>

#include <string>

class PhysicalObject;

/**
 * \brief An implementation of World, to provide the 3 strikes battle game mode
 * \ingroup modes
 */
class ThreeStrikesBattle : public WorldWithRank
{
private:
    struct BattleInfo
    {
        int m_lives;
    };

    /** This vector contains an 'BattleInfo' struct for every kart in the race.
    */
    std::vector<BattleInfo> m_kart_info;

    /** The mesh of the tire which is displayed when a kart loses a life. */
    irr::scene::IMesh* m_tire;

    /** Indicates the number of tires that should be
     *  inserted into the track. */
    int m_insert_tire;

    /** For tires that are blown away. */
    core::vector3df m_tire_position;

    /** The original locations of the tires of a kart. */
    core::vector3df m_tire_offsets[4];

    /** The radius of the karts original tires. */
    float m_tire_radius[4];

    /** The directory of the original kart tires. */
    std::string m_tire_dir;

    /** A rotation to apply to the tires when inserting them. */
    float m_tire_rotation;

    PtrVector<TrackObject, REF> m_tires;

    /** Function to udpate the locations of all karts on the polygon map */
    void updateKartNodes();

public:

    /** Used to show a nice graph when battle is over */
    struct BattleEvent
    {
        float m_time;
        std::vector<BattleInfo> m_kart_info;
    };
    std::vector<BattleEvent> m_battle_events;

    ThreeStrikesBattle();
    virtual ~ThreeStrikesBattle();

    virtual void init();

    // clock events
    virtual bool isRaceOver();
    virtual void terminateRace();

    // overriding World methods
    virtual void reset();

    //virtual void getDefaultCollectibles(int& collectible_type, int& amount);
    virtual bool useFastMusicNearEnd() const { return false; }
    virtual void getKartsDisplayInfo(
                          std::vector<RaceGUIBase::KartIconDisplayInfo> *info);
    virtual bool raceHasLaps(){ return false; }

    virtual const std::string& getIdent() const;

    virtual void kartHit(const unsigned int kart_id);
    virtual void update(float dt);

    virtual void kartAdded(AbstractKart* kart, scene::ISceneNode* node);


    void updateKartRanks();
};   // ThreeStrikesBattles


#endif
