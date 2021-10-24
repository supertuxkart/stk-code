//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_STK_CONFIG_HPP
#define HEADER_STK_CONFIG_HPP

/**
 * \defgroup config
 * This module handles the user configuration, the supertuxkart configuration
 * file (which contains options usually not edited by the player) and the input
 * configuration file.
 */

#include "network/remote_kart_info.hpp"
#include "utils/interpolation_array.hpp"
#include "utils/no_copy.hpp"

#include "utils/constants.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class KartProperties;
class MusicInformation;
class XMLNode;

/**
 * \brief Global STK configuration information.
 * Parameters here can be tuned without recompilation, but the user shouldn't actually modify
 * them. It also includes the list of default kart physics parameters which are used for
 * each kart (but which can be overwritten for each kart, too).
 * \ingroup config
 */
class STKConfig : public NoCopy
{
protected:
    /** Default kart properties. */
    KartProperties *m_default_kart_properties;
    std::map<std::string, KartProperties*> m_kart_properties;

public:
    /** What to do if a kart already has a powerup when it hits a bonus box:
     *  - NEW:  give it a random new bonx box.
     *  - SAME: give it one more item of the type it currently has.
     *  - ONLY_IF_SAME: only give it one more item if the randomly chosen item
     *          has the same type as the currently held item. */
    enum {
        POWERUP_MODE_NEW,
        POWERUP_MODE_SAME,
        POWERUP_MODE_ONLY_IF_SAME
    }
    m_same_powerup_mode;

    int   m_bonusbox_item_return_ticks;  /** Time until a bonus box collected by a kart returns */
    int   m_nitro_item_return_ticks;     /** Time until a nitro collected by a kart returns */
    int   m_banana_item_return_ticks;    /** Time until a banana collected by a kart return */
    int   m_bubblegum_item_return_ticks; /** Time until a bubble gum collected bý a kart returns */

    static float UNDEFINED;
    float m_bomb_time;                 /**<Time before a bomb explodes.        */
    float m_bomb_time_increase;        /**<Time added to bomb timer when it's
                                           passed on.                          */
    int   m_item_switch_ticks;          /**< Time items will be switched.       */
    int   m_bubblegum_counter;         /**< How many times bubble gums must be
                                            driven over before they disappear. */
    bool  m_shield_restrict_weapons;   /**<Wether weapon usage is punished. */
    float m_explosion_impulse_objects; /**<Impulse of explosion on moving
                                            objects, e.g. road cones, ...      */
    int   m_penalty_ticks;              /**< Penalty time when starting too
                                            early.                             */
    float m_delay_finish_time;         /**<Delay after a race finished before
                                           the results are displayed.          */
    float m_music_credit_time;         /**<Time the music credits are
                                           displayed.                          */
    int   m_max_karts;                 /**<Maximum number of karts.            */
    bool  m_smooth_normals;            /**< If normals for raycasts for wheels
                                           should be interpolated.             */

    /** Prevent early explosive items before this world time. */
    float m_no_explosive_items_timeout;

    /** Maximum number of moveable objects in a track when networking is on. */
    int m_max_moveable_objects;

    /** In case of a network race, remote karts will get their steering somewhat
     *  reduced each frame. This reduces stutter when a kart only does small
     *  steering adjustments. */
    float m_network_steering_reduction;

    /** If the angle between a normal on a vertex and the normal of the
     *  triangle are more than this value, the physics will use the normal
     *  of the triangle in smoothing normal. */
    float m_smooth_angle_limit;

    /** Default friction for the track and any track/library object. */
    float m_default_track_friction;

    /** Default friction to be used for any moveable, e.g. karts, balls. */
    float m_default_moveable_friction;

    /** Number of solver iterations. */
    int m_solver_iterations;

    /** If position and velocity constraints are solved separately. */
    bool m_solver_split_impulse;

    /** Threshold when to use the split impulse approach. */
    float m_solver_split_impulse_thresh;

    /** Bit flags to modify the solver mode. Bits set in set_flags are
     *  added to the solver mode, bits set in reset_flags are removed. */
    int m_solver_set_flags, m_solver_reset_flags;

    int   m_max_skidmarks;           /**<Maximum number of skid marks/kart.  */
    float m_skid_fadeout_time;       /**<Time till skidmarks fade away.      */
    float m_near_ground;             /**<Determines when a kart is not near
                                      *  ground anymore and the upright
                                      *  constraint is disabled to allow for
                                      *  more violent explosions.            */
    int   m_min_kart_version,        /**<The minimum and maximum .kart file  */
        m_max_kart_version;        /**<version supported by this binary.   */
    int   m_min_track_version,       /**<The minimum and maximum .track file */
        m_max_track_version;       /**<version supported by this binary.   */
    int   m_min_server_version,       /**<The minimum and maximum server */
        m_max_server_version;       /**<version supported by this binary.   */
    int   m_max_display_news;        /**<How often a news message is displayed
                                         before it is ignored. */

    /** Disable steering if skidding is stopped. This can help in making
     *  skidding more controllable (since otherwise when trying to steer while
     *  steering is reset to match the graphics it often results in the kart
     *  crashing). */
    bool m_disable_steer_while_unskid;

    /** If true the camera will stay behind the kart, potentially making it
     *  easier to see where the kart is going to after the skid. */
    bool m_camera_follow_skid;

    float m_ai_acceleration;         /**<Between 0 and 1, default being 1, can be
                                         used to give a handicap to AIs */

