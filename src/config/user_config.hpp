//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
//  Modelled after Supertux's configfile.h
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

#ifndef HEADER_USER_CONFIG_HPP
#define HEADER_USER_CONFIG_HPP

/* The following config versions are currently used:
   0: the 0.2 release config file, without config-version number
      (so that defaults to 0)
   1: Removed singleWindowMenu, newKeyboardStyle, oldStatusDisplay,
      added config-version number
      Version 1 can read version 0 without any problems, so
      SUPPORTED_CONFIG_VERSION is 0.
   2: Changed to SDL keyboard bindings
   3: Added username (userid was used for ALL players)
   4: Added username per player
   5: Enabled jumping, which might cause a problem with old
      config files (which have an unused entry for jump defined
      --> if a kart control for (say) player 2 uses the same key as
      jump for player 1, this problem is not noticed in 0.3, but will
      cause an undefined game action now
   6: Added stick configurations.
*/
#include <string>
#include <map>
#include <vector>
#include <fstream>

#include <irrString.h>
using irr::core::stringc;
using irr::core::stringw;

#include "utils/constants.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/time.hpp"

class PlayerProfile;
class SavedGrandPrix;
class XMLNode;
class UTFWriter;

/**
 *  The base of a set of small utilities to enable quickly adding/removing
 *  stuff to/from config painlessly.
 */
class UserConfigParam
{
    friend class GroupUserConfigParam;
protected:
    std::string m_param_name;
    std::string m_comment;
public:
    virtual     ~UserConfigParam();
    virtual void write(std::ofstream & stream) const = 0;
    virtual void writeInner(std::ofstream & stream, int level = 0) const;
    virtual void findYourDataInAChildOf(const XMLNode* node) = 0;
    virtual void findYourDataInAnAttributeOf(const XMLNode* node) = 0;
    virtual irr::core::stringc toString() const = 0;
};   // UserConfigParam

// ============================================================================
class GroupUserConfigParam : public UserConfigParam
{
    std::vector<UserConfigParam*> m_attributes;
    std::vector<GroupUserConfigParam*> m_children;
public:
    GroupUserConfigParam(const char* name, const char* comment=NULL);
    GroupUserConfigParam(const char* param_name,
                       GroupUserConfigParam* group,
                       const char* comment = NULL);
    void write(std::ofstream& stream) const;
    void writeInner(std::ofstream& stream, int level = 0) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    void addChild(UserConfigParam* child);
    void addChild(GroupUserConfigParam* child);
    void clearChildren();

    irr::core::stringc toString() const;
};   // GroupUserConfigParam

// ============================================================================
template<typename T, typename U>
class ListUserConfigParam : public UserConfigParam
{
    std::vector<T> m_elements;

public:
    ListUserConfigParam(const char* param_name,
                         const char* comment = NULL);
    ListUserConfigParam(const char* param_name,
                         const char* comment,
                         int nb_elts,
                         ...);
    ListUserConfigParam(const char* param_name,
                         GroupUserConfigParam* group,
                         const char* comment = NULL);
    ListUserConfigParam(const char* param_name,
                         GroupUserConfigParam* group,
                         const char* comment,
                         int nb_elts,
                         ...);

    void write(std::ofstream& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    void addElement(T element);

    irr::core::stringc toString() const;

    operator std::vector<T>() const
            { return m_elements; }
    float& operator=(const std::vector<T>& v)
            { m_elements = std::vector<T>(v); return m_elements; }
    float& operator=(const ListUserConfigParam& v)
            { m_elements = std::vector<T>(v); return m_elements; }
};   // ListUserConfigParam
typedef ListUserConfigParam<std::string, const char*>    StringListUserConfigParam;

// ============================================================================
class IntUserConfigParam : public UserConfigParam
{
    int m_value;
    int m_default_value;

public:

    IntUserConfigParam(int default_value, const char* param_name,
                       const char* comment = NULL);
    IntUserConfigParam(int default_value, const char* param_name,
                       GroupUserConfigParam* group,
                       const char* comment = NULL);

    void write(std::ofstream& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    irr::core::stringc toString() const;
    void revertToDefaults()      { m_value = m_default_value;        }

    operator int() const         { return m_value;                   }
    int& operator++(int dummy)   { m_value++; return m_value;        }
    int& operator=(const int& v) { m_value = v; return m_value;      }
    int& operator=(const IntUserConfigParam& v)
                                 { m_value = (int)v; return m_value; }
};   // IntUserConfigParam

// ============================================================================
class TimeUserConfigParam : public UserConfigParam
{
    StkTime::TimeType m_value;
    StkTime::TimeType m_default_value;

public:

    TimeUserConfigParam(StkTime::TimeType default_value, const char* param_name,
                        const char* comment = NULL);
    TimeUserConfigParam(StkTime::TimeType default_value, const char* param_name,
                        GroupUserConfigParam* group, const char* comment=NULL);

    void write(std::ofstream& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    irr::core::stringc toString() const;
    void revertToDefaults()               { m_value = m_default_value;        }
    operator StkTime::TimeType() const       { return m_value;                   }
    StkTime::TimeType& operator=(const StkTime::TimeType& v)
                                          { m_value = v; return m_value;      }
    StkTime::TimeType& operator=(const TimeUserConfigParam& v)
                                          { m_value = (int)v; return m_value; }
};   // TimeUserConfigParam

// ============================================================================
class StringUserConfigParam : public UserConfigParam
{
    std::string m_value;
    std::string m_default_value;

public:

    StringUserConfigParam(const char* default_value, const char* param_name,
                          const char* comment = NULL);
    StringUserConfigParam(const char* default_value, const char* param_name,
                          GroupUserConfigParam* group,
                          const char* comment = NULL);

