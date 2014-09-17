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

#include "audio/dummy_sfx.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_buffer.hpp"
#include "io/file_manager.hpp"

#include <stdexcept>
#include <algorithm>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if HAVE_OGGVORBIS
#  ifdef __APPLE__
#    include <OpenAL/al.h>
#    include <OpenAL/alc.h>
#  else
#    include <AL/al.h>
#    include <AL/alc.h>
#  endif
#endif

#include "audio/sfx_openal.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/constants.hpp"

std::map<std::string, SFXBase*> SFXManager::m_quick_sounds;
SFXManager *SFXManager::m_sfx_manager;

/** Static function to create the singleton sfx manager.
 */
void SFXManager::create()
{
    assert(!m_sfx_manager);
    m_sfx_manager = new SFXManager();
}   // create

// ------------------------------------------------------------------------
/** Static function to delete the singleton sfx manager.
 */
void SFXManager::destroy()
{
    assert(m_sfx_manager);
    delete m_sfx_manager;
    m_sfx_manager = NULL;
}    // destroy

// ----------------------------------------------------------------------------
/** Initialises the SFX manager and loads the sfx from a config file.
 */
SFXManager::SFXManager()
{
    // The sound manager initialises OpenAL
    m_initialized = music_manager->initialized();
    m_master_gain = UserConfigParams::m_sfx_volume;
    // Init position, since it can be used before positionListener is called.
    m_position    = Vec3(0,0,0);

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
        std::map<std::string, SFXBuffer*>::iterator i = m_all_sfx_types.begin();
        for (; i != m_all_sfx_types.end(); i++)
        {
            SFXBuffer* buffer = (*i).second;
            buffer->unload();
            delete buffer;
        }
        m_all_sfx_types.clear();
    }
    m_all_sfx_types.clear();

}   // ~SFXManager

//----------------------------------------------------------------------------

void SFXManager::soundToggled(const bool on)
{
    // When activating SFX, load all buffers
    if (on)
    {
        std::map<std::string, SFXBuffer*>::iterator i = m_all_sfx_types.begin();
        for (; i != m_all_sfx_types.end(); i++)
        {
            SFXBuffer* buffer = (*i).second;
            buffer->load();
        }

        resumeAll();

        const int sfx_amount = m_all_sfx.size();
        for (int n=0; n<sfx_amount; n++)
        {
            m_all_sfx[n]->onSoundEnabledBack();
        }
    }
    else
    {
        pauseAll();
    }
}

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
    std::string sfx_config_name = file_manager->getAsset(FileManager::SFX, "sfx.xml");
    XMLNode* root = file_manager->createXMLTree(sfx_config_name);
    if (!root || root->getName()!="sfx-config")
    {
        Log::fatal("SFXManager", "Could not read sound effects XML file '%s'.",
                   sfx_config_name.c_str());
    }

    int i;

    const int amount = root->getNumNodes();
    for (i=0; i<amount; i++)
    {
        const XMLNode* node = root->getNode(i);

        if (node->getName() == "sfx")
        {
            loadSingleSfx(node, "", false);
        }
        else
        {
            Log::warn("SFXManager", "Unknown node '%s' in sfx XML file '%s'.",
                      node->getName().c_str(), sfx_config_name.c_str());
            throw std::runtime_error("Unknown node in sfx XML file");
        }
    }// nend for

    delete root;

    // Now load them in parallel
    const int max = m_all_sfx_types.size();
    SFXBuffer **array = new SFXBuffer *[max];
    i = 0;

    for (std::map<std::string, SFXBuffer*>::iterator it = m_all_sfx_types.begin();
         it != m_all_sfx_types.end(); it++)
    {
        SFXBuffer* const buffer = (*it).second;
        array[i++] = buffer;
    }

    #pragma omp parallel for private(i)
    for (i = 0; i < max; i++)
    {
        array[i]->load();
    }

    delete [] array;
}   // loadSfx

// -----------------------------------------------------------------------------
/** Introduces a mechanism by which one can load sound effects beyond the basic
 *  enumerated types.  This will be used when loading custom sound effects for
 *  individual karts (character voices) so that we can avoid creating an
 *  enumeration for each effect, for each kart.
 *  \param sfx_name
 *  \param sfxFile must be an absolute pathname
 *  \return        whether loading this sound effect was successful

*/
SFXBuffer* SFXManager::addSingleSfx(const std::string &sfx_name,
                                    const std::string &sfx_file,
                                    bool               positional,
                                    float              rolloff,
                                    float              max_width,
                                    float              gain,
                                    const bool         load)
{

    SFXBuffer* buffer = new SFXBuffer(sfx_file, positional, rolloff, max_width, gain);

    m_all_sfx_types[sfx_name] = buffer;

    if (!m_initialized)
    {
        // Keep the buffer even if SFX is disabled, in case
        // SFX is enabled back later
        return NULL;
    }

    if (UserConfigParams::logMisc())
        Log::debug("SFXManager", "Loading SFX %s\n", sfx_file.c_str());

    if (load && buffer->load()) return buffer;

    return NULL;
} // addSingleSFX

//----------------------------------------------------------------------------
/** Loads a single sfx from the XML specification.
 *  \param node The XML node with the data for this sfx.
 */
