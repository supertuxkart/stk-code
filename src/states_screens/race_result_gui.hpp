//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#ifndef HEADER_RACE_RESULT_GUI_HPP
#define HEADER_RACE_RESULT_GUI_HPP


#include "guiengine/screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/race_gui_base.hpp"
#include "states_screens/state_manager.hpp"

#include <assert.h>
#include <vector>


namespace irr
{
    namespace gui
    {
        class ScalableFont;
    }
}

class MusicInformation;
class SFXBase;

/**
  * \brief Displays the results (while the end animation is shown).
  * \ingroup states_screens
  */
class RaceResultGUI : public RaceGUIBase,
                      public GUIEngine::Screen,
                      public GUIEngine::ScreenSingleton<RaceResultGUI>,
                      public MessageDialog::IConfirmDialogListener
{
private:
    /** Timer variable for animations. */
    float                      m_timer;

    /** Finite state machine for the animations:
        INIT:            Set up data structures.
        RACE_RESULT:     The rows scroll into place.
        WAITING_GP_RESULT: Waiting for user pressing continue button
        OLD_GP_TABLE:    Scroll new table into place, sorted by previous
                         GP ranks
        INCREASE_POINTS: The overall points are added up
        RESORT_TABLE:    Resort the table so that it is now sorted by
                         GP points.
        WAIT_TILL_END    Some delay to wait for end, after a period it
                         wii automatically end. */
    enum                       {RR_INIT,
                                RR_RACE_RESULT,
                                RR_WAITING_GP_RESULT,
                                RR_OLD_GP_RESULTS,
                                RR_INCREASE_POINTS,
                                RR_RESORT_TABLE,
                                RR_WAIT_TILL_END}
                               m_animation_state;

    class RowInfo
    {
    public:
        /** Kart ID in World */
        unsigned int     m_kart_id;
        /** Start time for each line of the animation. */
        float            m_start_at;
        /** Currenct X position. */
        float            m_x_pos;
        /** Currenct Y position. */
        float            m_y_pos;
        /** True if kart is a player kart. */
        bool             m_is_player_kart;
        /** The radius to use when sorting the entries. Positive values
            will rotate downwards, negatives are upwards. */
        float            m_radius;
        /** The center point when sorting the entries. */
        float            m_centre_point;
        /** The names of all karts in the right order. */
        core::stringw    m_kart_name;
        /** Points earned in this race. */
        float            m_new_points;
        /** New overall points after this race. */
        int              m_new_overall_points;
        /** New GP rank after this race. */
        int              m_new_gp_rank;
        /** When updating the number of points in the display, this is the
            currently displayed number of points. This is a floating point number
            since it stores the increments during increasing the points. */
        float            m_current_displayed_points;
        /** The kart icons. */
        video::ITexture *m_kart_icon;
        /** The times of all karts in the right order. */
        core::stringw    m_finish_time_string;
        /** The kart color */
        float            m_kart_color;
        /** Number of laps that kart finished */
        unsigned int     m_laps;
    };   // Rowinfo

    /** The team icons. */

    std::vector<RowInfo>       m_all_row_infos, m_all_row_info_waiting;

    /** Time to wait till the next row starts to be animated. */
    float                      m_time_between_rows;

    /** The time a single line scrolls into place. */
    float                      m_time_single_scroll;

    /** Time to rotate the GP entries. */
    float                      m_time_rotation;

    /** The time for inreasing the points by one during the
        point update phase. */
    float                      m_time_for_points;

    /** The overall time the first phase (scrolling) is displayed.
        This includes a small waiting time at the end. */
    float                      m_time_overall_scroll;

    /** Distance between each row of the race results */
    unsigned int               m_distance_between_rows;

    /** Distance between each row of the highscore, race data, etc.*/
    unsigned int               m_distance_between_meta_rows;

    /** The size of the kart icons. */
    unsigned int               m_width_icon;

    /** Width of the kart name column. */
    unsigned int               m_width_kart_name;

    /** Width of the finish time column. */
    unsigned int               m_width_finish_time;

    /** Width of the new points columns. */
    unsigned int               m_width_new_points;

    /** Position of left end of table (so that the whole
        table is aligned. */
    unsigned int               m_leftmost_column;

    /** Top-most pixel for first row. */
    unsigned int               m_top;

    /** Size of space between columns. */
    unsigned int               m_width_column_space;

    /** The overall width of the table. */
    unsigned int               m_table_width;

    /** The font to use. */
    gui::ScalableFont         *m_font;

    /** True if a GP position was changed. If not, the point increase
     *  animation can be skipped. */
    bool                       m_gp_position_was_changed;

    bool                       m_started_race_over_music;

    /** The previous monospace state of the font. */
    //bool                       m_was_monospace;

    /** Sound effect at end of race. */
    SFXBase                   *m_finish_sound;

    /** Music to be played after race ended. */
    MusicInformation          *m_race_over_music;

    /** For highscores */
    int m_highscore_rank;

    unsigned int m_width_all_points;

    int m_max_tracks;
    int m_start_track;
    int m_end_track;
    int m_sshot_height;

    PtrVector<GUIEngine::Widget, HOLD>  m_gp_progress_widgets;

    static const int SSHOT_SEPARATION = 10;

    void displayOneEntry(unsigned int x, unsigned int y,
                         unsigned int n, bool display_points);
    void determineTableLayout();
    void determineGPLayout();
    void enableAllButtons();
    void enableGPProgress();
    void addGPProgressWidget(GUIEngine::Widget* widget);
    void displayGPProgress();
    void displayPostRaceInfo();
    void displayCTFResults();
    void displaySoccerResults();
    void displayScreenShots();

    int  getFontHeight () const;

public:

                 RaceResultGUI();
    virtual void renderGlobal(float dt) OVERRIDE;

    /** \brief Implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE {};

    virtual void init() OVERRIDE;
    virtual void tearDown() OVERRIDE;
    virtual bool onEscapePressed() OVERRIDE;
    virtual void unload() OVERRIDE;
    virtual GUIEngine::EventPropagation
                 filterActions(PlayerAction action, int deviceID, const unsigned int value,
                               Input::InputType type, int playerId) OVERRIDE;
    void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                       const int playerID) OVERRIDE;
    friend class GUIEngine::ScreenSingleton<RaceResultGUI>;

    /** Should not be called anymore.  */
    const core::dimension2du getMiniMapSize() const OVERRIDE
                  { return core::dimension2du(0, 0); }

    /** No kart specific view needs to be rendered in the result gui. */
    virtual void renderPlayerView(const Camera *camera, float dt) OVERRIDE {}

    virtual void onUpdate(float dt) OVERRIDE;
    virtual void onDraw(float dt) OVERRIDE;

    /** No more messages need to be displayed, but the function might still be
     *  called (e.g. 'new lap' message if the end controller is used for more
     *  than one lap). So do nothing in this case.
    */
    virtual void addMessage(const irr::core::stringw &m,
                            const AbstractKart *kart,
                            float time,
                            const video::SColor &color=
                                video::SColor(255, 255, 0, 255),
                            bool important=true,
                            bool big_font=false,
                            bool outline=false) OVERRIDE { }

    void nextPhase();

    /** Show no highscore */
    void clearHighscores();

    /**
      * To call if the user got a new highscore
      * \param kart identity of the kart that made the highscore
      * \param player identity of the player that made the highscore
      * \param rank Highscore rank (first highscore, second highscore, etc.). This is not the race rank
      * \param time Finish time in seconds
      */
    void setHighscore(int rank);

    virtual void onConfirm() OVERRIDE;
    void cleanupGPProgress();
};   // RaceResultGUI

#endif