    void write(std::ofstream& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    void revertToDefaults() { m_value = m_default_value; }

    std::string getDefaultValue() const { return m_default_value; }

    irr::core::stringc toString() const { return m_value.c_str(); }

    operator std::string() const  { return m_value; }
    std::string& operator=(const std::string& v)
                                  { m_value = v; return m_value; }
    std::string& operator=(const StringUserConfigParam& v)
                                  { m_value = (std::string)v; return m_value; }

    const char* c_str() const { return m_value.c_str(); }
};   // StringUserConfigParam

// ============================================================================
class BoolUserConfigParam : public UserConfigParam
{
    bool m_value;
    bool m_default_value;

public:
    BoolUserConfigParam(bool default_value, const char* param_name,
                        const char* comment = NULL);
    BoolUserConfigParam(bool default_value, const char* param_name,
                        GroupUserConfigParam* group,
                        const char* comment = NULL);
    void write(std::ofstream& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    irr::core::stringc toString() const;
    void revertToDefaults() { m_value = m_default_value; }

    operator bool() const { return m_value; }
    bool& operator=(const bool& v) { m_value = v; return m_value; }
    bool& operator=(const BoolUserConfigParam& v)
                              { m_value = (bool)v; return m_value; }
};   // BoolUserConfigParam

// ============================================================================
class FloatUserConfigParam : public UserConfigParam
{
    float m_value;
    float m_default_value;

public:
    FloatUserConfigParam(float default_value, const char* param_name,
                         const char* comment = NULL);
    FloatUserConfigParam(float default_value, const char* param_name,
                         GroupUserConfigParam* group,
                         const char* comment = NULL);

    void write(std::ofstream& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    irr::core::stringc toString() const;
    void revertToDefaults() { m_value = m_default_value; }

    operator float() const { return m_value; }
    float& operator=(const float& v) { m_value = v; return m_value; }
    float& operator=(const FloatUserConfigParam& v)
                              { m_value = (float)v; return m_value; }
};   // FloatUserConfigParam

// ============================================================================
enum AnimType {ANIMS_NONE         = 0,
               ANIMS_PLAYERS_ONLY = 1,
               ANIMS_ALL          = 2 };

enum GeometryLevel
{
    /** Display everything */
    GEOLEVEL_0    = 0,
    /** a few details are displayed */
    GEOLEVEL_1    = 1,
    /** Lowest level, no details are displayed. */
    GEOLEVEL_2    = 2
};

/** Using X-macros for setting-possible values is not very pretty, but it's a
 *  no-maintenance case :
 *  when you want to add a new parameter, just add one signle line below and
 *  everything else automagically works (including default value, saving to
 *  file, loading from file)
 */

#ifndef PARAM_PREFIX
#define PARAM_PREFIX extern
#endif

#ifndef PARAM_DEFAULT
#define PARAM_DEFAULT(X)
#endif

// ============================================================================
/** \brief Contains all parameters that are stored in the user's config file
 *  \ingroup config
 */
namespace UserConfigParams
{

    // ---- Audio
    PARAM_PREFIX GroupUserConfigParam        m_audio_group
            PARAM_DEFAULT( GroupUserConfigParam("Audio", "Audio Settings") );

    PARAM_PREFIX BoolUserConfigParam         m_sfx
            PARAM_DEFAULT( BoolUserConfigParam(true, "sfx_on", &m_audio_group,
            "Whether sound effects are enabled or not (true or false)") );
    PARAM_PREFIX BoolUserConfigParam         m_music
            PARAM_DEFAULT(  BoolUserConfigParam(true, "music_on",
            &m_audio_group,
            "Whether musics are enabled or not (true or false)") );
    PARAM_PREFIX FloatUserConfigParam       m_sfx_volume
            PARAM_DEFAULT(  FloatUserConfigParam(1.0, "sfx_volume",
            &m_audio_group, "Volume for sound effects, see openal AL_GAIN "
                            "for interpretation") );
    PARAM_PREFIX FloatUserConfigParam       m_music_volume
            PARAM_DEFAULT(  FloatUserConfigParam(0.7f, "music_volume",
            &m_audio_group, "Music volume from 0.0 to 1.0") );

    // ---- Race setup
    PARAM_PREFIX GroupUserConfigParam        m_race_setup_group
        PARAM_DEFAULT( GroupUserConfigParam("RaceSetup",
                                            "Race Setup Settings") );

    PARAM_PREFIX IntUserConfigParam          m_num_karts
            PARAM_DEFAULT(  IntUserConfigParam(4, "numkarts",
                            &m_race_setup_group,
                            "Default number of karts. -1 means use all") );
    PARAM_PREFIX IntUserConfigParam          m_num_laps
            PARAM_DEFAULT(  IntUserConfigParam(4, "numlaps",
            &m_race_setup_group, "Default number of laps.") );
    PARAM_PREFIX IntUserConfigParam          m_num_goals
            PARAM_DEFAULT(  IntUserConfigParam(3, "numgoals",
            &m_race_setup_group, "Default number of goals in soccer mode.") );
    PARAM_PREFIX IntUserConfigParam          m_soccer_default_team
            PARAM_DEFAULT(  IntUserConfigParam(0, "soccer-default-team",
            &m_race_setup_group, "Default team in soccer mode for single player.") );
    PARAM_PREFIX IntUserConfigParam          m_soccer_time_limit
            PARAM_DEFAULT(  IntUserConfigParam(3, "soccer-time-limit",
            &m_race_setup_group, "Time limit in soccer mode.") );
    PARAM_PREFIX BoolUserConfigParam         m_soccer_use_time_limit
            PARAM_DEFAULT(  BoolUserConfigParam(false, "soccer-use-time-limit",
            &m_race_setup_group, "Enable time limit in soccer mode.") );
    PARAM_PREFIX IntUserConfigParam          m_difficulty
            PARAM_DEFAULT(  IntUserConfigParam(0, "difficulty",
                            &m_race_setup_group,
                        "Default race difficulty. 0=easy, 1=medium, 2=hard") );
    PARAM_PREFIX IntUserConfigParam          m_game_mode
            PARAM_DEFAULT(  IntUserConfigParam(0, "game_mode",
                            &m_race_setup_group,
                            "Game mode. 0=standard, 1=time trial, 2=follow "
                            "the leader, 3=3 strikes") );
    PARAM_PREFIX StringUserConfigParam m_default_kart
            PARAM_DEFAULT( StringUserConfigParam("tux", "kart",
                           "Kart to select by default (the last used kart)") );
    PARAM_PREFIX StringUserConfigParam m_last_used_kart_group
            PARAM_DEFAULT( StringUserConfigParam("all", "last_kart_group",
                                                 "Last selected kart group") );

