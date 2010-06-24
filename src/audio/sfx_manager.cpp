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

#include "audio/music_manager.hpp"

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
std::map<std::string, SFXBase*> SFXManager::m_quick_sounds;

/** Initialises the SFX manager and loads the sfx from a config file.
 */
SFXManager::SFXManager()
{
    // The sound manager initialises OpenAL
    m_initialized = music_manager->initialized();
    m_master_gain = 1.0f;
    
    loadSfx();
    if (!sfxAllowed()) return;
    setMasterSFXVolume( UserConfigParams::m_sfx_volume );
    
}  // SoundManager

//-----------------------------------------------------------------------------
SFXManager::~SFXManager()
{
    // ---- clear m_all_sfx
    const int sfx_amount = m_all_sfx.size();
    for (int n=0; n<sfx_amount; n++)
    {
        delete m_all_sfx[n];
    }
    m_all_sfx.clear();

    // ---- clear m_quick_sounds
    {
        std::map<std::string, SFXBase*>::iterator i = m_quick_sounds.begin();
        for (; i != m_quick_sounds.end(); i++)
        {
            SFXBase* snd = (*i).second;
            delete snd;
        }
    }
    m_quick_sounds.clear();
    
    // ---- clear m_all_sfx_types
    {
        std::map<std::string, SFXBufferInfo>::iterator i = m_all_sfx_types.begin();
        for (; i != m_all_sfx_types.end(); i++)
        {
            (*i).second.freeBuffer();
        }
        m_all_sfx_types.clear();
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
}   // sfxAllowed

//----------------------------------------------------------------------------
/** Loads all sounds specified in the sound config file.
 */
void SFXManager::loadSfx()
{
    std::string sfx_config_name = file_manager->getSFXFile("sfx.xml");
    XMLNode* root = file_manager->createXMLTree(sfx_config_name);
    if (!root || root->getName()!="sfx-config")
    {
        std::cerr << "Could not read sounf effects XML file " << sfx_config_name.c_str() << std::endl;
    }
    
    const int amount = root->getNumNodes();
    for (int i=0; i<amount; i++)
    {
        const XMLNode* node = root->getNode(i);
        
        if (node->getName() == "sfx")
        {
            loadSingleSfx(node);
        }
        else
        {
            std::cerr << "Unknown node in sfx XML file : " << node->getName().c_str() << std::endl;
            throw std::runtime_error("Unknown node in sfx XML file");
        }
    }// nend for
    
    delete root;
   }   // loadSfx

//----------------------------------------------------------------------------
/** Load a vorbis file into an OpenAL buffer
 *  based on a routine by Peter Mulholland, used with permission (quote : 
 *  "Feel free to use")
 */
bool SFXManager::loadVorbisBuffer(const std::string &name, ALuint buffer)
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
    
    file = fopen(name.c_str(), "rb");
    
    if(!file)
    {
        printf("LoadVorbisBuffer() - couldn't open file!");
	return false;
    }

    if (ov_open_callbacks(file, &oggFile, NULL, 0,  OV_CALLBACKS_NOCLOSE) != 0)
    {
        fclose(file);
        printf("LoadVorbisBuffer() - ov_open_callbacks() failed, file isn't vorbis?");
	return false;
    }

    info = ov_info(&oggFile, -1);
    
    long len = (long)ov_pcm_total(&oggFile, -1) * info->channels * 2;    // always 16 bit data
    
    char *data = (char *) malloc(len);
    if(!data)
    {
        ov_clear(&oggFile);
        printf("Error : LoadVorbisBuffer() - couldn't allocate decode buffer");
	return false;
    }

    int bs = -1;
    long todo = len;
    char *bufpt = data;
    
    while (todo)
    {
        int read = ov_read(&oggFile, bufpt, todo, ogg_endianness, 2, 1, &bs);
        todo -= read;
        bufpt += read;
    }
    
    alBufferData(buffer, (info->channels == 1) ? AL_FORMAT_MONO16 
                                               : AL_FORMAT_STEREO16,
                  data, len, info->rate);
    success = true;
    
    free(data);
    
    ov_clear(&oggFile);

    return success;
}

// -----------------------------------------------------------------------------
/*

getCustomTagName(int id)


    Uses CustomSFX as input (see sfx_manager.hpp) and returns associated config
    string tag.  Used when loading custom sfx in KartProperties::getAllData().

    TODO: Fix this to take advantage of some string array initialization trick
          I'm just too stupid with C++ to figure it out.  The switch code is
          less then ideal.
*/
/*
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
*/

