//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "audio/sound_manager.hpp"

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vorbis/codec.h"
#include <vorbis/vorbisfile.h>

#ifdef __APPLE__
#  include <OpenAL/al.h>
#  include <OpenAL/alc.h>
#else
#  include <AL/al.h>
#  include <AL/alc.h>
#endif

#include "audio/sfx_openal.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/constants.hpp"

SFXManager* sfx_manager= NULL;

/** Initialises the SFX manager and loads the sfx from a config file.
 */
SFXManager::SFXManager()
{
    // The sound manager initialises OpenAL
    m_initialized = sound_manager->initialized();
    m_masterGain = 1.0f;
    m_sfx_buffers.resize(NUM_SOUNDS);
    m_sfx_positional.resize(NUM_SOUNDS);
    m_sfx_rolloff.resize(NUM_SOUNDS);
    m_sfx_gain.resize(NUM_SOUNDS);
    if(!m_initialized) return;
    
    loadSfx();
    setMasterSFXVolume( UserConfigParams::m_sfx_volume );
    
}  // SoundManager

//-----------------------------------------------------------------------------
SFXManager::~SFXManager()
{
    //make sure there aren't any stray sfx's sitting around anywhere
    for(std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
        i!=m_all_sfx.end(); i++)
    {
        delete (*i);
    }   // for i in m_all_sfx
    m_all_sfx.clear();

    //the unbuffer all of the buffers
    for(unsigned int ii = 0; ii != m_sfx_buffers.size(); ii++)
    {
        alDeleteBuffers(1, &(m_sfx_buffers[ii]));
    } 
    
    sfx_manager = NULL;
}   // ~SFXManager

//----------------------------------------------------------------------------
bool SFXManager::sfxAllowed()
{
    if(!UserConfigParams::m_sfx || !m_initialized) 
        return false;
    else
        return true;
}
//----------------------------------------------------------------------------
/** Loads all sounds specified in the sound config file.
 */
void SFXManager::loadSfx()
{
    // TODO : implement as XML
    const lisp::Lisp* root = 0;
    std::string sfx_config_name = file_manager->getSFXFile("sfx.config");
    try
    {
        lisp::Parser parser;
        root = parser.parse(sfx_config_name);
    }
    catch(std::exception& e)
    {
        (void)e;  // avoid warning about unreferenced local variable
        std::ostringstream msg;
        msg << "Sfx config file '" << sfx_config_name 
            << "' does not exist - aborting.\n";
        throw std::runtime_error(msg.str());
    }

    const lisp::Lisp* lisp = root->getLisp("sfx-config");
    if(!lisp) 
    {
        std::string msg="No sfx-config node";
        throw std::runtime_error(msg);
    }
    loadSingleSfx(lisp, "ugh",           SOUND_UGH            );
    loadSingleSfx(lisp, "skid",          SOUND_SKID           );
    loadSingleSfx(lisp, "locked",        SOUND_LOCKED         );
    loadSingleSfx(lisp, "bowling_roll",  SOUND_BOWLING_ROLL   );
    loadSingleSfx(lisp, "bowling_strike",SOUND_BOWLING_STRIKE );
    loadSingleSfx(lisp, "winner",        SOUND_WINNER         );
    loadSingleSfx(lisp, "crash",         SOUND_CRASH          );
    loadSingleSfx(lisp, "grab",          SOUND_GRAB           );
    loadSingleSfx(lisp, "goo",           SOUND_GOO            );
    loadSingleSfx(lisp, "shot",          SOUND_SHOT           );
    loadSingleSfx(lisp, "wee",           SOUND_WEE            );
    loadSingleSfx(lisp, "explosion",     SOUND_EXPLOSION      );
    loadSingleSfx(lisp, "bzzt",          SOUND_BZZT           );
    loadSingleSfx(lisp, "beep",          SOUND_BEEP           );
    loadSingleSfx(lisp, "back_menu",     SOUND_BACK_MENU      );
    loadSingleSfx(lisp, "use_anvil",     SOUND_USE_ANVIL      );
    loadSingleSfx(lisp, "use_parachute", SOUND_USE_PARACHUTE  );
    loadSingleSfx(lisp, "select_menu",   SOUND_SELECT_MENU    );
    loadSingleSfx(lisp, "move_menu",     SOUND_MOVE_MENU      );
    loadSingleSfx(lisp, "full",          SOUND_FULL           );
    loadSingleSfx(lisp, "prestart",      SOUND_PRESTART       );
    loadSingleSfx(lisp, "start",         SOUND_START          );
    loadSingleSfx(lisp, "swap",          SOUND_SWAP           );

    loadSingleSfx(lisp, "engine_small",  SOUND_ENGINE_SMALL   );
    loadSingleSfx(lisp, "engine_large",  SOUND_ENGINE_LARGE   );
}   // loadSfx
//----------------------------------------------------------------------------
/** Load a vorbis file into an OpenAL buffer
    based on a routine by Peter Mulholland, used with permission (quote : "Feel free to use")
 */