    // ---- Wiimote data
    PARAM_PREFIX GroupUserConfigParam        m_wiimote_group
        PARAM_DEFAULT( GroupUserConfigParam("WiiMote",
                                            "Settings for the wiimote") );
    PARAM_PREFIX FloatUserConfigParam         m_wiimote_raw_max
            PARAM_DEFAULT( FloatUserConfigParam(20.0f, "wiimote-raw-max",
            &m_wiimote_group,
            "At what raw input value maximum steering is reached (between 1 and 25).") );

    PARAM_PREFIX FloatUserConfigParam         m_wiimote_weight_linear
            PARAM_DEFAULT( FloatUserConfigParam(1.0f, "wiimote-weight-linear",
            &m_wiimote_group,
            "A weight applied to the linear component of mapping wiimote angle to steering angle"));

    PARAM_PREFIX FloatUserConfigParam         m_wiimote_weight_square
            PARAM_DEFAULT( FloatUserConfigParam(0.0f, "wiimote-weight-square",
            &m_wiimote_group,
            "A weight applied to the square component of mapping wiimote angle to steering angle"));

    PARAM_PREFIX FloatUserConfigParam         m_wiimote_weight_asin
            PARAM_DEFAULT( FloatUserConfigParam(0.0f, "wiimote-weight-asin",
            &m_wiimote_group,
            "A weight applied to the asin component of mapping wiimote angle to steering angle"));

    PARAM_PREFIX FloatUserConfigParam         m_wiimote_weight_sin
            PARAM_DEFAULT( FloatUserConfigParam(0.0f, "wiimote-weight-sin",
            &m_wiimote_group,
            "A weight applied to the sin component of mapping wiimote angle to steering angle"));

    // ---- Multitouch device
    PARAM_PREFIX GroupUserConfigParam        m_multitouch_group
        PARAM_DEFAULT( GroupUserConfigParam("Multitouch",
                                            "Settings for the multitouch device") );

    PARAM_PREFIX BoolUserConfigParam         m_multitouch_enabled
            PARAM_DEFAULT( BoolUserConfigParam(false, "multitouch_enabled",
            &m_multitouch_group,
            "Enable multitouch support.") );
            
    PARAM_PREFIX IntUserConfigParam         m_multitouch_mode
            PARAM_DEFAULT( IntUserConfigParam(1, "multitouch_mode",
            &m_multitouch_group,
            "Steering mode: 0 = off, 1 = buttons"));

    PARAM_PREFIX BoolUserConfigParam         m_multitouch_inverted
            PARAM_DEFAULT( BoolUserConfigParam(false, "multitouch_inverted",
            &m_multitouch_group,
            "Draw steering wheel on right side.") );

    PARAM_PREFIX IntUserConfigParam         m_multitouch_accelerometer
            PARAM_DEFAULT( IntUserConfigParam(0, "multitouch_accelerometer",
            &m_multitouch_group,
            "Accelerometer mode: 0 = off, 1 = tablet, 2 = phone"));

    PARAM_PREFIX FloatUserConfigParam         m_multitouch_deadzone_center
            PARAM_DEFAULT( FloatUserConfigParam(0.1f, "multitouch_deadzone_center",
            &m_multitouch_group,
            "A parameter in range [0, 0.5] that determines the zone that is "
            "considered as centered in steering button."));

    PARAM_PREFIX FloatUserConfigParam         m_multitouch_deadzone_edge
            PARAM_DEFAULT( FloatUserConfigParam(0.1f, "multitouch_deadzone_edge",
            &m_multitouch_group,
            "A parameter in range [0, 0.5] that determines the zone that is "
            "considered as max value in steering button."));
            
    PARAM_PREFIX FloatUserConfigParam         m_multitouch_tilt_factor
            PARAM_DEFAULT( FloatUserConfigParam(4.0f, "multitouch_tilt_factor",
            &m_multitouch_group,
            "A parameter that determines general accelerometer sensitivity."));

    PARAM_PREFIX FloatUserConfigParam         m_multitouch_scale
            PARAM_DEFAULT( FloatUserConfigParam(1.1f, "multitouch_scale",
            &m_multitouch_group,
            "A parameter in range [0.5, 1.5] that determines the scale of the "
            "multitouch interface."));

    PARAM_PREFIX BoolUserConfigParam         m_screen_keyboard
            PARAM_DEFAULT( BoolUserConfigParam(false, "screen_keyboard",
            &m_multitouch_group,
            "Enable screen keyboard.") );

