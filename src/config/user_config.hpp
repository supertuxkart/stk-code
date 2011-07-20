// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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
const int CURRENT_CONFIG_VERSION = 8;

#include <string>
#include <map>
#include <vector>
#include <fstream>

#include <irrString.h>
using irr::core::stringc;
using irr::core::stringw;

#include "graphics/camera.hpp"
#include "utils/constants.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/time.hpp"

class XMLNode;
class XMLWriter;
class PlayerProfile;

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
    virtual void write(XMLWriter& stream) const = 0;
    virtual void findYourDataInAChildOf(const XMLNode* node) = 0;
    virtual void findYourDataInAnAttributeOf(const XMLNode* node) = 0;
    virtual irr::core::stringw toString() const = 0;
};   // UserConfigParam

// ============================================================================
class GroupUserConfigParam : public UserConfigParam
{
    std::vector<UserConfigParam*> m_children;
public:
    GroupUserConfigParam(const char* name, const char* comment=NULL);
    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);

    void addChild(UserConfigParam* child);
    irr::core::stringw toString() const;
};   // GroupUserConfigParam

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

    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);
    
    irr::core::stringw toString() const;
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
    Time::TimeType m_value;
    Time::TimeType m_default_value;
    
public:
    
    TimeUserConfigParam(Time::TimeType default_value, const char* param_name, 
                        const char* comment = NULL);
    TimeUserConfigParam(Time::TimeType default_value, const char* param_name, 
                        GroupUserConfigParam* group, const char* comment=NULL);

    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);
    
    irr::core::stringw toString() const;
    void revertToDefaults()               { m_value = m_default_value;        }
    operator Time::TimeType() const       { return m_value;                   }
    Time::TimeType& operator=(const Time::TimeType& v) 
                                          { m_value = v; return m_value;      }
    Time::TimeType& operator=(const TimeUserConfigParam& v) 
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

    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);
    
    void revertToDefaults() { m_value = m_default_value; }
   
    irr::core::stringw toString() const { return m_value.c_str(); }
    
    operator std::string() const  { return m_value; }
    std::string& operator=(const std::string& v)
                                  { m_value = v; return m_value; }
    std::string& operator=(const StringUserConfigParam& v) 
                                  { m_value = (std::string)v; return m_value; }

    const char* c_str() const { return m_value.c_str(); }
};   // StringUserConfigParam

// ============================================================================
class WStringUserConfigParam : public UserConfigParam
{
    stringw m_value;
    stringw m_default_value;
    
public:
    