SFXBuffer* SFXManager::loadSingleSfx(const XMLNode* node,
                                     const std::string &path,
                                     const bool load)
{
    std::string filename;

    if (node->get("filename", &filename) == 0)
    {
        Log::error("SFXManager",
                "/!\\ The 'filename' attribute is mandatory in the SFX XML file!\n");
        return NULL;
    }

    std::string sfx_name = StringUtils::removeExtension(filename);
    
    if(m_all_sfx_types.find(sfx_name)!=m_all_sfx_types.end())
    {
        Log::error("SFXManager",
                "There is already a sfx named '%s' installed - new one is ignored.\n",
                sfx_name.c_str());
        return NULL;
    }

    // Only use the filename if no full path is specified. This is used
    // to load terrain specific sfx.
    const std::string full_path = (path == "")
                                ? file_manager->getAsset(FileManager::SFX,filename)
                                : path;

    SFXBuffer tmpbuffer(full_path, node);

    return addSingleSfx(sfx_name, full_path,
                        tmpbuffer.isPositional(),
                        tmpbuffer.getRolloff(),
                        tmpbuffer.getMaxDist(),
                        tmpbuffer.getGain(),
                        load);

}   // loadSingleSfx

//----------------------------------------------------------------------------
/** Creates a new SFX object. The memory for this object is managed completely
 *  by the SFXManager. This makes it easy to use different implementations of
 *  SFX - since createSoundSource can return whatever type is used. To free the memory,
 *  call deleteSFX().
 *  \param id Identifier of the sound effect to create.
 */
SFXBase* SFXManager::createSoundSource(SFXBuffer* buffer,
                                       const bool add_to_SFX_list,
                                       const bool owns_buffer)
{
    bool positional = false;

    if (race_manager->getNumLocalPlayers() < 2)
    {
        positional = buffer->isPositional();
    }

    //printf("CREATING %s (%x), positional = %i (race_manager->getNumLocalPlayers() = %i, buffer->isPositional() = %i)\n",
    //       buffer->getFileName().c_str(), (unsigned int)buffer,
    //       positional,
    //       race_manager->getNumLocalPlayers(), buffer->isPositional());

#if HAVE_OGGVORBIS
    //assert( alIsBuffer(buffer->getBufferID()) ); crashes on server
    SFXBase* sfx = new SFXOpenAL(buffer, positional, buffer->getGain(), owns_buffer);
#else
    SFXBase* sfx = new DummySFX(buffer, positional, buffer->getGain(), owns_buffer);
#endif

    sfx->masterVolume(m_master_gain);

    if (add_to_SFX_list) m_all_sfx.push_back(sfx);

    return sfx;
}   // createSoundSource

//----------------------------------------------------------------------------
SFXBase* SFXManager::createSoundSource(const std::string &name,
                                       const bool addToSFXList)
{
    std::map<std::string, SFXBuffer*>::iterator i = m_all_sfx_types.find(name);
    if ( i == m_all_sfx_types.end() )
    {
        Log::error("SFXManager", "SFXManager::createSoundSource could not find the requested sound effect : '%s'\n", name.c_str());
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
    std::map<std::string, SFXBuffer*>::iterator i;
    i = m_all_sfx_types.find(name);

    if (i == m_all_sfx_types.end())
    {
        Log::warn("SFXManager", "SFXManager::deleteSFXMapping : Warning: sfx not found in list.\n");
        return;
    }
    (*i).second->unload();

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
        Log::warn("SFXManager", "SFXManager::deleteSFX : Warning: sfx not found in list.\n");
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
/**
  * Resumes all paused SFXs. If sound is disabled, does nothing.
  */
void SFXManager::resumeAll()
{
    // ignore unpausing if sound is disabled
    if (!sfxAllowed()) return;

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
#if HAVE_OGGVORBIS
    // Check (and clear) the error flag
    int error = alGetError();

    if(error != AL_NO_ERROR)
    {
        Log::error("SFXManager", "SFXOpenAL OpenAL error while %s: %s\n",
            context.c_str(), SFXManager::getErrorString(error).c_str());
        return false;
    }
#endif
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
            (*i)->masterVolume(m_master_gain);
        }   // for i in m_all_sfx
    }

    // quick SFX
    {
        std::map<std::string, SFXBase*>::iterator i = m_quick_sounds.begin();
        for (; i != m_quick_sounds.end(); i++)
        {
            (*i).second->masterVolume(m_master_gain);
        }
    }

}   // setMasterSFXVolume

//-----------------------------------------------------------------------------
const std::string SFXManager::getErrorString(int err)
{
#if HAVE_OGGVORBIS
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
#else
    return std::string("sound disabled");
#endif
}   // getErrorString

//-----------------------------------------------------------------------------

void SFXManager::positionListener(const Vec3 &position, const Vec3 &front)
{
#if HAVE_OGGVORBIS
    if (!UserConfigParams::m_sfx || !m_initialized) return;

    m_position = position;

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
#endif
}

//-----------------------------------------------------------------------------
/** Positional sound is cool, but creating a new object just to play a simple
 *  menu sound is not. This function allows for 'quick sounds' in a single call.
 *  \param sound_type Internal name of the sfx to play.
 *  \return a pointer to the sound, for instance to check when the sound finished.
 *          don't delete the returned pointer.
 */
SFXBase* SFXManager::quickSound(const std::string &sound_type)
{
    if (!sfxAllowed()) return NULL;
    std::map<std::string, SFXBase*>::iterator sound = m_quick_sounds.find(sound_type);

    if (sound == m_quick_sounds.end())
    {
        // sound not yet in our local list of quick sounds
        SFXBase* newSound = SFXManager::get()->createSoundSource(sound_type, false);
        if (newSound == NULL) return NULL;
        newSound->play();
        m_quick_sounds[sound_type] = newSound;
        return newSound;
    }
    else
    {
        (*sound).second->play();
        return (*sound).second;
    }
}   // quickSound


