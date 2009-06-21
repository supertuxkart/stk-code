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
const int CURRENT_CONFIG_VERSION = 7;

#include <string>
#include <map>
#include <vector>
#include <fstream>

#include "input/input.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "utils/ptr_vector.hpp"

class XMLNode;
class Player;

/**
  * The base of a set of small utilities to enable quickly adding/removing stuff to/from config painlessly.
  */
class UserConfigParam
{
    friend class GroupUserConfigParam;
protected:
    std::string paramName, comment;
public:
    virtual ~UserConfigParam();
    virtual void write(std::ofstream& stream) const = 0;
    virtual void readAsNode(const XMLNode* node) = 0;
    virtual void readAsProperty(const XMLNode* node) = 0;
    virtual std::string toString() const = 0;
};

class GroupUserConfigParam : public UserConfigParam
{
    std::vector<UserConfigParam*> m_children;
public:
    GroupUserConfigParam(const char* name, const char* comment=NULL);
    void write(std::ofstream& stream) const;
    void readAsNode(const XMLNode* node);
    void readAsProperty(const XMLNode* node);

    void addChild(UserConfigParam* child);
    std::string toString() const;
};

class IntUserConfigParam : public UserConfigParam
{
    int value;
public:
    IntUserConfigParam(int defaultValue, const char* paramName, const char* comment = NULL);
    IntUserConfigParam(int defaultValue, const char* paramName, GroupUserConfigParam* group, const char* comment = NULL);

    void write(std::ofstream& stream) const;
    void readAsNode(const XMLNode* node);
    void readAsProperty(const XMLNode* node);
    
    std::string toString() const;
    
    operator int() const { return value; }
    int& operator=(const int& v) { value = v; return value; }
};

class StringUserConfigParam : public UserConfigParam
{
    std::string value;
public:
    StringUserConfigParam(const char* defaultValue, const char* paramName, const char* comment = NULL);
    StringUserConfigParam(const char* defaultValue, const char* paramName, GroupUserConfigParam* group, const char* comment = NULL);

    
    void write(std::ofstream& stream) const;
    void readAsNode(const XMLNode* node);
    void readAsProperty(const XMLNode* node);
    
    std::string toString() const;
    
    operator std::string() const { return value; }
    std::string& operator=(const std::string& v) { value = v; return value; }
    const char* c_str() const { return value.c_str(); }
};

class BoolUserConfigParam : public UserConfigParam
{
    bool value;
public:
    BoolUserConfigParam(bool defaultValue, const char* paramName, const char* comment = NULL);
    BoolUserConfigParam(bool defaultValue, const char* paramName, GroupUserConfigParam* group, const char* comment = NULL);

    
    void write(std::ofstream& stream) const;
    void readAsNode(const XMLNode* node);
    void readAsProperty(const XMLNode* node);
    
    std::string toString() const;
    
    operator bool() const { return value; }
    bool& operator=(const bool& v) { value = v; return value; }
};

class FloatUserConfigParam : public UserConfigParam
{
    float value;
public:
    FloatUserConfigParam(float defaultValue, const char* paramName, const char* comment = NULL);
    FloatUserConfigParam(float defaultValue, const char* paramName, GroupUserConfigParam* group, const char* comment = NULL);

    void write(std::ofstream& stream) const;
    void readAsNode(const XMLNode* node);
    void readAsProperty(const XMLNode* node);
    
    std::string toString() const;
    
    operator float() const { return value; }
    float& operator=(const float& v) { value = v; return value; }
};


/**
  * Using X-macros for setting-possible values is not very pretty, but it's a no-maintenance case :
  * when you want to add a new parameter, just add one signle line below and everything else automagically works
  * (including default value, saving to file, loading from file)
  */

#ifndef PARAM_PREFIX
#define PARAM_PREFIX extern
#endif

