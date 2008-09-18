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

#include <assert.h>
#include <fstream>
#include <stdexcept>
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strcasecmp _strcmpi
#  define snprintf _snprintf
#endif

#ifdef __APPLE__
#  include <OpenAL/al.h>
#  include <OpenAL/alc.h>
#else
#  include <AL/al.h>
#  include <AL/alc.h>
#endif

#include "audio/sound_manager.hpp"
#include "audio/sfx_openal.hpp"
#include "user_config.hpp"
#include "file_manager.hpp"
#include "translation.hpp"

SFXManager* sfx_manager= NULL;

SFXManager::SFXManager()
{
    m_initialized = sound_manager->sfxAllowed();
    m_sfx_buffers.resize(NUM_SOUNDS);
    loadSfx();
}  // SoundManager

//-----------------------------------------------------------------------------
SFXManager::~SFXManager()
{
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
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), 
                     "Sfx config file '%s' does not exist - aborting.\n",
                     sfx_config_name.c_str());
            throw std::runtime_error(msg);
    }

    const lisp::Lisp* lisp = root->getLisp("sfx-config");
    if(!lisp) 
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), "No sfx-config node");
        throw std::runtime_error(msg);
    }
    loadSingleSfx(lisp, "ugh",           SOUND_UGH          );
    loadSingleSfx(lisp, "winner",        SOUND_WINNER       );
    loadSingleSfx(lisp, "crash",         SOUND_CRASH        );
    loadSingleSfx(lisp, "grab",          SOUND_GRAB         );
    loadSingleSfx(lisp, "shot",          SOUND_SHOT         );
    loadSingleSfx(lisp, "wee",           SOUND_WEE          );
    loadSingleSfx(lisp, "explosion",     SOUND_EXPLOSION    );
    loadSingleSfx(lisp, "bzzt",          SOUND_BZZT         );
    loadSingleSfx(lisp, "beep",          SOUND_BEEP         );
    loadSingleSfx(lisp, "back_menu",     SOUND_BACK_MENU    );
    loadSingleSfx(lisp, "use_anvil",     SOUND_USE_ANVIL    );
    loadSingleSfx(lisp, "use_parachute", SOUND_USE_PARACHUTE);
    loadSingleSfx(lisp, "select_menu",   SOUND_SELECT_MENU  );
    loadSingleSfx(lisp, "move_menu",     SOUND_MOVE_MENU    );
    loadSingleSfx(lisp, "full",          SOUND_FULL         );
    loadSingleSfx(lisp, "prestart",      SOUND_PRESTART     );
    loadSingleSfx(lisp, "start",         SOUND_START        );
    loadSingleSfx(lisp, "missile_lock",  SOUND_MISSILE_LOCK );
    loadSingleSfx(lisp, "engine",        SOUND_ENGINE       );
}   // loadSfx
//----------------------------------------------------------------------------
void SFXManager::loadSingleSfx(const lisp::Lisp* lisp, 
                               const char *name, SFXType item)
{
    std::string wav;
    lisp->get(name, wav);

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
SFXBase *SFXManager::getSfx(SFXType id)
{
    SFXBase *p=new SFXOpenAL(m_sfx_buffers[id]);
    return p;
}   // getSfx

//----------------------------------------------------------------------------
bool SFXManager::checkError(const std::string &context)
{
    // Check (and clear) the error flag
    int error = alGetError();

    if(error != AL_NO_ERROR)
    {
        fprintf(stdout, "SFXOpenAL OpenAL error while %s: %s\n",
            context.c_str(), SFXManager::getErrorString(error).c_str());
        //m_loaded = 0;//prevents further usage of this sound effect
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
