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

#ifndef HEADER_LINEAR_WORLD_HPP
#define HEADER_LINEAR_WORLD_HPP

#include "modes/world_with_rank.hpp"
#include "utils/aligned_array.hpp"

#include <climits>
#include <vector>

class SFXBase;

/*
 * A 'linear world' is a subcategory of world used in 'standard' races, i.e.
 * with a start line and a road that loops. This includes management of drivelines
 * and lap counting.
 * \ingroup modes
 */
class LinearWorld : public WorldWithRank
{
private:
    /** Sfx for the final lap. */
    SFXBase     *m_last_lap_sfx;

    /** Last lap sfx should only be played once. */
    bool         m_last_lap_sfx_played;

    bool         m_last_lap_sfx_playing;

    /** The fastest lap time, in ticks of physics dt. */
    int          m_fastest_lap_ticks;

    /** The track length returned by Track::getLength() only covers the
     *  distance from start line to finish line, i.e. it does not include
     *  the distance the karts actually start behind the start line (the
     *  karts would have a negative distance till they reach the start line
     *  for the first time). This values stores the additional distance by
     *  which the track length must be increased, which is important to
     *  get valid finish times estimates. */
    float       m_distance_increase;

    // ------------------------------------------------------------------------
    /** Some additional info that needs to be kept for each kart
     * in this kind of race.
     */
    class KartInfo
    {
    public:
        /** Number of finished laps. */
        int         m_finished_laps;

        /** Time at finishing last lap. */
        int         m_ticks_at_last_lap;

        /** Time at start of a new lap. */
        int         m_lap_start_ticks;

        /** During last lap only: estimated finishing time!   */
        float       m_estimated_finish;

        /** How far the kart has travelled (this is (number-of-laps-1) times
         *  track-length plus distance-along-track). */
        float       m_overall_distance;

        /** Accumulates the time a kart has been driving in the wrong
         *  direction so that a message can be displayed. */
        float       m_wrong_way_timer;

        /** Initialises all fields. */
        KartInfo()  { reset(); }
        // --------------------------------------------------------------------
        /** Re-initialises all data. */
        void reset()
        {
            m_finished_laps     = -1;
            m_lap_start_ticks   = 0;
            m_ticks_at_last_lap = INT_MAX;
            m_estimated_finish  = -1.0f;
            m_overall_distance  = 0.0f;
            m_wrong_way_timer   = 0.0f;
        }   // reset
    };
    // ------------------------------------------------------------------------

protected:

    /** This vector contains an 'KartInfo' struct for every kart in the race.
      * This member is not strictly private but try not to use it directly outside
      * tightly related classes (e.g. AI)
      */
    AlignedArray<KartInfo> m_kart_info;

    virtual void  checkForWrongDirection(unsigned int i, float dt);
    void          updateRacePosition();
    virtual float estimateFinishTimeForKart(AbstractKart* kart) OVERRIDE;

public:
                  LinearWorld();
   /** call just after instanciating. can't be moved to the contructor as child
       classes must be instanciated, otherwise polymorphism will fail and the
       results will be incorrect */
    virtual void  init() OVERRIDE;
    virtual      ~LinearWorld();

    virtual void  update(int ticks) OVERRIDE;
    virtual void  updateGraphics(float dt) OVERRIDE;
    float         getDistanceDownTrackForKart(const int kart_id) const;
    float         getDistanceDownTrackForKart(const int kart_id,
                                            bool account_for_checklines) const;
    float         getDistanceToCenterForKart(const int kart_id) const;
    float         getEstimatedFinishTime(const int kart_id) const;
    int           getLapForKart(const int kart_id) const;
    int           getTicksAtLapForKart(const int kart_id) const;

    virtual  void getKartsDisplayInfo(
                  std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE;

    virtual unsigned int getNumberOfRescuePositions() const OVERRIDE;
    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) OVERRIDE;
    virtual btTransform getRescueTransform(unsigned int index) const OVERRIDE;
    virtual void  reset() OVERRIDE;
    virtual void  newLap(unsigned int kart_index) OVERRIDE;

    // ------------------------------------------------------------------------
    /** Returns if this race mode has laps. */
    virtual bool  raceHasLaps() OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    /** Returns if this race mode has bonus items. */
    virtual bool  haveBonusBoxes() OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    /** Override settings from base class */
    virtual bool useChecklineRequirements() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    /** Returns the number of laps a kart has completed.
     *  \param kart_index World index of the kart. */
    int getFinishedLapsOfKart(unsigned int kart_index) const OVERRIDE
    {
        assert(kart_index < m_kart_info.size());
        return m_kart_info[kart_index].m_finished_laps;
    }   // getkartLap
    // ------------------------------------------------------------------------
    void setLastTriggeredCheckline(unsigned int kart_index, int index);
    // ------------------------------------------------------------------------
    /** Returns how far the kart has driven so far (i.e.
     *  number-of-laps-finished times track-length plus distance-on-track.
     *  \param kart_index World kart id of the kart. */
    float getOverallDistance(unsigned int kart_index) const
    {
        return m_kart_info[kart_index].m_overall_distance;
    }   // getOverallDistance
    // ------------------------------------------------------------------------
    /** Returns time for the fastest laps */
    float getFastestLap() const
    {
        return stk_config->ticks2Time(m_fastest_lap_ticks);
    }
    // ------------------------------------------------------------------------
    /** Network use: get fastest lap in ticks */
    int getFastestLapTicks() const
    {
        return m_fastest_lap_ticks;
    }
    // ------------------------------------------------------------------------
    /** Network use: set fastest lap in ticks */
    void setFastestLapTicks(int ticks)
    {
        m_fastest_lap_ticks = ticks;
    }
};   // LinearWorld

#endif