// -----------------------------------------------------------------------------
/** Introduces a mechanism by which one can load sound effects beyond the basic
 *  enumerated types.  This will be used when loading custom sound effects for
 *  individual karts (character voices) so that we can avoid creating an
 *  enumeration for each effect, for each kart.
 *  \param sfx_name
 *  \param sfxFile must be an absolute pathname
 *  \return        whether loading this sound effect was successful

*/
bool SFXManager::addSingleSfx(const std::string &sfx_name,
                              const std::string &sfx_file,
                              bool               positional,
                              float              rolloff,
                              float              gain)
{

    SFXBufferInfo sfx_info;
    sfx_info.m_rolloff    = rolloff;
    sfx_info.m_positional = positional;
    sfx_info.m_gain       = gain;
    
    if(!m_initialized) 
    {
        // Even if not initialised store the (useless) data in the mapping to
        // avoid later warnings/errors.
        m_all_sfx_types[sfx_name] = sfx_info;
        return false;
    }

    if(UserConfigParams::m_verbosity>=5) 
        printf("Loading SFX %s\n", sfx_file.c_str());
    
    alGetError(); // clear errors from previously
    
    alGenBuffers(1, &sfx_info.m_buffer);
    if (!checkError("generating a buffer"))
    {
        return false;
    }

    assert( alIsBuffer(sfx_info.m_buffer) );

    if (!loadVorbisBuffer(sfx_file, sfx_info.m_buffer))
    {
        fprintf(stderr, "Could not load sound effect %s\n", sfx_file.c_str());
        return false;
    }
    
    m_all_sfx_types[sfx_name] = sfx_info;
    
    return true;
} // addSingleSFX

//----------------------------------------------------------------------------
/** Loads a single sfx from the XML specification.
 *  \param node The XML node with the data for this sfx.
 */
void SFXManager::loadSingleSfx(const XMLNode* node,
                               const std::string &path)
{
    std::string filename;

    if (node->get("filename", &filename) == 0)
    {
        fprintf(stderr, 
                "/!\\ The 'filename' attribute is mandatory in the SFX XML file!\n");
        return;
    }
    
    std::string sfx_name;
    if (node->get("name", &sfx_name) == 0)
    {
        fprintf(stderr, 
                "/!\\ The 'name' attribute is mandatory in the SFX XML file!\n");
        return;
    }
    if(m_all_sfx_types.find(sfx_name)!=m_all_sfx_types.end())
    {
        fprintf(stderr, 
                "There is already a sfx named '%s' installed - new one is ignored.\n",
                sfx_name.c_str());
        return;
    }

    SFXBufferInfo sfx_info;
    node->get("rolloff",     &sfx_info.m_rolloff     );
    node->get("positional",  &sfx_info.m_positional  );
    node->get("volume",      &sfx_info.m_gain        );

    // Only use the filename if no full path is specified. This is used
    // to load terrain specific sfx.
    const std::string full_path = (path=="") ? file_manager->getSFXFile(filename)
                                             : path;
    addSingleSfx(sfx_name, full_path, sfx_info.m_positional, sfx_info.m_rolloff,
                 sfx_info.m_gain);

    /*
    std::map<std::string, SFXBufferInfo>::iterator i = m_all_sfx_types.begin();
    for (; i != m_all_sfx_types.end(); i++ )
    {
        printf("    got '%s', buffer is %i\n", i->first.c_str(), i->second.m_sfx_buffer);
    }
     */
    
    
}   // loadSingleSfx

//----------------------------------------------------------------------------
/** Creates a new SFX object. The memory for this object is managed completely
 *  by the SFXManager. This makes it easy to use different implementations of
 *  SFX - since createSoundSource can return whatever type is used. To free the memory,
 *  call deleteSFX().
 *  \param id Identifier of the sound effect to create.
 */
SFXBase* SFXManager::createSoundSource(const SFXBufferInfo& info, 
                                       const bool add_to_SFX_list)
{
    bool positional = false;

    if (race_manager->getNumLocalPlayers() < 2)
    {
        positional = info.m_positional;
    }

    assert( alIsBuffer(info.m_buffer) );

    SFXBase* sfx = new SFXOpenAL(info.m_buffer, positional, info.m_rolloff, info.m_gain);
    
    // debugging
    /*printf("newSfx(): id:%d buffer:%p, rolloff:%f, gain:%f %p\n", id, m_sfx_buffers[id], m_sfx_rolloff[id], m_sfx_gain[id], p);*/
    
    sfx->volume(m_master_gain);
    
    if (add_to_SFX_list) m_all_sfx.push_back(sfx);
    
    return sfx;
}   // createSoundSource