    std::vector<float>
        m_leader_intervals;        /**<Interval in follow the leader till
                                       last kart is reomved.               */
    float m_leader_time_per_kart;    /**< Additional time to each leader
                                          interval for each additional kart. */
    std::vector<int> m_switch_items; /**< How to switch items.               */
    /** The number of points a kart on position X has more than the
     *  next kart. From this the actual number of points for each
     *  position is computed. */
    std::vector<int> m_score_increase;

    /** Filename of the title music to play.*/
    MusicInformation *m_title_music;

    /** Filename of the music that is played when the track's music was not found.*/
    MusicInformation *m_default_music;

    /** Filename of the music to play when a player finishes in 1st place.*/
    MusicInformation *m_race_win_music;

    /** Filename of the music to play when a player finishes anywhere in the top half
     *  of the total karts but 1st place, rounded down.*/
    MusicInformation *m_race_neutral_music;

    /** Filename of the music to play when a player finishes in the bottom half of
      * the total karts.*/
    MusicInformation *m_race_lose_music;

    /** Filename of the grand prix win music to play.*/
    MusicInformation *m_gp_win_music;

    /** Filename of the grand prix lose music to play.*/
    MusicInformation *m_gp_lose_music;

    /** Filename of the feature unlock music to play.*/
    MusicInformation *m_unlock_music;

    /** Maximum number of transform events of a replay. */
    int m_replay_max_frames;

    /** Maximum time between consecutive saved tranform events.  */
    float m_replay_dt;

    /** If the speed difference with the last transform event
     *  is larger than this, a new event is generated. */
    float m_replay_delta_speed;

    /** A steering difference of more than that will trigger a new event to
     *  be generated. */
    float m_replay_delta_steering;

    /** The minimap size */
    float m_minimap_size;

    /* The size of icons for AIs and human players, respectively */
    float m_minimap_ai_icon;
    float m_minimap_player_icon;

    /** The field of view for 1, 2, 3, 4 player split screen. */
    float m_camera_fov[MAX_PLAYER_COUNT];

    float m_cutscene_fov;

    unsigned m_max_skinning_bones;

    unsigned m_tc_quality;

    /** Client and server port use random ports if enabled in user config. */
    uint16_t m_server_discovery_port;
    uint16_t m_client_port;
    uint16_t m_server_port;

    /* URLs for donating and reseting the password */
    std::string m_donate_url;
    std::string m_password_reset_url;
    std::string m_assets_download_url;

    /* SRV records for stun server lists created */
    std::string m_stun_ipv4;
    std::string m_stun_ipv6;

    /** Lists of TTF files used in STK. */
    std::vector<std::string> m_normal_ttf;
    std::vector<std::string> m_digit_ttf;
    std::string m_color_emoji_ttf;

    /** Configurable values used in SmoothNetworkBody class. */
    float m_snb_min_adjust_length, m_snb_max_adjust_length,
        m_snb_min_adjust_speed, m_snb_max_adjust_time,
        m_snb_adjust_length_threshold;

    /** URL for the server used for the API multiplayer. */
    std::string m_server_api;

    /** Version of the server API to use */
    uint32_t m_server_api_version = 0;

    /** URL for the server used for the addons management. */
    std::string m_server_addons;

    /** URL for the server used for hardware reporting statistics */
    std::string m_server_hardware_report;

    /** If true we allow all the server urls to be redirected by the news.xml. */
    bool m_allow_news_redirects = true;

    /** List of network capabilities to handle different servers with same
     *  version. */
    std::set<std::string> m_network_capabilities;

private:
    /** True if stk_config has been loaded. This is necessary if the
     *  --stk-config command line parameter has been specified to avoid
     *  that stk loads the default configuration after already having
     *  loaded a user specified config file. */
    bool  m_has_been_loaded;

    /** Default FPS rate for physics. */
    int m_physics_fps;

    std::string m_title_music_file;
    std::string m_default_music_file;
    std::string m_race_win_music_file;
    std::string m_race_neutral_music_file;
    std::string m_race_lose_music_file;
    std::string m_gp_win_music_file;
    std::string m_gp_lose_music_file;
    std::string m_unlock_music_file;

public:
    STKConfig();
    ~STKConfig();
    void init_defaults();
    void getAllData(const XMLNode * root);
    void load(const std::string &filename);
    const std::string &getMainMenuPicture(int n);
    const std::string &getBackgroundPicture(int n);
    void initMusicFiles();
    void  getAllScores(std::vector<int> *all_scores, int num_karts);
    // ------------------------------------------------------------------------
    /** Returns the default kart properties for each kart. */
    const KartProperties &
        getDefaultKartProperties() const { return *m_default_kart_properties; }

    // ------------------------------------------------------------------------
    /** Returns the kart properties for a certain type of kart.
     *  \throw out_of_range if there is no data for 'type'.
     *  \param type Type of kart (e.g. heavy, medium, ...).
     */
    const KartProperties& getKartProperties(const std::string &type)
    {
        return *m_kart_properties.at(type);
    }   // getKartProperties
    // ------------------------------------------------------------------------
    /** Converts a tick value (in physics time step size) into seconds. */
    float ticks2Time(int ticks) { return float(ticks)/m_physics_fps; }
    // ------------------------------------------------------------------------
    /** Converts a time value into ticks (of physics time steps). */
    int time2Ticks(float t) { return int(t * m_physics_fps);  }
    // ------------------------------------------------------------------------
    /** Returns the physics frame per seconds rate. */
    int getPhysicsFPS() const { return m_physics_fps; }
}
;   // STKConfig

extern STKConfig* stk_config;
#endif
