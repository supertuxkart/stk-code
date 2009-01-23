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

#ifdef __APPLE__
#  include <OpenAL/al.h>
#  include <OpenAL/alc.h>
#else
#  include <AL/al.h>
#  include <AL/alc.h>
#endif

#include "audio/sfx_openal.hpp"
#include "user_config.hpp"
#include "file_manager.hpp"
#include "race_manager.hpp"

SFXManager* sfx_manager= NULL;

/** Initialises the SFX manager and loads the sfx from a config file.
 */
SFXManager::SFXManager()
{
    // The sound manager initialises OpenAL
    m_initialized = sound_manager->initialized();
    m_sfx_buffers.resize(NUM_SOUNDS);
    m_sfx_positional.resize(NUM_SOUNDS);
    m_sfx_rolloff.resize(NUM_SOUNDS);
    m_sfx_gain.resize(NUM_SOUNDS);
    if(!m_initialized) return;
    loadSfx();
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

    //the unbuffer all of the buffers
    for(unsigned int ii = 0; ii != m_sfx_buffers.size(); ii++)
    {
        alDeleteBuffers(1, &(m_sfx_buffers[ii]));
    } 
}   // ~SFXManager

//----------------------------------------------------------------------------
bool SFXManager::sfxAllowed()
{
    if(!user_config->doSFX() || !m_initialized) 
        return false;
    else
        return true;
}
//----------------------------------------------------------------------------
/** Loads all sounds specified in the sound config file.
 */
void SFXManager::loadSfx()
{
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
    loadSingleSfx(lisp, "engine_small",  SOUND_ENGINE_SMALL   );
    loadSingleSfx(lisp, "engine_large",  SOUND_ENGINE_LARGE   );
}   // loadSfx
//----------------------------------------------------------------------------
void SFXManager::loadSingleSfx(const lisp::Lisp* lisp, 
                               const char *name, SFXType item)
{
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

    alGenBuffers(1, &(m_sfx_buffers[item]));
    if (!checkError("generating a buffer")) return;

    ALenum format = 0;
    Uint32 size = 0;
    Uint8* data = NULL;
    SDL_AudioSpec spec;

    if( SDL_LoadWAV( path.c_str(), &spec, &data, &size ) == NULL)
    {
        fprintf(stdout, "SDL_LoadWAV() failed to load %s\n", path.c_str());
        return;
    }

    switch( spec.format )
    {
        case AUDIO_U8:
        case AUDIO_S8:
            format = ( spec.channels == 2 ) ? AL_FORMAT_STEREO8
                                            : AL_FORMAT_MONO8;
            break;
        case AUDIO_U16LSB:
        case AUDIO_S16LSB:
        case AUDIO_U16MSB:
        case AUDIO_S16MSB:
            format = ( spec.channels == 2 ) ? AL_FORMAT_STEREO16
                                            : AL_FORMAT_MONO16;
            
#if defined(WORDS_BIGENDIAN) || SDL_BYTEORDER==SDL_BIG_ENDIAN
            // swap bytes around for big-endian systems
            for(unsigned int n=0; n<size-1; n+=2)
            {
                Uint8 temp = data[n+1];
                data[n+1] = data[n];
                data[n] = temp;
            }
#endif
            
            break;
    }   // switch( spec.format )

    alBufferData(m_sfx_buffers[item], format, data, size, spec.freq);
    if (!checkError("buffering data")) return;

    SDL_FreeWAV(data);

}   // loadSingleSfx

//----------------------------------------------------------------------------
/** Creates a new SFX object. The memory for this object is managed completely
 *  by the SFXManager. This makes it easy to use different implementations of
 *  SFX - since newSFX can return whatever type is used. To free the memory,
 *  call deleteSFX().
 *  \param id Identifier of the sound effect to create.
 */
SFXBase *SFXManager::newSFX(SFXType id)
{
    bool positional = false;
    if(race_manager->getNumLocalPlayers() < 2)
        positional = m_sfx_positional[id]!=0;

    SFXBase *p=new SFXOpenAL(m_sfx_buffers[id], positional, m_sfx_rolloff[id], m_sfx_gain[id]);
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
    delete sfx;
    if(i==m_all_sfx.end())
    {
        fprintf(stderr, "SFXManager::deleteSFX : Warning: sfx not found in list.\n");
        return;
    }

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

//----------------------------------------------------------------------------
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
