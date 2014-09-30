//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 SuperTuxKart-Team
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
#include "tracks/track_sector.hpp"
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
private:
    /** Sfx for the final lap. */
    SFXBase     *m_last_lap_sfx;

    /** Last lap sfx should only be played once. */
    bool         m_last_lap_sfx_played;

    bool         m_last_lap_sfx_playing;

    /** The fastest lap time. */
    float       m_fastest_lap;

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
        /** Number of finished(!) laps. */
        int         m_race_lap;

        /** Time at finishing last lap. */
        float       m_time_at_last_lap;

        /** Time at start of a new lap. */
        float       m_lap_start_time;

        /** During last lap only: estimated finishing time!   */
        float       m_estimated_finish;

        /** How far the kart has travelled (this is (number-of-laps-1) times
         *  track-length plus distance-along-track). */
        float       m_overall_distance;

        /** Stores the current graph node and track coordinates etc. */
        TrackSector m_track_sector;

        /** Initialises all fields. */
        KartInfo()  { reset(); }
        // --------------------------------------------------------------------
        /** Re-initialises all data. */
        void reset()
        {
            m_race_lap         = -1;
            m_lap_start_time   = 0;
            m_time_at_last_lap = 99999.9f;
            m_estimated_finish = -1.0f;
            m_overall_distance = 0.0f;
            m_track_sector.reset();
        }   // reset
        // --------------------------------------------------------------------
        /** Returns a pointer to the current node object. */
        TrackSector *getTrackSector() {return &m_track_sector; }
        // --------------------------------------------------------------------
        /** Returns a pointer to the current node object. */
        const TrackSector *getTrackSector() const {return &m_track_sector; }
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

    virtual void  update(float delta) OVERRIDE;
    int           getSectorForKart(const AbstractKart *kart) const;
    float         getDistanceDownTrackForKart(const int kart_id) const;
    float         getDistanceToCenterForKart(const int kart_id) const;
    float         getEstimatedFinishTime(const int kart_id) const;
    int           getLapForKart(const int kart_id) const;
    float         getTimeAtLapForKart(const int kart_id) const;

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
    /** Returns true if the kart is on a valid driveline quad.
     *  \param kart_index  Index of the kart. */
    bool isOnRoad(unsigned int kart_index) const
    {
        return m_kart_info[kart_index].getTrackSector()->isOnRoad();
    }   // isOnRoad

    // ------------------------------------------------------------------------
    /** Returns the number of laps a kart has completed.
     *  \param kart_index World index of the kart. */
    int getKartLaps(unsigned int kart_index) const
    {
        assert(kart_index < m_kart_info.size());
        return m_kart_info[kart_index].m_race_lap;
    }   // getkartLap

    // ------------------------------------------------------------------------
    /** Returns the track_sector object for the specified kart.
     *  \param kart_index World index of the kart. */
    TrackSector& getTrackSector(unsigned int kart_index)
    {
        return m_kart_info[kart_index].m_track_sector;
    }   // getTrackSector

    // ------------------------------------------------------------------------
    /** Returns how far the kart has driven so far (i.e.
     *  number-of-laps-finished times track-length plus distance-on-track.
     *  \param kart_index World kart id of the kart. */
    float getOverallDistance(unsigned int kart_index) const
    {
        return m_kart_info[kart_index].m_overall_distance;
    }   // getOverallDistance
};   // LinearWorld

#endif
