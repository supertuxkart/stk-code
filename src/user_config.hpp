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

#define PLAYERS 4

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
#define CURRENT_CONFIG_VERSION   7

#include <string>
#include <map>
#include <vector>
#include "input/input.hpp"
#include "player.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"

//class ActionMap;
struct Input;

/** Class for managing general STK user configuration data. */
class UserConfig
{
public:
    /** Stores information about joystick and gamepads. */
    /*
    class StickConfig
    {
    public:
        std::string  m_id;
        int          m_preferredIndex;
        int          m_deadzone;
        StickConfig(const std::string &id) : m_id(id) {}
    };
     */

private:
    // This class stores the last used input configuration (i.e. which action
    // is used for left, right, ..., look back) for a certain input
    // device (i.e. keyboard, joystick, ...)
    /*
    struct InputConfiguration
    {
        Input m_input[PA_COUNT];
    };
     */
    // The mapping of input device name to the last used configuration.
    // Note that std::map can not be used with Input[KC_COUNT] as 2nd
    // parameter
    // std::map<std::string, InputConfiguration> m_last_input_configuration;

    //std::string getInputDeviceName(int player_index) const;

    //std::vector <StickConfig *> m_stickconfigs;
    /*
    typedef struct
    {
        int count;
        Input inputs[4];
    } InputMapEntry;
     */

    /** Filename of the user config file. */
    std::string m_filename;

    /** Stores the GameAction->Input mappings in a way that is suitable for
     * quick modification of the mappings. Internally this allows multiple
     * Input instances per GameAction but the public methods allow only one
     * mapping.
     *
     * It is named after what is put in as values.
     */
    //InputMapEntry m_input_map[GA_COUNT];

    void        setFilename      ();

    // Attributes which have setter/getter
    int         m_sfx;
    int         m_music;
    std::string m_warning;
    /** Default number of karts. */
    int         m_num_karts;
    /** Default number of laps. */
    int         m_num_laps;
    /** Default difficulty. */
    int         m_difficulty;

    /** Index of current background image. */
    int         m_background_index;

    //void readStickConfigs(const lisp::Lisp *);

    //void writeStickConfigs(lisp::Writer *);

    /** Iterates through the input mapping and unsets all
     * where the given input occurs.
     *
     * This makes sure an input is not bound multiple times.
     */
    void unsetDuplicates(PlayerAction, const Input &);

    /** Creates an GameAction->Input mapping with one Input */
    //void setStaticAction(StaticAction, const Input &);

    /** Creates an GameAction->Input mapping with two Inputs */
    //void setStaticAction(StaticAction, const Input &, const Input &);

    /** Creates an GameAction->Input mapping with three Inputs */
    //void setStaticAction(StaticAction, const Input &, const Input &, const Input &);

    /** Creates an GameAction->Input mapping with four Inputs */
    //void setStaticAction(StaticAction, const Input &, const Input &, const Input &, const Input &);

    std::string getInputAsString(const Input &);

    /** Creates an ActionMap for the GameAction values of the specified
     * range.
     */
   // ActionMap *newActionMap(const int, const int);

    /** Sets the Input for the given GameAction. Includes a check for
     * duplicates and automatic removing of the other candidate(s).
     *
     * For use when reading from file.
     */
    void setInput(PlayerAction, const Input &);

public:
    enum UC_Mode {UC_ENABLE, UC_DISABLE, UC_TEMPORARY_DISABLE};

    int         CheckAndCreateDir();
    
    // Attributes that are accessed directly.
    bool        m_gamepad_debug;
    int         m_track_debug;
    bool        m_bullet_debug;
    bool        m_fullscreen;
    bool        m_no_start_screen;
    bool        m_graphical_effects;
    bool        m_display_fps;
    int         m_profile;         // Positive number: time in seconds, neg: # laps. (used to profile AI)
    bool        m_print_kart_sizes; // print all kart sizes
                                   // 0 if no profiling. Never saved in config file!
    float       m_sfx_volume;
    float       m_music_volume;
    
    int         m_max_fps;
    std::string m_item_style;
    std::string m_username;
    std::string m_background_music;
    std::string m_kart_group;      /**< Kart group used last.        */
    std::string m_track_group;     /**< Track group used last.       */
    std::string m_last_track;      /**< name of the last track used. */
    std::string m_server_address;
    int         m_server_port;
    int         m_width;
    int         m_height;
    int         m_prev_width;
    int         m_prev_height;
    bool        m_prev_windowed;
    bool        m_crashed;
    std::vector<std::string>
                m_blacklist_res;
    Player      m_player[PLAYERS];
    bool        m_log_errors;

         UserConfig();
         UserConfig(const std::string& filename);
        ~UserConfig();
    void setDefaults();
    void setMusic(int m)                  { m_music        =  m;        }
    void setSFX(int m)                    { m_sfx          =  m;        }
    bool doMusic() const                  { return m_music == UC_ENABLE;}
    bool doSFX()   const                  { return m_sfx   == UC_ENABLE;}
    /** Sets the default number of karts. This is only used to store
     *  this number in the user config file as a default next time. */
    void setDefaultNumKarts(int n)        { m_num_karts = n;            }
    /** Returns the default number of karts. */
    int  getDefaultNumKarts() const       { return m_num_karts;         }

    /** Sets the default number of laps. This is only used to store
     *  this number in the user config file as a default next time. */
    void setDefaultNumLaps(int n)         { m_num_laps = n;             }
    /** Returns the default number of laps. */
    int getDefaultNumLaps() const         { return m_num_laps;          }

    /** Sets the default difficulty. This is only used to store
     *  this number in the user config file as a default next time. */
    void setDefaultNumDifficulty(int n)   { m_difficulty = n;           }
    /** Returns the default difficulty. */
    int getDefaultDifficulty() const      { return m_difficulty;        }
    void nextBackgroundIndex();

    /** Get the index of the background image. */
    int   getBackgroundIndex() const      { return m_background_index;  }

    void  loadConfig();
    void  loadConfig(const std::string& filename);
    void  saveConfig()                    { saveConfig(m_filename);     }
    void  saveConfig(const std::string& filename);
    /*
    void  addStickConfig(UserConfig::StickConfig *sc)
                                          {m_stickconfigs.push_back(sc);}
    const std::vector<StickConfig *>
         *getStickConfigs() const         { return &m_stickconfigs;     }
     */
    const std::string
         &getWarning()                     { return m_warning;  }
    void  resetWarning()                   { m_warning="";      }
    void  setWarning(std::string& warning) { m_warning=warning; }
    
};


extern UserConfig *user_config;

#endif

/*EOF*/