    WStringUserConfigParam(const stringw& default_value, 
                           const char* param_name,
                           const char* comment = NULL);
    WStringUserConfigParam(const stringw& default_value, 
                           const char* param_name,
                           GroupUserConfigParam* group, 
                           const char* comment = NULL);
    
    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);
    
    void revertToDefaults() { m_value = m_default_value; }
    
    irr::core::stringw toString() const { return m_value; }
    
    operator stringw() const { return m_value; }
    stringw& operator=(const stringw& v) { m_value = v; return m_value;  }
    stringw& operator=(const WStringUserConfigParam& v) 
                                 { m_value = (stringw)v; return m_value; }
    const wchar_t* c_str() const { return m_value.c_str(); }
};   // WStringUserConfigParam

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
    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);
    
    irr::core::stringw toString() const;
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

    void write(XMLWriter& stream) const;
    void findYourDataInAChildOf(const XMLNode* node);
    void findYourDataInAnAttributeOf(const XMLNode* node);
    
    irr::core::stringw toString() const;
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
            PARAM_DEFAULT( BoolUserConfigParam(true, "sfx_on", &m_audio_group, "Whether sound effects are enabled or not (true or false)") );
    PARAM_PREFIX BoolUserConfigParam         m_music
            PARAM_DEFAULT(  BoolUserConfigParam(true, "music_on", &m_audio_group, "Whether musics are enabled or not (true or false)") );
    PARAM_PREFIX FloatUserConfigParam       m_sfx_volume
            PARAM_DEFAULT(  FloatUserConfigParam(1.0, "sfx_volume", &m_audio_group, "Volume for sound effects, see openal AL_GAIN for interpretation") );
    PARAM_PREFIX FloatUserConfigParam       m_music_volume
            PARAM_DEFAULT(  FloatUserConfigParam(0.7f, "music_volume", &m_audio_group, "Music volume from 0.0 to 1.0") );
    
    // ---- Race setup
    PARAM_PREFIX GroupUserConfigParam        m_race_setup_group
        PARAM_DEFAULT( GroupUserConfigParam("RaceSetup", "Race Setup Settings") );
    
    PARAM_PREFIX IntUserConfigParam          m_num_karts
            PARAM_DEFAULT(  IntUserConfigParam(4, "numkarts", &m_race_setup_group, "Default number of karts. -1 means use all") );
    PARAM_PREFIX IntUserConfigParam          m_num_laps
            PARAM_DEFAULT(  IntUserConfigParam(4, "numlaps", &m_race_setup_group, "Default number of laps.") );
    PARAM_PREFIX IntUserConfigParam          m_difficulty
            PARAM_DEFAULT(  IntUserConfigParam(0, "difficulty", &m_race_setup_group, "Default race difficulty. 0=easy, 1=medium, 2=hard") );
    PARAM_PREFIX IntUserConfigParam          m_game_mode
            PARAM_DEFAULT(  IntUserConfigParam(0, "game_mode", &m_race_setup_group, "Game mode. 0=standard, 1=time trial, 2=follow the leader, 3=3 strikes") );
    PARAM_PREFIX StringUserConfigParam m_default_kart
            PARAM_DEFAULT( StringUserConfigParam("tux", "kart", "Kart to select by default (the last used kart)") );
    PARAM_PREFIX StringUserConfigParam m_last_used_kart_group
            PARAM_DEFAULT( StringUserConfigParam("all", "last_kart_group", "Last selected kart group") );
    
    // ---- Video
    PARAM_PREFIX GroupUserConfigParam        m_video_group
        PARAM_DEFAULT( GroupUserConfigParam("Video", "Video Settings") );
    
    PARAM_PREFIX IntUserConfigParam         m_width
            PARAM_DEFAULT(  IntUserConfigParam(800, "width", &m_video_group, "Screen/window width in pixels") );
    PARAM_PREFIX IntUserConfigParam         m_height
            PARAM_DEFAULT(  IntUserConfigParam(600, "height", &m_video_group, "Screen/window height in pixels") );
    PARAM_PREFIX BoolUserConfigParam        m_fullscreen
            PARAM_DEFAULT(  BoolUserConfigParam(false, "fullscreen", &m_video_group) );
    PARAM_PREFIX IntUserConfigParam         m_prev_width
            PARAM_DEFAULT(  IntUserConfigParam(800, "prev_width", &m_video_group, "Previous screen/window width") );
    PARAM_PREFIX IntUserConfigParam         m_prev_height
            PARAM_DEFAULT(  IntUserConfigParam(600, "prev_height", &m_video_group,"Previous screen/window height") );
    PARAM_PREFIX BoolUserConfigParam        m_prev_fullscreen
            PARAM_DEFAULT(  BoolUserConfigParam(false, "prev_fullscreen", &m_video_group) );


    PARAM_PREFIX BoolUserConfigParam        m_display_fps
            PARAM_DEFAULT(  BoolUserConfigParam(false, "show_fps", &m_video_group, "Display frame per seconds") );
    PARAM_PREFIX IntUserConfigParam         m_max_fps
            PARAM_DEFAULT(  IntUserConfigParam(120, "max_fps", &m_video_group, "Maximum fps, should be at least 60") );

    // Renderer type (OpenGL, Direct3D9, Direct3D8, Software, etc)
    PARAM_PREFIX IntUserConfigParam         m_renderer
            PARAM_DEFAULT(  IntUserConfigParam(0, "renderer", &m_video_group,
                                               "Type of the renderer.") );

    // ---- Debug - not saved to config file
    /** If gamepad debugging is enabled. */
    PARAM_PREFIX bool                       m_gamepad_debug     PARAM_DEFAULT( false );
    
    PARAM_PREFIX bool                       m_gamepad_visualisation PARAM_DEFAULT( false );
    
    /** If material debugging (printing terrain specific slowdown) is enabled. */
    PARAM_PREFIX bool                       m_material_debug    PARAM_DEFAULT( false );

    /** If track debugging is enabled. */
    PARAM_PREFIX int                        m_track_debug       PARAM_DEFAULT( false );

    /** True if check structures should be debugged. */
    PARAM_PREFIX bool                       m_check_debug       PARAM_DEFAULT( false );

    /** Special debug camera being high over the kart. */
    PARAM_PREFIX bool                       m_camera_debug      PARAM_DEFAULT( false );

    /** True if slipstream debugging is activated. */
    PARAM_PREFIX bool                       m_slipstream_debug  PARAM_DEFAULT( false );

    /** True if follow-the-leader debug information should be printed. */
    PARAM_PREFIX bool                       m_ftl_debug    PARAM_DEFAULT( false );
    
    /** True if currently developed tutorial debugging is enabled. */
    PARAM_PREFIX bool                       m_tutorial_debug    PARAM_DEFAULT( false );

    /** Verbosity level for debug messages. Note that error and important warnings
     *  must always be printed. */
    PARAM_PREFIX int                        m_verbosity         PARAM_DEFAULT( 0 );

    PARAM_PREFIX bool                       m_no_start_screen   PARAM_DEFAULT( false );
    
    PARAM_PREFIX bool                       m_race_now          PARAM_DEFAULT( false );

    /** True to test funky ambient/diffuse/specularity in RGB & all anisotropic */
    PARAM_PREFIX bool                       m_rendering_debug   PARAM_DEFAULT( false );
    
    /** True if graphical profiler should be displayed */
    PARAM_PREFIX bool                       m_profiler_enabled  PARAM_DEFAULT( false );
    
    /** True if hardware skinning should be enabled */
    PARAM_PREFIX bool                       m_hw_skinning_enabled  PARAM_DEFAULT( false );

    /** True if post-processing effects should be enabled */
    PARAM_PREFIX bool                       m_postprocess_enabled  PARAM_DEFAULT( false );

    // not saved to file

    // ---- Networking
    PARAM_PREFIX StringUserConfigParam      m_server_address
            PARAM_DEFAULT(  StringUserConfigParam("localhost", "server_adress", "Information about last server used") );
    PARAM_PREFIX IntUserConfigParam         m_server_port
            PARAM_DEFAULT(  IntUserConfigParam(2305, "server_port", "Information about last server used") );
    
    // ---- Graphic Quality
    PARAM_PREFIX GroupUserConfigParam        m_graphics_quality
            PARAM_DEFAULT( GroupUserConfigParam("GFX", "Graphics Quality Settings") );
    
    // On OSX 10.4 and before there may be driver issues with FBOs, so to be safe disable them by default