#ifndef PARAM_DEFAULT
#define PARAM_DEFAULT(X)
#endif

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
            PARAM_DEFAULT(  FloatUserConfigParam(0.7f, "music_volume", &m_audio_group, "music volume from 0 to 1") );
    
    // ---- Race setup
    PARAM_PREFIX GroupUserConfigParam        m_race_setup_group
        PARAM_DEFAULT( GroupUserConfigParam("RaceSetup", "Race Setup Settings") );
    
    PARAM_PREFIX IntUserConfigParam          m_num_karts
            PARAM_DEFAULT(  IntUserConfigParam(4, "numkarts", &m_race_setup_group, "Default number of karts. -1 means use all") );
    PARAM_PREFIX IntUserConfigParam          m_num_laps
            PARAM_DEFAULT(  IntUserConfigParam(4, "numlaps", &m_race_setup_group, "Default number of laps.") );
    PARAM_PREFIX IntUserConfigParam          m_difficulty
            PARAM_DEFAULT(  IntUserConfigParam(0, "difficulty", &m_race_setup_group, "Default race difficulty. 0=easy, 1=medium, 2=hard") );
    
    // ---- Video
    PARAM_PREFIX GroupUserConfigParam        m_video_group
        PARAM_DEFAULT( GroupUserConfigParam("Video", "Video Settings") );
    
    PARAM_PREFIX IntUserConfigParam         m_width
            PARAM_DEFAULT(  IntUserConfigParam(800, "width", &m_video_group, "screen resolution width") );
    PARAM_PREFIX IntUserConfigParam         m_height
            PARAM_DEFAULT(  IntUserConfigParam(600, "height", &m_video_group, "screen resolution height") );
    PARAM_PREFIX BoolUserConfigParam        m_fullscreen
            PARAM_DEFAULT(  BoolUserConfigParam(false, "fullscreen", &m_video_group) );
    PARAM_PREFIX IntUserConfigParam         m_prev_width
            PARAM_DEFAULT(  IntUserConfigParam(800, "prev_width", &m_video_group) );
    PARAM_PREFIX IntUserConfigParam         m_prev_height
            PARAM_DEFAULT(  IntUserConfigParam(600, "prev_height", &m_video_group) );
    PARAM_PREFIX BoolUserConfigParam        m_prev_windowed
            PARAM_DEFAULT(  BoolUserConfigParam(true, "prev_windowed", &m_video_group) );
    
    // TODO : adapt to be more powerful with irrlicht
    PARAM_PREFIX BoolUserConfigParam        m_graphical_effects
            PARAM_DEFAULT(  BoolUserConfigParam(true, "gfx", &m_video_group) );
    
    PARAM_PREFIX BoolUserConfigParam        m_display_fps
            PARAM_DEFAULT(  BoolUserConfigParam(false, "show_fps", &m_video_group, "Display frame per seconds") );
    PARAM_PREFIX IntUserConfigParam         m_max_fps
            PARAM_DEFAULT(  IntUserConfigParam(120, "max_fps", &m_video_group, "maximum fps, should be at least 60") );

    // ---- Debug
    PARAM_PREFIX BoolUserConfigParam        m_gamepad_debug     PARAM_DEFAULT( BoolUserConfigParam(false, "gamepad_debug") );
    PARAM_PREFIX BoolUserConfigParam        m_track_debug       PARAM_DEFAULT( BoolUserConfigParam(false, "track_debug") );
    PARAM_PREFIX bool                       m_bullet_debug      PARAM_DEFAULT( false );
    PARAM_PREFIX bool                       m_print_kart_sizes  PARAM_DEFAULT( false );

    // Used to profile AI. Not saved to file.
    // Positive: time in seconds; Negative: # laps; 0: no profiling.
    PARAM_PREFIX int         m_profile                          PARAM_DEFAULT( 0 ); 

    
    // ---- Networking
    PARAM_PREFIX StringUserConfigParam      m_server_address
            PARAM_DEFAULT(  StringUserConfigParam("localhost", "server_adress", "Information about last server used") );
    PARAM_PREFIX IntUserConfigParam         m_server_port
            PARAM_DEFAULT(  IntUserConfigParam(2305, "server_port", "Information about last server used") );
    
    // ---- Misc
    PARAM_PREFIX BoolUserConfigParam        m_crashed
            PARAM_DEFAULT(  BoolUserConfigParam(false, "crashed") ); // TODO : is this used with new code? does it still work?
    PARAM_PREFIX BoolUserConfigParam        m_log_errors
            PARAM_DEFAULT(  BoolUserConfigParam(false, "log_errors", "Enable logging of stdout and stderr to logfile") );
    
    PARAM_PREFIX StringUserConfigParam      m_item_style
            PARAM_DEFAULT(  StringUserConfigParam("items", "item_style", "Name of the .items file to use.") );
    
    PARAM_PREFIX StringUserConfigParam      m_kart_group
            PARAM_DEFAULT(  StringUserConfigParam("standard", "kart_group", "Last selected kart group") );
    PARAM_PREFIX StringUserConfigParam      m_track_group
            PARAM_DEFAULT(  StringUserConfigParam("standard", "track_group", "Last selected track group") ); 
    PARAM_PREFIX StringUserConfigParam      m_last_track
            PARAM_DEFAULT(  StringUserConfigParam("jungle", "last_track", "Name of the last track used.") ); 
    
    PARAM_PREFIX StringUserConfigParam      m_skin_file
            PARAM_DEFAULT(  StringUserConfigParam("glass.stkskin", "skin_file", "Name of the skin to use") );

    PARAM_PREFIX bool        m_no_start_screen   PARAM_DEFAULT( false ); // not saved to file

    // TODO? implement blacklist for new irrlicht device and GUI
    PARAM_PREFIX std::vector<std::string>   m_blacklist_res;
    
    PARAM_PREFIX ptr_vector<Player>         m_player;
    
}
#undef PARAM_PREFIX
#undef PARAM_SUFFIX

/** Class for managing general STK user configuration data. */
class UserConfig
{
private:
   
    /** Filename of the user config file. */
    std::string m_filename;

    void        addDefaultPlayer ();

public:

    std::string m_warning;
    int         CheckAndCreateDir();
    
         UserConfig();
         UserConfig(const std::string& filename);
        ~UserConfig();
    void setDefaults();

    bool  loadConfig();
    bool  loadConfig(const std::string& filename);
    void  saveConfig()                    { saveConfig(m_filename);     }
    void  saveConfig(const std::string& filename);

    const std::string
         &getWarning()                     { return m_warning;  }
    void  resetWarning()                   { m_warning="";      }
    void  setWarning(std::string& warning) { m_warning=warning; }
    
};


extern UserConfig *user_config;

#endif

/*EOF*/
