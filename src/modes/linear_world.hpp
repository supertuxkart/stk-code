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

#ifndef HEADER_LINEAR_WORLD_HPP
#define HEADER_LINEAR_WORLD_HPP

#include <vector>

#include "modes/world_with_rank.hpp"
#include "utils/aligned_array.hpp"

class SFXBase;

/*
 * A 'linear world' is a subcategory of world used in 'standard' races, i.e.
 * with a start line and a road that loops. This includes management of drivelines
 * and lap counting.
 * \ingroup modes
 */
class LinearWorld : public WorldWithRank
{
    /** Sfx for the final lap. */
    SFXBase     *m_last_lap_sfx;

    /** Last lap sfx should only be played once. */
    bool         m_last_lap_sfx_played;
    
    bool         m_last_lap_sfx_playing;

private:
    /** Some additional info that needs to be kept for each kart
     * in this kind of race.
     */
    struct KartInfo
    {
        int         m_race_lap;             /**<Number of finished(!) laps. */
        float       m_time_at_last_lap;     /**<Time at finishing last lap. */
        float       m_lap_start_time;       /**<Time at start of a new lap. */
        float       m_estimated_finish;     /**<During last lap only:
                                            *  estimated finishing time!   */
        int         m_track_sector;         /**<Index in driveline, special values
                                            * e.g. UNKNOWN_SECTOR can be negative!*/

        int         m_last_valid_sector;    /* used when rescusing, e.g. for invalid shortcuts */

        Vec3        m_curr_track_coords;
        /** True if the kart is on top of the road path drawn by the drivelines */
        bool        m_on_road;

    };

protected:
    RaceGUIBase::KartIconDisplayInfo* m_kart_display_info;

    /** This vector contains an 'KartInfo' struct for every kart in the race.
      * This member is not strictly private but try not to use it directly outside
      * tightly related classes (e.g. AI)
      */
    AlignedArray<KartInfo> m_kart_info;

    
    /** Linear races can trigger rescues for one additional reason : shortcuts.
      * It may need to do some specific world before calling the generic Kart::forceRescue
      */
    void          rescueKartAfterShortcut(Kart* kart, KartInfo& kart_info);
    void          checkForWrongDirection(unsigned int i);
    void          updateRacePosition();
    virtual float estimateFinishTimeForKart(Kart* kart);

public:
                  LinearWorld();
   /** call just after instanciating. can't be moved to the contructor as child
       classes must be instanciated, otherwise polymorphism will fail and the
       results will be incorrect */
    virtual void  init();
    virtual      ~LinearWorld();

    virtual void  update(float delta);
    int           getSectorForKart(const int kart_id) const;
    float         getDistanceDownTrackForKart(const int kart_id) const;
    float         getDistanceToCenterForKart(const int kart_id) const;
    float         getEstimatedFinishTime(const int kart_id) const;
    int           getLapForKart(const int kart_id) const;
    float         getTimeAtLapForKart(const int kart_id) const;

    virtual  RaceGUIBase::KartIconDisplayInfo* 
                  getKartsDisplayInfo();
    virtual void  moveKartAfterRescue(Kart* kart);
    virtual void  restartRace();
    
    virtual bool  raceHasLaps(){ return true; }
    virtual void  newLap(unsigned int kart_index);

    virtual bool  haveBonusBoxes(){ return true; }
    
    /** Returns true if the kart is on a valid driveline quad.
     *  \param kart_index  Index of the kart.
     */
    bool          isOnRoad(unsigned int kart_index) const 
                 { return m_kart_info[kart_index].m_on_road; }
};   // LinearWorld

#endif