bool loadVorbisBuffer(const char *name, ALuint buffer)
{
    const int ogg_endianness = (IS_LITTLE_ENDIAN ? 0 : 1);

    
    bool success = false;
    FILE *file;
    vorbis_info *info;
    OggVorbis_File oggFile;
    
    if (alIsBuffer(buffer) == AL_FALSE)
    {
        printf("Error, bad OpenAL buffer");
        return false;
    }
    
    file = fopen(name, "rb");
    
    if (file)
    {
        if (ov_open_callbacks(file, &oggFile, NULL, 0,  OV_CALLBACKS_NOCLOSE) == 0)
        {
            info = ov_info(&oggFile, -1);
            
            long len = (long)ov_pcm_total(&oggFile, -1) * info->channels * 2;    // always 16 bit data
            
            char *data = (char *) malloc(len);
            
            if (data)
            {
                int bs = -1;
                long todo = len;
                char *bufpt = data;
                
                while (todo)
                {
                    int read = ov_read(&oggFile, bufpt, todo, ogg_endianness, 2, 1, &bs);
                    todo -= read;
                    bufpt += read;
                }
                
                alBufferData(buffer, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, len, info->rate);
                success = true;
                
                free(data);
            }
            else
                printf("Error : LoadVorbisBuffer() - couldn't allocate decode buffer");
            
            ov_clear(&oggFile);
        }
        else
        {
            fclose(file);
            printf("LoadVorbisBuffer() - ov_open_callbacks() failed, file isn't vorbis?");
        }
    }
    else
        printf("LoadVorbisBuffer() - couldn't open file!");
    
    
    return success;
}
/*

getCustomTagName(int id)


    Uses CustomSFX as input (see sfx_manager.hpp) and returns associated config
    string tag.  Used when loading custom sfx in KartProperties::getAllData().

    TODO: Fix this to take advantage of some string array initialization trick
          I'm just too stupid with C++ to figure it out.  The switch code is
          less then ideal.
*/

const char *SFXManager::getCustomTagName(int id)
{
    switch (id)
    {
        case CUSTOM_HORN:    return "horn-sound";       // Replaces horn
        case CUSTOM_CRASH:   return "crash-sound";      // Played when colliding with another kart
        case CUSTOM_WIN:     return "win-sound";        // Played when racer wins (not yet implemented)
        case CUSTOM_EXPLODE: return "explode-sound";    // Played when kart explodes
        case CUSTOM_GOO:     return "goo-sound";        // Played when driving through goo
        case CUSTOM_PASS:    return "pass-sound";       // Played when passing another kart (not yet implemented)
        case CUSTOM_ZIPPER:  return "zipper-sound";     // Played when kart hits zipper
        case CUSTOM_NAME:    return "name-sound";       // Introduction "I'm Tux!" (not yet implemented)
        case CUSTOM_ATTACH:  return "attach-sound";     // Played when something is attached to kart
        case CUSTOM_SHOOT:   return "shoot-sound";      // Played when weapon is used
    };
    return "";
} // getCustomTagName