//----------------------------------------------------------------------------
SFXBase* SFXManager::createSoundSource(const std::string &name, 
                                       const bool addToSFXList)
{
    std::map<std::string, SFXBufferInfo>::iterator i = m_all_sfx_types.find(name);
    if ( i == m_all_sfx_types.end() )
    {
        fprintf( stderr, "SFXManager::createSoundSource could not find the requested sound effect : '%s'\n", name.c_str());
        
        /*
        std::map<std::string, SFXBufferInfo>::iterator it = m_all_sfx_types.begin();
        for (; it != m_all_sfx_types.end(); it++ )
        {
            printf("    got '%s'\n", it->first.c_str());
        }*/
        
        return NULL;
    }
    
    return createSoundSource( i->second, addToSFXList );
}  // createSoundSource

//----------------------------------------------------------------------------
/** Returns true if a sfx with the given name exists.
 *  \param name The internal name of the sfx (not the name of the ogg file)
 */
bool SFXManager::soundExist(const std::string &name)
{
    return m_all_sfx_types.find(name) != m_all_sfx_types.end();
}   // soundExist

//----------------------------------------------------------------------------
/** This function removes a sfx buffer info entry from the mapping, and
 *  frees the openal buffer.
 *  \param name The name of the mapping entry to remove.
 */
void SFXManager::deleteSFXMapping(const std::string &name)
{
    std::map<std::string, SFXBufferInfo>::iterator i;
    i = m_all_sfx_types.find(name);

    if(i==m_all_sfx_types.end())
    {
        fprintf(stderr, "SFXManager::deleteSFXMapping : Warning: sfx not found in list.\n");
        return;
    }
    (*i).second.freeBuffer();
    
    m_all_sfx_types.erase(i);

}   // deleteSFXMapping

//----------------------------------------------------------------------------
/** Delete a sound effect object, and removes it from the internal list of
 *  all SFXs. This call deletes the object, and removes it from the list of
 *  all SFXs.
 *  \param sfx SFX object to delete.
 */
void SFXManager::deleteSFX(SFXBase *sfx)
{
    if(sfx) sfx->stop();
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
    for (std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
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
    for (std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
        i!=m_all_sfx.end(); i++)
    {
        SFXStatus status = (*i)->getStatus();
        // Initial happens when 
        if (status==SFX_PAUSED) (*i)->resume();
    }   // for i in m_all_sfx
}   // resumeAll

//-----------------------------------------------------------------------------
/** Returns whether or not an openal error has occurred. If so, an error 
 *  message is printed containing the given context.
 *  \param context Context to specify in the error message.
 */
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
/** Sets the master volume for all sound effects.
 *  \param gain The volume to set.
 */
void SFXManager::setMasterSFXVolume(float gain)
{
    if (gain > 1.0)  gain = 1.0f;
    if (gain < 0.0f) gain = 0.0f;

    m_master_gain = gain;

    // regular SFX
    {
        for (std::vector<SFXBase*>::iterator i=m_all_sfx.begin();
            i!=m_all_sfx.end(); i++)
        {
            (*i)->volume(m_master_gain);
        }   // for i in m_all_sfx
    }
    
    // quick SFX
    {
        std::map<std::string, SFXBase*>::iterator i = m_quick_sounds.begin();
        for (; i != m_quick_sounds.end(); i++)
        {
            (*i).second->volume(m_master_gain);
        }
    }
    
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

void SFXManager::positionListener(const Vec3 &position, const Vec3 &front)
{
    if (!UserConfigParams::m_sfx || !m_initialized) return;
    
    //forward vector
    m_listenerVec[0] = front.getX(); 
    m_listenerVec[1] = front.getY();
    m_listenerVec[2] = front.getZ(); 
    
    //up vector
    m_listenerVec[3] = 0; 
    m_listenerVec[4] = 0;
    m_listenerVec[5] = 1;
    
    alListener3f(AL_POSITION, position.getX(), position.getY(), position.getZ());
    alListenerfv(AL_ORIENTATION, m_listenerVec);
}

//-----------------------------------------------------------------------------
/** Positional sound is cool, but creating a new object just to play a simple
 *  menu sound is not. This function allows for 'quick sounds' in a single call.
 *  \param sound_type Internal name of the sfx to play.
 */

void SFXManager::quickSound(const std::string &sound_type)
{
    if(!sfxAllowed()) return;
    std::map<std::string, SFXBase*>::iterator sound = m_quick_sounds.find(sound_type);
    
    if (sound == m_quick_sounds.end())
    {
        // sound not yet in our local list of quick sounds
        SFXBase* newSound = sfx_manager->createSoundSource(sound_type, false);
        newSound->play();
        m_quick_sounds[sound_type] = newSound;
    }
    else
    {
        (*sound).second->play();
    }
    
    //     m_locked_sound = sfx_manager->newSFX(SFXManager::SOUND_LOCKED);
    // m_locked_sound->play();
}