#ifdef __APPLE__
    #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    #define FBO_DEFAULT false
    #else
    #define FBO_DEFAULT true
    #endif
#else
#define FBO_DEFAULT true
#endif
    
    PARAM_PREFIX BoolUserConfigParam        m_fbo
        PARAM_DEFAULT(  BoolUserConfigParam(FBO_DEFAULT, "fbo", &m_graphics_quality, "Use frame buffer objects (FBOs)") );
    
    PARAM_PREFIX BoolUserConfigParam        m_graphical_effects
            PARAM_DEFAULT(  BoolUserConfigParam(true, "anim_gfx", &m_graphics_quality, "Scenery animations") );
    
    PARAM_PREFIX BoolUserConfigParam        m_weather_effects
            PARAM_DEFAULT(  BoolUserConfigParam(true, "weather_gfx", &m_graphics_quality, "Weather effects") );
    PARAM_PREFIX IntUserConfigParam        m_show_steering_animations
            PARAM_DEFAULT(  IntUserConfigParam(ANIMS_ALL, "steering_animations", &m_graphics_quality,
                                               "Whether to display kart animations (0=disabled for all; "
                                               "1=enabled for humans, disabled for AIs; 2=enabled for all") );
    PARAM_PREFIX BoolUserConfigParam         m_anisotropic
            PARAM_DEFAULT( BoolUserConfigParam(true, "anisotropic", &m_graphics_quality,
                                               "Whether anisotropic filtering is allowed to be used (true or false)") );
    PARAM_PREFIX BoolUserConfigParam         m_trilinear
            PARAM_DEFAULT( BoolUserConfigParam(true, "trilinear", &m_graphics_quality,
                                               "Whether trilinear filtering is allowed to be used (true or false)") );
    PARAM_PREFIX BoolUserConfigParam         m_fullscreen_antialiasing
            PARAM_DEFAULT( BoolUserConfigParam(false, "fullscreen_antialiasing", &m_graphics_quality,
                                               "Whether fullscreen antialiasing is enabled") );
    PARAM_PREFIX BoolUserConfigParam         m_vsync
            PARAM_DEFAULT( BoolUserConfigParam(false, "vsync", &m_graphics_quality,
                                               "Whether vertical sync is enabled") );
    
    // ---- Misc
    PARAM_PREFIX BoolUserConfigParam        m_minimal_race_gui
            PARAM_DEFAULT(  BoolUserConfigParam(false, "minimal-race-gui") );
    PARAM_PREFIX BoolUserConfigParam        m_crashed
            PARAM_DEFAULT(  BoolUserConfigParam(false, "crashed") ); // TODO : is this used with new code? does it still work?
    PARAM_PREFIX BoolUserConfigParam        m_log_errors
            PARAM_DEFAULT(  BoolUserConfigParam(false, "log_errors", "Enable logging of stdout and stderr to logfile") );
    
    PARAM_PREFIX IntUserConfigParam         m_reverse_look_threshold
            PARAM_DEFAULT(  IntUserConfigParam(0, "reverse_look_threshold", 
            "If the kart is driving backwards faster than this value,\n"
            "switch automatically to reverse camera (set to 0 to disable).") );
    PARAM_PREFIX IntUserConfigParam         m_camera_style
            PARAM_DEFAULT( IntUserConfigParam(Camera::CS_MODERN, "camera_style", "Camera Style") );

    
    PARAM_PREFIX StringUserConfigParam      m_item_style
            PARAM_DEFAULT(  StringUserConfigParam("items", "item_style", "Name of the .items file to use.") );
    
    PARAM_PREFIX StringUserConfigParam      m_last_track
            PARAM_DEFAULT(  StringUserConfigParam("olivermath", "last_track", "Name of the last track used.") ); 
    PARAM_PREFIX StringUserConfigParam m_last_used_track_group
            PARAM_DEFAULT( StringUserConfigParam("all", "last_track_group", "Last selected track group") );
    
    PARAM_PREFIX StringUserConfigParam      m_skin_file
            PARAM_DEFAULT(  StringUserConfigParam("Peach.stkskin", "skin_file", "Name of the skin to use") );
            
    // ---- Addon server related entries
    PARAM_PREFIX GroupUserConfigParam       m_addon_group
        PARAM_DEFAULT( GroupUserConfigParam("AddonAndNews", 
                                            "Addon and news related settings") );

    PARAM_PREFIX StringUserConfigParam      m_server_addons
            PARAM_DEFAULT(  StringUserConfigParam("http://stkaddons.net/dl/xml",
                                                  "server_addons", 
                                                  &m_addon_group,
                                                  "The server used for addon.") );

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
                                               "How often all news messages have been displayed") );

    PARAM_PREFIX IntUserConfigParam         m_ignore_message_id
            PARAM_DEFAULT(  IntUserConfigParam(-1, "ignore_message_id", 
                                               &m_addon_group,
                                               "Ignore all messages with this id and lower") );

    PARAM_PREFIX IntUserConfigParam        m_internet_status
            PARAM_DEFAULT(  IntUserConfigParam(0, "enable_internet",
                                               &m_addon_group,
                                               "Status of internet: 0 user wasn't asked, 1: allowed, 2: not allowed") );

    PARAM_PREFIX TimeUserConfigParam        m_addons_last_updated
            PARAM_DEFAULT(  TimeUserConfigParam(0, "addon_last_updated",
                                                &m_addon_group,
                                                "Time addon-list was updated last.") );

    PARAM_PREFIX StringUserConfigParam      m_language
            PARAM_DEFAULT( StringUserConfigParam("system", "language", "Which language to use (language code or 'system')") );
    
    PARAM_PREFIX BoolUserConfigParam        m_artist_debug_mode
            PARAM_DEFAULT( BoolUserConfigParam(false, "artist_debug_mode", "Whether to enable track debugging features") );
    
    // TODO? implement blacklist for new irrlicht device and GUI
    PARAM_PREFIX std::vector<std::string>   m_blacklist_res;
    
    PARAM_PREFIX PtrVector<PlayerProfile>   m_all_players;

    /** Some constants to bitmask to enable various messages to be printed. */
    enum { LOG_MEMORY = 0x0001,
           LOG_GUI    = 0x0002,
           LOG_ADDONS = 0x0004,
           LOG_MISC   = 0x0008,
           LOG_ALL    = 0xffff };

    /** Returns true if the user want additional messages for memory usage. */
    bool   logMemory();
    /** Returns true if the user want additional messages related to GUI. */
    bool   logGUI   ();
    /** Returns true if the user want additional messages related to addons. */
    bool   logAddons();
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
    
    void  addDefaultPlayer();

};   // UserConfig


extern UserConfig *user_config;

#endif

/*EOF*/