/*

addSingleSfx()

    Introduces a mechanism by which one can load sound effects beyond the basic
    enumerated types.  This will be used when loading custom sound effects for
    individual karts (character voices) so that we can avoid creating an
    enumeration for each effect, for each kart.

    sfxFile must be an absolute pathname, so get that straight first.

*/
int SFXManager::addSingleSfx(std::string    sfxFile,
                             int            positional,
                             float          rolloff,
                             float          gain)
/* Returns sfx ID or -1 on error*/
{
    int         sfxID;

    m_sfx_buffers.push_back(0);
    sfxID = m_sfx_buffers.size() - 1;

    /* FIXME: Check for existance of file before resizing vectors */

    m_sfx_rolloff.push_back(rolloff);
    m_sfx_positional.push_back(positional);
    m_sfx_gain.push_back(gain);

    alGenBuffers(1, &(m_sfx_buffers[sfxID]));
    if (!checkError("generating a buffer")) return -1;

    if (!loadVorbisBuffer(sfxFile.c_str(), m_sfx_buffers[sfxID]))
    {
        printf("Failed to load sound effect %s\n", sfxFile.c_str());
    }

    // debugging
    /*printf("addSingleSfx() id:%d sfxFile:%s\n", sfxID, sfxFile.c_str());*/

    return sfxID;
} // addSingleSFX

void SFXManager::loadSingleSfx(const lisp::Lisp* lisp, 
                               const char *name, int item)
{
    if (item < 0 || item >= (int)m_sfx_gain.size())
    {
        printf("loadSingleSfx: Invalid SFX ID.\n");
        return;
    }

    const lisp::Lisp* sfxLisp = lisp->getLisp(name);
    std::string wav; float rolloff = 0.1f; float gain = 1.0f; int positional = 0;

    sfxLisp->get("filename",    wav         );
    sfxLisp->get("roll-off",    rolloff     );
    sfxLisp->get("positional",  positional  );
    sfxLisp->get("volume",      gain        );

    m_sfx_rolloff[item] = rolloff;
    m_sfx_positional[item] = positional;
    m_sfx_gain[item] = gain;

    std::string path = file_manager->getSFXFile(wav);
    printf("Loading SFX %s\n", path.c_str());

    alGenBuffers(1, &(m_sfx_buffers[item]));
    if (!checkError("generating a buffer")) return;

    if(!loadVorbisBuffer(path.c_str(), m_sfx_buffers[item]))
    {
        printf("Could not load sound effect %s\n", name);
    }
}   // loadSingleSfx

//----------------------------------------------------------------------------
/** Creates a new SFX object. The memory for this object is managed completely
 *  by the SFXManager. This makes it easy to use different implementations of
 *  SFX - since newSFX can return whatever type is used. To free the memory,
 *  call deleteSFX().
 *  \param id Identifier of the sound effect to create.
 */
SFXBase *SFXManager::newSFX(int id)
{
    bool positional = false;

    if (id < 0 || id >= (int)m_sfx_gain.size())
    {
        printf("newSFX: Invalid SFX ID %d.\n", id);
        return NULL;
    }

    if(race_manager->getNumLocalPlayers() < 2)
        positional = m_sfx_positional[id]!=0;

    SFXBase *p = new SFXOpenAL(m_sfx_buffers[id], positional, m_sfx_rolloff[id], m_sfx_gain[id]);
    // debugging
    /*printf("newSfx(): id:%d buffer:%p, rolloff:%f, gain:%f %p\n", id, m_sfx_buffers[id], m_sfx_rolloff[id], m_sfx_gain[id], p);*/
    p->volume(m_masterGain);
    m_all_sfx.push_back(p);
    return p;
}   // newSFX