    // ---- GP start order
    PARAM_PREFIX GroupUserConfigParam        m_gp_start_order
            PARAM_DEFAULT( GroupUserConfigParam("GpStartOrder",
                                                "Order karts start in GP") );
    PARAM_PREFIX BoolUserConfigParam         m_gp_most_points_first
            PARAM_DEFAULT( BoolUserConfigParam(true, "most_points_first",
            &m_gp_start_order,
            "Starting order from most to least points (true) or other "
            "way around (false)") );
    PARAM_PREFIX BoolUserConfigParam         m_gp_player_last
            PARAM_DEFAULT( BoolUserConfigParam(false, "player_last",
            &m_gp_start_order,
            "Always put the player at the back or not (Bully mode).") );
    PARAM_PREFIX StringUserConfigParam       m_additional_gp_directory
        PARAM_DEFAULT( StringUserConfigParam("", "additional_gp_directory",
                                            "Directory with additional GP's."));

    // ---- Video
    PARAM_PREFIX GroupUserConfigParam        m_video_group
        PARAM_DEFAULT( GroupUserConfigParam("Video", "Video Settings") );

    PARAM_PREFIX IntUserConfigParam         m_width
            PARAM_DEFAULT(  IntUserConfigParam(1024, "width", &m_video_group,
                                            "Screen/window width in pixels") );
    PARAM_PREFIX IntUserConfigParam         m_height
            PARAM_DEFAULT(  IntUserConfigParam(768, "height", &m_video_group,
                                           "Screen/window height in pixels") );
    PARAM_PREFIX BoolUserConfigParam        m_fullscreen
            PARAM_DEFAULT(  BoolUserConfigParam(false, "fullscreen",
                                                &m_video_group) );
    PARAM_PREFIX IntUserConfigParam         m_prev_width
            PARAM_DEFAULT(  IntUserConfigParam(1024, "prev_width",
                            &m_video_group, "Previous screen/window width") );
    PARAM_PREFIX IntUserConfigParam         m_prev_height
            PARAM_DEFAULT(  IntUserConfigParam(768, "prev_height",
                            &m_video_group,"Previous screen/window height") );
    PARAM_PREFIX BoolUserConfigParam        m_prev_fullscreen
            PARAM_DEFAULT(  BoolUserConfigParam(false, "prev_fullscreen",
                            &m_video_group) );


    PARAM_PREFIX BoolUserConfigParam        m_remember_window_location
            PARAM_DEFAULT(  BoolUserConfigParam(false, "remember_window_location",
                            &m_video_group) );
    PARAM_PREFIX IntUserConfigParam         m_window_x
            PARAM_DEFAULT(  IntUserConfigParam(-1, "window_x",
                            &m_video_group,"If remember_window_location is true") );
    PARAM_PREFIX IntUserConfigParam         m_window_y
            PARAM_DEFAULT(  IntUserConfigParam(-1, "window_y",
                            &m_video_group,"If remember_window_location is true") );

    PARAM_PREFIX BoolUserConfigParam        m_display_fps
            PARAM_DEFAULT(  BoolUserConfigParam(false, "show_fps",
                            &m_video_group, "Display frame per seconds") );
    PARAM_PREFIX IntUserConfigParam         m_max_fps
            PARAM_DEFAULT(  IntUserConfigParam(120, "max_fps",
                       &m_video_group, "Maximum fps, should be at least 60") );
    PARAM_PREFIX BoolUserConfigParam        m_force_legacy_device
        PARAM_DEFAULT(BoolUserConfigParam(false, "force_legacy_device",
        &m_video_group, "Force OpenGL 2 context, even if OpenGL 3 is available."));

