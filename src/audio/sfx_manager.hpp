//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#ifndef HEADER_SFX_MANAGER_HPP
#define HEADER_SFX_MANAGER_HPP

#include "utils/can_be_deleted.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/synchronised.hpp"
#include "utils/vec3.hpp"

#include <condition_variable>
#include <map>
#include <string>
#include <thread>

#include <vector>

#ifdef ENABLE_SOUND
#  include <AL/al.h>
#  include <AL/alc.h>
#else
  typedef unsigned int ALuint;
#endif

class MusicInformation;
class SFXBase;
class SFXBuffer;
class XMLNode;

/**
 * \brief Manager of all sound effects.
 * The manager reads all sound effects and
 *  maintains the corresponding buffers. Each sound effect objects uses
 *  on of the (shared) buffers from the sound manager.
 * \ingroup audio
 */
class SFXManager : public NoCopy, public CanBeDeleted
{
private:
    /** Singleton pointer. */
    static SFXManager *m_sfx_manager;

public:

    /** The various commands to be executed by the sfx manager thread
     *  for each sfx. */
    enum SFXCommands
    {
        SFX_PLAY = 1,
        SFX_PLAY_POSITION,
        SFX_STOP,
        SFX_PAUSE,
        SFX_PAUSE_ALL,
        SFX_RESUME,
        SFX_RESUME_ALL,
        SFX_DELETE,
        SFX_SPEED,
        SFX_POSITION,
        SFX_SPEED_POSITION,
        SFX_VOLUME,
        SFX_MASTER_VOLUME,
        SFX_LOOP,
        SFX_LISTENER,
        SFX_UPDATE,
        SFX_MUSIC_START,
        SFX_MUSIC_STOP,
        SFX_MUSIC_PAUSE,
        SFX_MUSIC_RESUME,
        SFX_MUSIC_SWITCH_FAST,
        SFX_MUSIC_SET_TMP_VOLUME,
        SFX_MUSIC_WAITING,
        SFX_MUSIC_DEFAULT_VOLUME,
        SFX_EXIT,
        SFX_CREATE_SOURCE
    };   // SFXCommands

    /**
      *  Entries for custom SFX sounds.  These are unique for each kart.
      * eg. kart->playCustomSFX(SFX_MANAGER::CUSTOM_HORN)
      */
    enum CustomSFX : int
    {
        CUSTOM_HORN,    //!< Replaces default horn
        CUSTOM_CRASH,   //!< Played when colliding with another kart
        CUSTOM_WIN,     //!< Played when racer wins
        CUSTOM_EXPLODE, //!< Played when struck by bowling ball or dynamite
        CUSTOM_GOO,     //!< Played when driving through goo
        CUSTOM_PASS,    //!< Played when passing another kart
        CUSTOM_ZIPPER,  //!< Played when kart hits zipper
        CUSTOM_NAME,    //!< Introduction (e.g. "I'm Tux!")
        CUSTOM_ATTACH,  //!< Played when something is attached to kart (Uh-Oh)
        CUSTOM_SHOOT,   //!< Played when weapon is used
        NUM_CUSTOMS
    };

private:

    /** Data structure for the queue, which stores a sfx and the command to 
     *  execute for it. */
    class SFXCommand : public NoCopy
    {
    private:
        LEAK_CHECK()
    public:
        /** The sound effect for which the command should be executed. */
        SFXBase *m_sfx;

        /** The sound buffer to play (null = no change) */
        SFXBuffer *m_buffer = NULL;

        /** Stores music information for music commands. */
        MusicInformation *m_music_information;

        /** The command to execute. */
        SFXCommands m_command;
        /** Optional parameter for commands that need more input. Single
         *  floating point values are stored in the X component. */
        Vec3        m_parameter;
        // --------------------------------------------------------------------
        SFXCommand(SFXCommands command, SFXBase *base)
        {
            m_command   = command;
            m_sfx       = base;
        }   // SFXCommand()
        // --------------------------------------------------------------------
        /** Constructor for music information commands. */
        SFXCommand(SFXCommands command, MusicInformation *mi)
        {
            m_command           = command;
            m_music_information = mi;
        }   // SFXCommnd(MusicInformation*)
        // --------------------------------------------------------------------
        /** Constructor for music information commands that take a floating
         *  point parameter (which is stored in the X value of m_parameter). */
        SFXCommand(SFXCommands command, MusicInformation *mi, float f)
        {
            m_command = command;
            m_parameter.setX(f);
            m_music_information = mi;
        }   // SFXCommnd(MusicInformation *, float)
        // --------------------------------------------------------------------
        SFXCommand(SFXCommands command, SFXBase *base, float parameter)
        {
            m_command   = command;
            m_sfx       = base;
            m_parameter.setX(parameter);
        }   // SFXCommand(float)
        // --------------------------------------------------------------------
        SFXCommand(SFXCommands command, SFXBase *base, const Vec3 &parameter)
        {
            m_command   = command;
            m_sfx       = base;
            m_parameter = parameter;
        }   // SFXCommand(Vec3)
        // --------------------------------------------------------------------
        /** Store a float and vec3 parameter. The float is stored as W
         *  component of the vector. A bit hacky, but this class is used
         *  very frequently, so should remain as small as possible). */
        SFXCommand(SFXCommands command, SFXBase *base, float f,
                   const Vec3 &parameter)
        {
            m_command   = command;
            m_sfx       = base;
            m_parameter = parameter;
            m_parameter.setW(f);
        }   // SFXCommand(Vec3)
    };   // SFXCommand
    // ========================================================================

