//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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

#include "utils/no_copy.hpp"
#include "utils/synchronised.hpp"
#include "utils/vec3.hpp"

#include <map>
#include <string>
#include <vector>

#if HAVE_OGGVORBIS
#  ifdef __APPLE__
#    include <OpenAL/al.h>
#  else
#    include <AL/al.h>
#  endif
#else
  typedef unsigned int ALuint;
#endif


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
class SFXManager : public NoCopy
{
private:
    /** Singleton pointer. */
    static SFXManager *m_sfx_manager;

public:

    /**
      *  Entries for custom SFX sounds.  These are unique for each kart.
      * eg. kart->playCustomSFX(SFX_MANAGER::CUSTOM_HORN)
      */
    enum CustomSFX
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

    /** Status of a sound effect. */
    enum SFXStatus
    {
        SFX_UNKNOWN = -1, SFX_STOPPED = 0, SFX_PAUSED = 1, SFX_PLAYING = 2,
        SFX_INITIAL = 3
    };

private:

    /** Listener position */
    Vec3 m_position;

    /** The buffers and info for all sound effects. These are shared among all
     *  instances of SFXOpenal. */
    std::map<std::string, SFXBuffer*> m_all_sfx_types;

    /** The actual instances (sound sources) */
    std::vector<SFXBase*> m_all_sfx;

    /** The list of sound effects to be played in the next update. */
    Synchronised< std::vector<SFXBase*> > m_sfx_to_play;

    /** To play non-positional sounds without having to create a new object for each */
    std::map<std::string, SFXBase*> m_quick_sounds;

    /** listener vector (position vector + up vector) */
    float                     m_listenerVec[6];

    /** If the sfx manager has been initialised. */
    bool                      m_initialized;

    /** Master gain value, taken from the user config value. */
    float                     m_master_gain;

    void                      loadSfx();
                             SFXManager();
    virtual                 ~SFXManager();

public:
    static void create();
    static void destroy();
    void queue(SFXBase *sfx);
    void update();
    // ------------------------------------------------------------------------
    /** Static function to get the singleton sfx manager. */
    static SFXManager *get()
    {
        assert(m_sfx_manager);
        return m_sfx_manager;
    }   // get

    // ------------------------------------------------------------------------
    bool                     sfxAllowed();
    SFXBuffer*               loadSingleSfx(const XMLNode* node,
                                           const std::string &path=std::string(""),
                                           const bool load = true);
    SFXBuffer*               addSingleSfx(const std::string &sfx_name,
                                          const std::string &filename,
                                          bool               positional,
                                          float              rolloff,
                                          float              max_width,
                                          float              gain,
                                          const bool         load = true);

    SFXBase*                 createSoundSource(SFXBuffer* info,
                                               const bool addToSFXList=true,
                                               const bool owns_buffer=false);
    SFXBase*                 createSoundSource(const std::string &name,
                                               const bool addToSFXList=true);

    void                     deleteSFX(SFXBase *sfx);
    void                     deleteSFXMapping(const std::string &name);
    void                     pauseAll();
    void                     resumeAll();
    bool                     soundExist(const std::string &name);
    void                     setMasterSFXVolume(float gain);
    float                    getMasterSFXVolume() const { return m_master_gain; }

    static bool              checkError(const std::string &context);
    static const std::string getErrorString(int err);

    void                     positionListener(const Vec3 &position, const Vec3 &front);
    SFXBase*                 quickSound(const std::string &soundName);

    /** Called when sound was muted/unmuted */
    void                     soundToggled(const bool newValue);

    // ------------------------------------------------------------------------
    /** Prints the list of currently loaded sounds to stdout. Useful to
     *  debug audio leaks */
    void dump();

    // ------------------------------------------------------------------------
    /** Returns the current position of the listener. */
    Vec3 getListenerPos() const { return m_position; }

};

#endif // HEADER_SFX_MANAGER_HPP