    PARAM_PREFIX BoolUserConfigParam        m_texture_compression
        PARAM_DEFAULT(BoolUserConfigParam(true, "enable_texture_compression",
        &m_video_group, "Enable Texture Compression"));
    /** This is a bit flag: bit 0: enabled (1) or disabled(0).
     *  Bit 1: setting done by default(0), or by user choice (2). This allows
     *  to e.g. disable h.d. textures on hd3000 as default, but still allow the
     *  user to enable it. */
    PARAM_PREFIX IntUserConfigParam        m_high_definition_textures
        PARAM_DEFAULT(IntUserConfigParam(1, "enable_high_definition_textures",
        &m_video_group, "Enable high definition textures. Bit flag: "
                        "bit 0 = enabled/disabled; bit 1 = set by user/set as default"));
    PARAM_PREFIX BoolUserConfigParam        m_glow
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_glow",
        &m_video_group, "Enable Glow"));
    PARAM_PREFIX BoolUserConfigParam        m_bloom
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_bloom",
        &m_video_group, "Enable Bloom"));
    PARAM_PREFIX BoolUserConfigParam        m_light_shaft
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_light_shaft",
        &m_video_group, "Enable Light Shafts"));
    PARAM_PREFIX BoolUserConfigParam        m_dynamic_lights
        PARAM_DEFAULT(BoolUserConfigParam(true, "enable_dynamic_lights",
        &m_video_group, "Enable Dynamic Lights"));
    PARAM_PREFIX BoolUserConfigParam        m_dof
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_dof",
        &m_video_group, "Enable Depth of Field"));
    PARAM_PREFIX BoolUserConfigParam        m_gi
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_gi",
        &m_video_group, "Enable Global Illumination"));
    PARAM_PREFIX BoolUserConfigParam        m_azdo
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_azdo",
        &m_video_group, "Enable 'Approaching Zero Driver Overhead' mode (very experimental !)"));
    PARAM_PREFIX BoolUserConfigParam        m_sdsm
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_sdsm",
        &m_video_group, "Enable Sampled Distribued Shadow Map (buggy atm)"));
    PARAM_PREFIX BoolUserConfigParam        m_esm
        PARAM_DEFAULT(BoolUserConfigParam(false, "enable_esm",
        &m_video_group, "Enable Exponential Shadow Map (better but slower)"));
    PARAM_PREFIX BoolUserConfigParam        m_old_driver_popup
        PARAM_DEFAULT(BoolUserConfigParam(true, "old_driver_popup",
        &m_video_group, "Determines if popup message about too old drivers should be displayed."));
    PARAM_PREFIX FloatUserConfigParam       m_scale_rtts_factor
        PARAM_DEFAULT(FloatUserConfigParam(1.0f, "scale_rtts_factor",
        &m_video_group, "Allows to increase performance by setting lower RTTs "
                        "resolution. Value should be smaller or equal to 1.0"));
    PARAM_PREFIX IntUserConfigParam         m_max_texture_size
        PARAM_DEFAULT(IntUserConfigParam(512, "max_texture_size",
        &m_video_group, "Max texture size when high definition textures are "
                        "disabled"));

    PARAM_PREFIX BoolUserConfigParam        m_hq_mipmap
        PARAM_DEFAULT(BoolUserConfigParam(false, "hq_mipmap",
        &m_video_group, "Generate mipmap for textures using "
                        "high quality method with SSE"));
                        
    // ---- Recording
    PARAM_PREFIX GroupUserConfigParam        m_recording_group
        PARAM_DEFAULT(GroupUserConfigParam("Recording",
                            "Recording Settings"));

    PARAM_PREFIX BoolUserConfigParam        m_limit_game_fps
        PARAM_DEFAULT(BoolUserConfigParam(true, "limit_game_fps",
        &m_recording_group, "Limit game framerate not beyond the fps of"
                            " recording video."));

    PARAM_PREFIX IntUserConfigParam         m_video_format
        PARAM_DEFAULT(IntUserConfigParam(0, "video_format",
        &m_recording_group, "Specify the video for record, which is the enum"
                            " of VideoFormat in libopenglrecorder. It will"
                            " auto fallback to MJPEG if libopenglrecorder was"
                            " not compiled with such video encoder."));

    PARAM_PREFIX IntUserConfigParam         m_audio_bitrate
        PARAM_DEFAULT(IntUserConfigParam(112000, "audio_bitrate",
        &m_recording_group, "Specify the bitrate for audio"));

    PARAM_PREFIX IntUserConfigParam         m_video_bitrate
        PARAM_DEFAULT(IntUserConfigParam(20000, "video_bitrate",
        &m_recording_group, "Specify the bitrate for video"));

    PARAM_PREFIX IntUserConfigParam         m_recorder_jpg_quality
        PARAM_DEFAULT(IntUserConfigParam(90, "recorder_jpg_quality",
        &m_recording_group, "Specify the jpg compression level of recorder"));

    PARAM_PREFIX IntUserConfigParam         m_record_fps
        PARAM_DEFAULT(IntUserConfigParam(30, "record_fps",
        &m_recording_group, "Specify the fps of recording video"));

    // ---- Debug - not saved to config file
    /** If gamepad debugging is enabled. */
    PARAM_PREFIX bool m_unit_testing PARAM_DEFAULT(false);

    /** If gamepad debugging is enabled. */
    PARAM_PREFIX bool m_gamepad_debug PARAM_DEFAULT( false );

    /** If gamepad debugging is enabled. */
    PARAM_PREFIX bool m_keyboard_debug PARAM_DEFAULT(false);

    /** Wiimote debugging. */
    PARAM_PREFIX bool m_wiimote_debug PARAM_DEFAULT( false );

    /** Debug gamepads  by visualising their values. */
    PARAM_PREFIX bool m_gamepad_visualisation PARAM_DEFAULT( false );

    /** If material debugging (printing terrain specific slowdown)
     *  is enabled. */
    PARAM_PREFIX bool m_material_debug PARAM_DEFAULT( false );

    /** If track debugging is enabled. */
    PARAM_PREFIX int m_track_debug PARAM_DEFAULT( false );

    /** If random number of items is used in an arena. */
    PARAM_PREFIX bool m_random_arena_item PARAM_DEFAULT( false );

    /** True if check structures should be debugged. */
    PARAM_PREFIX bool m_check_debug PARAM_DEFAULT( false );

    /** True if physics debugging should be enabled. */
    PARAM_PREFIX bool m_physics_debug PARAM_DEFAULT( false );

    /** True if fps should be printed each frame. */
    PARAM_PREFIX bool m_fps_debug PARAM_DEFAULT(false);

    /** True if arena (battle/soccer) ai profiling. */
    PARAM_PREFIX bool m_arena_ai_stats PARAM_DEFAULT(false);

    /** True if slipstream debugging is activated. */
    PARAM_PREFIX bool m_slipstream_debug  PARAM_DEFAULT( false );

    /** True if follow-the-leader debug information should be printed. */
    PARAM_PREFIX bool m_ftl_debug    PARAM_DEFAULT( false );

    /** True if currently developed tutorial debugging is enabled. */
    PARAM_PREFIX bool m_tutorial_debug    PARAM_DEFAULT( false );

    /** Verbosity level for debug messages. Note that error and
     *  important warnings must always be printed. */
    PARAM_PREFIX int  m_verbosity         PARAM_DEFAULT( 0 );

    PARAM_PREFIX bool m_no_start_screen   PARAM_DEFAULT( false );

    PARAM_PREFIX bool m_race_now          PARAM_DEFAULT( false );

    PARAM_PREFIX bool m_enforce_current_player PARAM_DEFAULT( false );

    /** True to test funky ambient/diffuse/specularity in RGB &
     *  all anisotropic */
    PARAM_PREFIX bool m_rendering_debug   PARAM_DEFAULT( false );

    /** True if graphical profiler should be displayed */
    PARAM_PREFIX bool m_profiler_enabled  PARAM_DEFAULT( false );

    /** How many seconds worth of data the circular profile buffer
     *  can store. */
    PARAM_PREFIX float m_profiler_buffer_duration PARAM_DEFAULT(20.0f);

    // not saved to file

    // ---- Networking

    PARAM_PREFIX IntUserConfigParam         m_server_max_players
            PARAM_DEFAULT(  IntUserConfigParam(16, "server_max_players",
                                       "Maximum number of players on the server.") );

    PARAM_PREFIX StringListUserConfigParam         m_stun_servers
            PARAM_DEFAULT(  StringListUserConfigParam("Stun_servers", "The stun servers"
                            " that will be used to know the public address.",
                            24,
                            "provserver.televolution.net",
                            "sip1.lakedestiny.cordiaip.com",
                            "stun1.voiceeclipse.net",
                            "stun01.sipphone.com",
                            "stun.callwithus.com",
                            "stun.counterpath.net",
                            "stun.endigovoip.com",
                            "stun.ekiga.net",
                            "stun.ideasip.com",
                            "stun.internetcalls.com",
                            "stun.ipns.com",
                            "stun.noc.ams-ix.net",
                            "stun.phonepower.com",
                            "stun.phoneserve.com",
                            "stun.rnktel.com",
                            "stun.softjoys.com",
                            "stunserver.org",
                            "stun.sipgate.net",
                            "stun.stunprotocol.org",
                            "stun.voip.aebc.com",
                            "stun.voipbuster.com",
                            "stun.voxalot.com",
                            "stun.voxgratia.org",
                            "stun.xten.com") );

    PARAM_PREFIX BoolUserConfigParam m_log_packets
            PARAM_DEFAULT( BoolUserConfigParam(false, "log-network-packets",
                                                 "If all network packets should be logged") );

    // ---- Graphic Quality
    PARAM_PREFIX GroupUserConfigParam        m_graphics_quality
            PARAM_DEFAULT( GroupUserConfigParam("GFX",
                                                "Graphics Quality Settings") );

    // On OSX 10.4 and before there may be driver issues with FBOs, so to be
    // safe disable them by default
