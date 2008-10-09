//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#ifndef _LINEAR_WORLD_H_
#define _LINEAR_WORLD_H_

#include "modes/world.hpp"
#include <vector>

struct KartIconDisplayInfo;
class RaceGUI;

/** Some additional info that needs to be kept for each kart
  * in this kind of race.
  */
struct KartInfo
{
    int         m_race_lap;             // number of finished(!) laps
    float       m_time_at_last_lap;     // time at finishing last lap
    float       m_lap_start_time;       // Time at start of a new lap
    int         m_track_sector;         // index in driveline, special values
                                        // e.g. UNKNOWN_SECTOR can be negative!
    int         m_last_valid_sector;
    Vec3        m_curr_track_coords;
    Vec3        m_last_track_coords;
    
    bool        m_on_road;             // true if the kart is on top of the
                                       // road path drawn by the drivelines
};

/*
 * A 'linear world' is a subcategory of world used in 'standard' races, i.e.
 * with a start line and a road that loops. This includes management of drivelines
 * and lap counting.
 */
class LinearWorld : public World
{
protected:
    KartIconDisplayInfo* m_kart_display_info;
    
    /** Linear races can trigger rescues for one additional reason : shortcuts.
    * It may need to do some specific world before calling the generic Kart::forceRescue
    */
    void            forceRescue(Kart* kart, KartInfo& kart_info, bool shortcut);
    
    void            doLapCounting ( KartInfo& kart_info, Kart* kart );
    
public:
    LinearWorld();
    virtual ~LinearWorld();
    
    /** This vector contains an 'KartInfo' struct for every kart in the race.
      * This member is not strictly private but try not to use it directly outside
      * tightly related classes (e.g. AI)
      */
    std::vector<KartInfo> m_kart_info;

    virtual void    update(float delta);
    
    int             getSectorForKart(const int kart_id) const;
    float           getDistanceDownTrackForKart(const int kart_id) const;
    float           getDistanceToCenterForKart(const int kart_id) const;
    int             getLapForKart(const int kart_id) const;
    void            setTimeAtLapForKart(float t, const int kart_id);
    float           getTimeAtLapForKart(const int kart_id) const;

    virtual KartIconDisplayInfo* getKartsDisplayInfo(const RaceGUI* caller);
    virtual void moveKartAfterRescue(Kart* kart, btRigidBody* body);
    
    virtual void    terminateRace();
    virtual void    restartRace();
    
    float           estimateFinishTimeForKart  (Kart* kart, KartInfo& kart_info);
    void            updateRacePosition ( Kart* kart, KartInfo& kart_info );
    
    virtual bool raceHasLaps(){ return true; }
};

#endif