    /** The position of the listener. Its lock will be used to
     *  access m_listener_{position,front, up}. */
    Synchronised<Vec3>        m_listener_position;

    /** The direction the listener is facing. */
    Vec3                      m_listener_front;

    /** Up vector of the listener. */
    Vec3                      m_listener_up;


    /** The buffers and info for all sound effects. These are shared among all
     *  instances of SFXOpenal. */
    std::map<std::string, SFXBuffer*> m_all_sfx_types;

    /** The actual instances (sound sources) */
    Synchronised<std::vector<SFXBase*> > m_all_sfx;

    /** The list of sound effects to be played in the next update. */
    Synchronised< std::vector<SFXCommand*> > m_sfx_commands;

    /** To play non-positional sounds without having to create a
     *  new object for each. */
    Synchronised<std::map<std::string, SFXBase*> > m_quick_sounds;

    /** If the sfx manager has been initialised. */
    bool                      m_initialized;

    /** Master gain value, taken from the user config value. */
    float                     m_master_gain;

#ifndef __SWITCH__
    /** Thread id of the thread running in this object. */
    std::thread               m_thread;
#endif

    uint64_t                  m_last_update_time;

    /** A conditional variable to wake up the main loop. */
    std::condition_variable   m_condition_variable;

    void                      loadSfx();
                             SFXManager();
    virtual                 ~SFXManager();

    static void mainLoop(void *obj);
    void deleteSFX(SFXBase *sfx);
    void queueCommand(SFXCommand *command);
    void reallyPositionListenerNow();

public:
    static void create();
    static void destroy();
    void queue(SFXCommands command, SFXBase *sfx=NULL);
    void queue(SFXCommands command, SFXBase *sfx, float f);
    void queue(SFXCommands command, SFXBase *sfx, const Vec3 &p);
    void queue(SFXCommands command, SFXBase *sfx, float f, const Vec3 &p);
    void queue(SFXCommands command, MusicInformation *mi);
    void queue(SFXCommands command, MusicInformation *mi, float f);
    void queue(SFXCommands command, SFXBase *sfx, const Vec3 &p, SFXBuffer* buffer);

    // ------------------------------------------------------------------------
    /** Static function to get the singleton sfx manager. */
    static SFXManager *get()
    {
        return m_sfx_manager;
    }   // get

    // ------------------------------------------------------------------------
    void                     stopThread();
    bool                     sfxAllowed();
    SFXBuffer*               loadSingleSfx(const XMLNode* node,
                                           const std::string &path=std::string(""),
                                           const bool load = true);
    SFXBuffer*               addSingleSfx(const std::string &sfx_name,
                                          const std::string &filename,
                                          bool               positional,
                                          float              rolloff,
                                          float              max_dist,
                                          float              gain,
                                          const bool         load = true);

    SFXBase*                 createSoundSource(SFXBuffer* info,
                                               const bool add_to_SFX_list=true,
                                               const bool owns_buffer=false);
    SFXBase*                 createSoundSource(const std::string &name,
                                               const bool addToSFXList=true);

    void                     deleteSFXMapping(const std::string &name);
    void                     pauseAll();
    void                     reallyPauseAllNow();
    void                     resumeAll();
    void                     reallyResumeAllNow();
    void                     update();
    void                     reallyUpdateNow(SFXCommand *current);
    bool                     soundExist(const std::string &name);
    void                     setMasterSFXVolume(float gain);
    float                    getMasterSFXVolume() const { return m_master_gain; }

    static bool              checkError(const std::string &context);
    static const std::string getErrorString(int err);

    void                     positionListener(const Vec3 &position,
                                              const Vec3 &front, const Vec3 &up);
    SFXBase*                 quickSound(const std::string &soundName);

    /** Called when sound was muted/unmuted */
    void                     toggleSound(const bool newValue);

    // ------------------------------------------------------------------------
    /** Prints the list of currently loaded sounds to stdout. Useful to
     *  debug audio leaks */
    void dump();

    // ------------------------------------------------------------------------
    /** Returns the current position of the listener. */
    Vec3 getListenerPos() const { return m_listener_position.getAtomic(); }

    // ------------------------------------------------------------------------

    SFXBuffer* getBuffer(const std::string &name);
};

#endif // HEADER_SFX_MANAGER_HPP