#ifdef __APPLE__
    #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    #define FBO_DEFAULT false
    #else
    #define FBO_DEFAULT true
    #endif
#else
#define FBO_DEFAULT true
#endif

    PARAM_PREFIX IntUserConfigParam        m_graphical_effects
            PARAM_DEFAULT(  IntUserConfigParam(2, "animate_graphics",
                            &m_graphics_quality, "Scenery animations: 0 disabled, 1 only important, 2 enabled") );

    // This saves the actual user preference.
    PARAM_PREFIX IntUserConfigParam        m_xmas_mode
            PARAM_DEFAULT(  IntUserConfigParam(0, "christmas-mode",
                            &m_graphics_quality, "Christmas hats: 0 use current date, 1 always on, 2 always off") );

    // This saves the actual user preference.
    PARAM_PREFIX IntUserConfigParam        m_easter_ear_mode
        PARAM_DEFAULT(IntUserConfigParam(0, "easter-ear-mode",
        &m_graphics_quality, "Easter Bunny Ears: 0 use current date, 1 always on, 2 always off"));

    PARAM_PREFIX BoolUserConfigParam        m_weather_effects
            PARAM_DEFAULT(  BoolUserConfigParam(true, "weather_gfx",
                                     &m_graphics_quality, "Weather effects") );
    PARAM_PREFIX IntUserConfigParam        m_show_steering_animations
            PARAM_DEFAULT(  IntUserConfigParam(ANIMS_PLAYERS_ONLY,
                            "steering_animations", &m_graphics_quality,
                "Whether to display kart animations (0=disabled for all; "
                "1=enabled for humans, disabled for AIs; 2=enabled for all") );

    PARAM_PREFIX IntUserConfigParam        m_geometry_level
            PARAM_DEFAULT(  IntUserConfigParam(GEOLEVEL_0,
                            "geometry_level", &m_graphics_quality,
                "Geometry quality 0=everything is displayed; "
                "1=a few details are displayed; 2=lowest level, no details") );

    PARAM_PREFIX IntUserConfigParam         m_anisotropic
            PARAM_DEFAULT( IntUserConfigParam(4, "anisotropic",
                           &m_graphics_quality,
                           "Quality of anisotropic filtering (usual values include 2-4-8-16; 0 to disable)") );
    PARAM_PREFIX BoolUserConfigParam         m_trilinear
            PARAM_DEFAULT( BoolUserConfigParam(true, "trilinear",
                           &m_graphics_quality,
                           "Whether trilinear filtering is allowed to be "
                           "used (true or false)") );
    /*
    PARAM_PREFIX IntUserConfigParam          m_antialiasing
            PARAM_DEFAULT( IntUserConfigParam(0,
                           "antialiasing", &m_graphics_quality,
                           "Whether antialiasing is enabled (0 = disabled, 1 = 2x, 2 = 4x, 3 = 8x") );
    */
    PARAM_PREFIX BoolUserConfigParam         m_vsync
            PARAM_DEFAULT( BoolUserConfigParam(false, "vsync",
                           &m_graphics_quality,
                           "Whether vertical sync is enabled") );
    PARAM_PREFIX BoolUserConfigParam         m_motionblur
            PARAM_DEFAULT( BoolUserConfigParam(false,
                           "motionblur_enabled", &m_graphics_quality,
                           "Whether motion blur should be enabled") );
    PARAM_PREFIX BoolUserConfigParam         m_mlaa
            PARAM_DEFAULT( BoolUserConfigParam(false,
                           "mlaa", &m_graphics_quality,
                           "Whether MLAA anti-aliasing should be enabled") );
    PARAM_PREFIX BoolUserConfigParam          m_ssao
            PARAM_DEFAULT(BoolUserConfigParam(false,
                           "ssao", &m_graphics_quality,
                           "Enable Screen Space Ambient Occlusion") );
    PARAM_PREFIX IntUserConfigParam          m_shadows_resolution
            PARAM_DEFAULT( IntUserConfigParam(0,
                           "shadows_resolution", &m_graphics_quality,
                           "Shadow resolution (0 = disabled") );
    PARAM_PREFIX BoolUserConfigParam          m_degraded_IBL
        PARAM_DEFAULT(BoolUserConfigParam(true,
        "Degraded_IBL", &m_graphics_quality,
        "Disable specular IBL"));

    // ---- Misc
    PARAM_PREFIX BoolUserConfigParam        m_cache_overworld
            PARAM_DEFAULT(  BoolUserConfigParam(true, "cache-overworld") );

    // TODO : is this used with new code? does it still work?
    PARAM_PREFIX BoolUserConfigParam        m_crashed
            PARAM_DEFAULT(  BoolUserConfigParam(false, "crashed") );