//----------------------------------------------------------------------------
/** Delete a sound effect object, and removes it from the internal list of
 *  all SFXs. This call deletes the object, and removes it from the list of
 *  all SFXs.
 *  \param sfx SFX object to delete.
 */
void SFXManager::deleteSFX(SFXBase *sfx)
{
    std::vector<SFXBase*>::iterator i;
    i=std::find(m_all_sfx.begin(), m_all_sfx.end(), sfx);

    if(i==m_all_sfx.end())
    {
        fprintf(stderr, "SFXManager::deleteSFX : Warning: sfx not found in list.\n");
        return;
    }
    
    delete sfx;

    m_all_sfx.erase(i);

}   // deleteSFX

//----------------------------------------------------------------------------
/** Pauses all looping SFXs. Non-looping SFX will be finished, since it's
 *  otherwise not possible to determine which SFX must be resumed (i.e. were
 *  actually playing at the time pause was called.
 */
void SFXManager::pauseAll()
{
    for(std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
        i!=m_all_sfx.end(); i++)
    {
        (*i)->pause();
    }   // for i in m_all_sfx
}   // pauseAll
//----------------------------------------------------------------------------
/** Resumes all paused SFXs.
 */
void SFXManager::resumeAll()
{
    for(std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
        i!=m_all_sfx.end(); i++)
    {
        SFXStatus status = (*i)->getStatus();
        // Initial happens when 
        if(status==SFX_PAUSED)
            (*i)->resume();
    }   // for i in m_all_sfx
}   // resumeAll

//-----------------------------------------------------------------------------
bool SFXManager::checkError(const std::string &context)
{
    // Check (and clear) the error flag
    int error = alGetError();

    if(error != AL_NO_ERROR)
    {
        fprintf(stdout, "SFXOpenAL OpenAL error while %s: %s\n",
            context.c_str(), SFXManager::getErrorString(error).c_str());
        return false;
    }
    return true;
}   // checkError

//-----------------------------------------------------------------------------
void SFXManager::setMasterSFXVolume(float gain)
{
    if(gain > 1.0)
        gain = 1.0f;
    if(gain < 0.0f)
        gain = 0.0f;

    m_masterGain = gain;

    for(std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
        i!=m_all_sfx.end(); i++)
    {
        (*i)->volume(m_masterGain);
    }   // for i in m_all_sfx

}   // setMasterSFXVolume

//-----------------------------------------------------------------------------
const std::string SFXManager::getErrorString(int err)
{
    switch(err)
    {
        case AL_NO_ERROR:          return std::string("AL_NO_ERROR"         );
        case AL_INVALID_NAME:      return std::string("AL_INVALID_NAME"     );
        case AL_INVALID_ENUM:      return std::string("AL_INVALID_ENUM"     );
        case AL_INVALID_VALUE:     return std::string("AL_INVALID_VALUE"    );
        case AL_INVALID_OPERATION: return std::string("AL_INVALID_OPERATION");
        case AL_OUT_OF_MEMORY:     return std::string("AL_OUT_OF_MEMORY"    );
        default:                   return std::string("UNKNOWN");
    };
}   // getErrorString

//-----------------------------------------------------------------------------
void SFXManager::quickSound(SFXType soundType)
{
    static std::map<SFXType, SFXBase*> allSounds;
    
    std::map<SFXType, SFXBase*>::iterator sound = allSounds.find(soundType);
    
    if (sound == allSounds.end())
    {
        // sound not yet in our list
        SFXBase* newSound = sfx_manager->newSFX(soundType);
        newSound->play();
        allSounds[soundType] = newSound;
    }
    else
    {
        (*sound).second->play();
    }
    
    //     m_locked_sound = sfx_manager->newSFX(SFXManager::SOUND_LOCKED);
    // m_locked_sound->play();
}