#if defined(WIN32) && !defined(__CYGWIN__)
    // No console on windows
#  define CONSOLE_DEFAULT false
#else
#  define CONSOLE_DEFAULT true
#endif
    // No console on windows
    PARAM_PREFIX BoolUserConfigParam        m_log_errors_to_console
            PARAM_DEFAULT(  BoolUserConfigParam(
            CONSOLE_DEFAULT, "log_errors", "Enable logging to console.") );

    // ---- Camera
    PARAM_PREFIX GroupUserConfigParam        m_camera
            PARAM_DEFAULT( GroupUserConfigParam("camera",
                                                "(Debug) camera settings.") );

    PARAM_PREFIX IntUserConfigParam         m_reverse_look_threshold
            PARAM_DEFAULT(  IntUserConfigParam(0, "reverse_look_threshold",
            &m_camera,
            "If the kart is driving backwards faster than this value,\n"
            "switch automatically to reverse camera (set to 0 to disable).") );

    PARAM_PREFIX FloatUserConfigParam       m_fpscam_direction_speed
            PARAM_DEFAULT(  FloatUserConfigParam(0.003f, "fpscam_rotation_speed",
            &m_camera,
            "How fast the first person camera's direction speed changes when\n"
            "moving the mouse (means acceleration).") );

    PARAM_PREFIX FloatUserConfigParam       m_fpscam_smooth_direction_max_speed
            PARAM_DEFAULT(  FloatUserConfigParam(0.04f, "fpscam_smooth_rotation_max_speed",
            &m_camera,
            "How fast the first person camera's direction can change.") );

    PARAM_PREFIX FloatUserConfigParam       m_fpscam_angular_velocity
            PARAM_DEFAULT(  FloatUserConfigParam(0.02f, "fpscam_angular_velocity",
            &m_camera,
            "How fast the first person camera's rotation speed changes.") );

    PARAM_PREFIX FloatUserConfigParam       m_fpscam_max_angular_velocity
            PARAM_DEFAULT(  FloatUserConfigParam(1.0f, "fpscam_max_angular_velocity",
            &m_camera,
            "How fast the first person camera can rotate.") );

    PARAM_PREFIX StringUserConfigParam      m_item_style
            PARAM_DEFAULT(  StringUserConfigParam("items", "item_style",
                            "Name of the .items file to use.") );

    PARAM_PREFIX StringUserConfigParam      m_last_track
            PARAM_DEFAULT(  StringUserConfigParam("olivermath", "last_track",
                            "Name of the last track used.") );
    PARAM_PREFIX StringUserConfigParam m_last_used_track_group
            PARAM_DEFAULT( StringUserConfigParam("all", "last_track_group",
                           "Last selected track group") );

    PARAM_PREFIX StringUserConfigParam      m_skin_file
            PARAM_DEFAULT(  StringUserConfigParam("Peach.stkskin", "skin_file",
                                                  "Name of the skin to use") );

    // ---- Handicap
    PARAM_PREFIX GroupUserConfigParam       m_handicap
            PARAM_DEFAULT( GroupUserConfigParam("Handicap",
                                          "Everything related to handicaps.") );

    PARAM_PREFIX BoolUserConfigParam        m_per_player_difficulty
            PARAM_DEFAULT(  BoolUserConfigParam(false, "per_player_difficulty",
                            &m_handicap,
                            "If handicapped users can be selected") );

    // ---- Internet related

    PARAM_PREFIX IntUserConfigParam        m_internet_status
            PARAM_DEFAULT(  IntUserConfigParam(0, "enable_internet",
                                               "Status of internet: 0 user "
                                               "wasn't asked, 1: allowed, 2: "
                                               "not allowed") );

    PARAM_PREFIX GroupUserConfigParam       m_hw_report_group
            PARAM_DEFAULT( GroupUserConfigParam("HWReport",
                                          "Everything related to hardware configuration.") );

    PARAM_PREFIX IntUserConfigParam        m_last_hw_report_version
            PARAM_DEFAULT(  IntUserConfigParam(0, "report-version", &m_hw_report_group,
                                                  "Version of hardware report "
                                                  "that was reported last") );
    PARAM_PREFIX IntUserConfigParam        m_random_identifier
            PARAM_DEFAULT(  IntUserConfigParam(0, "random-identifier", &m_hw_report_group,
                                                  "A random number to avoid duplicated reports.") );

    PARAM_PREFIX StringUserConfigParam      m_server_hw_report
            PARAM_DEFAULT( StringUserConfigParam(   "http://addons.supertuxkart.net:8080",
                                                     "hw-report-server",
                                                     &m_hw_report_group,
                                                    "The server used for reporting statistics to."));

    PARAM_PREFIX BoolUserConfigParam      m_hw_report_enable
            PARAM_DEFAULT( BoolUserConfigParam(   true,
                                                     "hw-report-enabled",
                                                     &m_hw_report_group,
                                                    "If HW reports are enabled."));

    // ---- User management

    PARAM_PREFIX BoolUserConfigParam        m_always_show_login_screen
            PARAM_DEFAULT(  BoolUserConfigParam(false, "always_show_login_screen",
          "Always show the login screen even if last player's session was saved."));


    // ---- Online gameplay related
    PARAM_PREFIX GroupUserConfigParam       m_online_group
            PARAM_DEFAULT( GroupUserConfigParam("OnlineServer",
                                          "Everything related to online play.") );

    PARAM_PREFIX StringUserConfigParam      m_server_multiplayer
            PARAM_DEFAULT( StringUserConfigParam(   "https://addons.supertuxkart.net/api/",
                                                     "server_multiplayer",
                                                     &m_online_group,
                                                    "The server used for online multiplayer."));

    PARAM_PREFIX IntUserConfigParam        m_server_version
            PARAM_DEFAULT( IntUserConfigParam(   2,
                                                 "server-version",
                                                 &m_online_group,
                                                    "Version of the server API to use."));


    // ---- Addon server related entries
    PARAM_PREFIX GroupUserConfigParam       m_addon_group
            PARAM_DEFAULT( GroupUserConfigParam("AddonServer",
                                          "Addon and news related settings") );

    PARAM_PREFIX StringUserConfigParam      m_server_addons
            PARAM_DEFAULT( StringUserConfigParam("http://addons.supertuxkart.net/dl/xml",
                                                 "server_addons",
                                                 &m_addon_group,
                                                "The server used for addon."));

    PARAM_PREFIX TimeUserConfigParam        m_news_last_updated
            PARAM_DEFAULT(  TimeUserConfigParam(0, "news_last_updated",
                                              &m_addon_group,
                                              "Time news was updated last.") );

    PARAM_PREFIX IntUserConfigParam         m_news_frequency
            PARAM_DEFAULT(  IntUserConfigParam(0, "news_frequency",
                                               &m_addon_group,
                                        "How often news should be updated.") );

    PARAM_PREFIX StringUserConfigParam      m_display_count
            PARAM_DEFAULT(  StringUserConfigParam("", "news_display_count",
                                               &m_addon_group,
                                               "How often all news messages "
                                               "have been displayed") );

    PARAM_PREFIX IntUserConfigParam         m_last_important_message_id
            PARAM_DEFAULT(  IntUserConfigParam(-1, "last_important_message_id",
                                               &m_addon_group,
                                               "Don't show important message "
                                               "with this or a lower id again") );

    PARAM_PREFIX TimeUserConfigParam        m_addons_last_updated
            PARAM_DEFAULT(  TimeUserConfigParam(0, "addon_last_updated",
                                                &m_addon_group,
                                        "Time addon-list was updated last.") );

    PARAM_PREFIX StringUserConfigParam      m_language
            PARAM_DEFAULT( StringUserConfigParam("system", "language",
                        "Which language to use (language code or 'system')") );

    PARAM_PREFIX BoolUserConfigParam        m_artist_debug_mode
            PARAM_DEFAULT( BoolUserConfigParam(false, "artist_debug_mode",
                               "Whether to enable track debugging features") );

    PARAM_PREFIX BoolUserConfigParam        m_hide_gui
        PARAM_DEFAULT(BoolUserConfigParam(false, "debug_hide_gui",
            "Whether to hide the GUI (artist debug mode)"));

    PARAM_PREFIX BoolUserConfigParam        m_everything_unlocked
            PARAM_DEFAULT( BoolUserConfigParam(false, "everything_unlocked",
                               "Enable all karts and tracks") );

    // TODO? implement blacklist for new irrlicht device and GUI
    PARAM_PREFIX std::vector<std::string>   m_blacklist_res;

    /** List of all saved GPs. */
    PARAM_PREFIX PtrVector<SavedGrandPrix>   m_saved_grand_prix_list;

    /** Some constants to bitmask to enable various messages to be printed. */
    enum { LOG_MEMORY  = 0x0001,
           LOG_GUI     = 0x0002,
           LOG_ADDONS  = 0x0004,
           LOG_MISC    = 0x0008,
           LOG_FLYABLE = 0x0010,
           LOG_ALL     = 0xffff };

    /** Returns true if the user want additional messages for memory usage. */
    bool   logMemory();
    /** Returns true if the user want additional messages related to GUI. */
    bool   logGUI   ();
    /** Returns true if the user want additional messages related to addons. */
    bool   logAddons();
    /** Returns true if the user want additional debug info for flyables */
    bool   logFlyable();
    /** Returns true if the user want additional messages for general items. */
    bool   logMisc  ();


}
#undef PARAM_PREFIX
#undef PARAM_SUFFIX

// ============================================================================
/**
  * \brief Class for managing general STK user configuration data.
  * \ingroup config
  */
class UserConfig : public NoCopy
{
private:

    /** Filename of the user config file. */
    std::string        m_filename;
    irr::core::stringw m_warning;

    static const int m_current_config_version;

public:
          /** Create the user config object; does not actually load it,
           *  UserConfig::loadConfig needs to be called. */
          UserConfig();
         ~UserConfig();

    bool  loadConfig();
    void  saveConfig();

    const irr::core::stringw& getWarning()        { return m_warning;  }
    void  resetWarning()                          { m_warning="";      }
    void  setWarning(irr::core::stringw& warning) { m_warning=warning; }

};   // UserConfig


extern UserConfig *user_config;

#endif

/*EOF*/
