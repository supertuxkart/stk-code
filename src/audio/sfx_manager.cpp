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

#include "audio/dummy_sfx.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_openal.hpp"
#include "audio/sfx_buffer.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/stk_process.hpp"
#include "utils/profiler.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <stdexcept>
#include <algorithm>
#include <map>

#include <functional>

#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <math.h>

// Define this if the profiler should also collect data of the sfx manager
#undef ENABLE_PROFILING_FOR_SFX_MANAGER
#ifndef ENABLE_PROFILING_FOR_SFX_MANAGER
     // Otherwise ignore the profiler push/pop events
     // Use undef to remove preprocessor warning
#    undef PROFILER_PUSH_CPU_MARKER
#    undef  PROFILER_POP_CPU_MARKER
#    define PROFILER_PUSH_CPU_MARKER(name, r, g, b)
#    define PROFILER_POP_CPU_MARKER()
#endif

SFXManager *SFXManager::m_sfx_manager;

// ----------------------------------------------------------------------------
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
    m_last_update_time = std::numeric_limits<uint64_t>::max();
    // Init position, since it can be used before positionListener is called.
    // No need to use lock here, since the thread will be created later.
    m_listener_position.getData() = Vec3(0, 0, 0);
    m_listener_front              = Vec3(0, 0, 1);
    m_listener_up                 = Vec3(0, 1, 0);

    loadSfx();

#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
        // The thread is created even if there atm sfx are disabled
        // (since the user might enable it later).
#ifndef __SWITCH__
        m_thread = std::thread(std::bind(mainLoop, this));
#endif
        setMasterSFXVolume( UserConfigParams::m_sfx_volume );
        m_sfx_commands.lock();
        m_sfx_commands.getData().clear();
        m_sfx_commands.unlock();
    }
#endif
}  // SoundManager

//-----------------------------------------------------------------------------
/** Destructor, frees all sound effects.
 */
SFXManager::~SFXManager()
{
#if defined(ENABLE_SOUND) && !defined(__SWITCH__)
    if (UserConfigParams::m_enable_sound)
    {
        m_thread.join();
    }
#endif

    // ---- clear m_all_sfx
    // not strictly necessary, but might avoid copy&paste problems
    m_all_sfx.lock();
    const int sfx_amount = (int) m_all_sfx.getData().size();
    for (int n=0; n<sfx_amount; n++)
    {
        delete m_all_sfx.getData()[n];
    }
    m_all_sfx.getData().clear();
    m_all_sfx.unlock();

    m_quick_sounds.lock();
    // ---- clear m_quick_sounds
    {
        std::map<std::string, SFXBase*>::iterator i = m_quick_sounds.getData().begin();
        for (; i != m_quick_sounds.getData().end(); i++)
        {
            SFXBase* snd = (*i).second;
            delete snd;
        }
    }
    m_quick_sounds.getData().clear();
    m_quick_sounds.unlock();

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
/** Adds a sound effect command to the queue of the sfx manager. Openal 
 *  commands can sometimes cause a 5ms delay, so it is done in a separate 
 *  thread.
 *  \param command The command to execute.
 *  \param sfx The sound effect to be started.
 */
void SFXManager::queue(SFXCommands command,  SFXBase *sfx)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, sfx);
    queueCommand(sfx_command);
#endif
}   // queue

//----------------------------------------------------------------------------
/** Adds a sound effect command with a single floating point parameter to the
 *  queue of the sfx manager. Openal commands can sometimes cause a 5ms delay,
 *  so it is done in a separate thread.
 *  \param command The command to execute.
 *  \param sfx The sound effect to be started.
 *  \param f Floating point parameter for the command.
 */
void SFXManager::queue(SFXCommands command, SFXBase *sfx, float f)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, sfx, f);
    queueCommand(sfx_command);
#endif
}   // queue(float)

//----------------------------------------------------------------------------
/** Adds a sound effect command with a Vec3 parameter to the queue of the sfx
 *  manager. Openal commands can sometimes cause a 5ms delay, so it is done in
 *  a separate thread.
 *  \param command The command to execute.
 *  \param sfx The sound effect to be started.
 *  \param p A Vec3 parameter for the command.
 */
void SFXManager::queue(SFXCommands command, SFXBase *sfx, const Vec3 &p)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, sfx, p);
    queueCommand(sfx_command);
#endif
}   // queue (Vec3)

//----------------------------------------------------------------------------

void SFXManager::queue(SFXCommands command, SFXBase *sfx, const Vec3 &p, SFXBuffer* buffer)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, sfx, p);
    sfx_command->m_buffer = buffer;
    queueCommand(sfx_command);
#endif
}   // queue (Vec3)

//----------------------------------------------------------------------------
/** Adds a sound effect command with a float and a Vec3 parameter to the queue
 *  of the sfx manager. Openal commands can sometimes cause a 5ms delay, so it
 *   is done in a separate thread.
 *  \param command The command to execute.
 *  \param sfx The sound effect to be started.
 *  \param f A float parameter for the command.
 *  \param p A Vec3 parameter for the command.
 */
void SFXManager::queue(SFXCommands command, SFXBase *sfx, float f,
                       const Vec3 &p)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, sfx, f, p);
    queueCommand(sfx_command);
#endif
}   // queue(float, Vec3)

//----------------------------------------------------------------------------
/** Queues a command for the music manager.
 *  \param mi The music for which the command is.
 */
void SFXManager::queue(SFXCommands command, MusicInformation *mi)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, mi);
    queueCommand(sfx_command);
#endif
}   // queue(MusicInformation)
//----------------------------------------------------------------------------
/** Queues a command for the music manager that takes a floating point value
 *  (e.g. setTemporaryVolume).
 *  \param mi The music for which the command is.
 *  \param f The floating point parameter.
 */
void SFXManager::queue(SFXCommands command, MusicInformation *mi, float f)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;

    SFXCommand *sfx_command = new SFXCommand(command, mi, f);
    queueCommand(sfx_command);
#endif
}   // queue(MusicInformation)

//----------------------------------------------------------------------------
/** Enqueues a command to the sfx queue threadsafe. Then signal the
 *  sfx manager to wake up.
 *  \param command Pointer to the command to queue up.
 */
void SFXManager::queueCommand(SFXCommand *command)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound || STKProcess::getType() != PT_MAIN)
        return;
        
    m_sfx_commands.lock();
    if (StateManager::get()->getGameState() != GUIEngine::MENU &&
        m_sfx_commands.getData().size() > 20*RaceManager::get()->getNumberOfKarts()+20 &&
        RaceManager::get()->getMinorMode() != RaceManager::MINOR_MODE_CUTSCENE)
    {
        if(command->m_command==SFX_POSITION || command->m_command==SFX_LOOP ||
           command->m_command==SFX_SPEED    || 
           command->m_command==SFX_SPEED_POSITION                               )
        {
            delete command;
            static int count_messages = 0;
            if(count_messages < 5)
            {
                Log::warn("SFXManager", "Throttling sfx - queue size %d",
                         m_sfx_commands.getData().size());
                count_messages++;
            }
            m_sfx_commands.unlock();
            return;
        }   // if throttling
    }
    m_sfx_commands.getData().push_back(command);
    m_sfx_commands.unlock();
#endif
}   // queueCommand

//----------------------------------------------------------------------------
/** Puts a NULL request into the queue, which will trigger the thread to
 *  exit.
 */
void SFXManager::stopThread()
{
#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
        queue(SFX_EXIT);
        // Make sure the thread wakes up.
        m_condition_variable.notify_one();
    }
    else
#endif
    {
        setCanBeDeleted();
    }
}   // stopThread

//----------------------------------------------------------------------------
/** This loops runs in a different threads, and starts sfx to be played.
 *  This can sometimes take up to 5 ms, so it needs to be handled in a thread
 *  in order to avoid rendering delays.
 *  \param obj A pointer to the SFX singleton.
 */
void SFXManager::mainLoop(void *obj)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound)
        return;
        
#ifndef __SWITCH__
    VS::setThreadName("SFXManager");
#endif
    SFXManager *me = (SFXManager*)obj;

    std::unique_lock<std::mutex> ul = me->m_sfx_commands.acquireMutex();

#ifdef __SWITCH__
    int iterCount = 0;
#endif
    // Wait till we have an empty sfx in the queue
    while (
#ifdef __SWITCH__
           // Don't spend too much time working on audio
           ++iterCount != 30 && !me->m_sfx_commands.getData().empty()
#else
           me->m_sfx_commands.getData().empty() || me->m_sfx_commands.getData().front()->m_command!=SFX_EXIT
#endif
    )
    {
        PROFILER_PUSH_CPU_MARKER("Wait", 255, 0, 0);
        bool empty = me->m_sfx_commands.getData().empty();

        // Wait in cond_wait for a request to arrive. The 'while' is necessary
        // since "spurious wakeups from the pthread_cond_wait ... may occur"
        // (pthread_cond_wait man page)!
#ifndef __SWITCH__
        while (empty)
        {
            me->m_condition_variable.wait(ul);
            empty = me->m_sfx_commands.getData().empty();
        }
#endif
        SFXCommand *current = me->m_sfx_commands.getData().front();
        me->m_sfx_commands.getData().erase(me->m_sfx_commands.getData().begin());

        if (current->m_command == SFX_EXIT)
        {
            delete current;
#ifdef __SWITCH__
            return;
#else
            break;
#endif
        }
        bool empty_queue = me->m_sfx_commands.getData().empty();
        ul.unlock();
        PROFILER_POP_CPU_MARKER();
        PROFILER_PUSH_CPU_MARKER("Execute", 0, 255, 0);
        switch (current->m_command)
        {
        case SFX_PLAY:     current->m_sfx->reallyPlayNow();       break;
        case SFX_PLAY_POSITION:
            current->m_sfx->reallyPlayNow(current->m_parameter, current->m_buffer);  break;
        case SFX_STOP:     current->m_sfx->reallyStopNow();       break;
        case SFX_PAUSE:    current->m_sfx->reallyPauseNow();      break;
        case SFX_RESUME:   current->m_sfx->reallyResumeNow();     break;
        case SFX_SPEED:    current->m_sfx->reallySetSpeed(
                                  current->m_parameter.getX());   break;
        case SFX_POSITION: current->m_sfx->reallySetPosition(
                                         current->m_parameter);   break;
        case SFX_SPEED_POSITION: current->m_sfx->reallySetSpeedPosition(
                                         // Extract float from W component
                                         current->m_parameter.getW(),
                                         current->m_parameter);   break;
        case SFX_VOLUME:   current->m_sfx->reallySetVolume(
                                  current->m_parameter.getX());   break;
        case SFX_MASTER_VOLUME:
            current->m_sfx->reallySetMasterVolumeNow(
                                  current->m_parameter.getX());   break;
        case SFX_LOOP:     current->m_sfx->reallySetLoop(
                             current->m_parameter.getX() != 0);   break;
        case SFX_DELETE:     me->deleteSFX(current->m_sfx);       break;
        case SFX_PAUSE_ALL:  me->reallyPauseAllNow();             break;
        case SFX_RESUME_ALL: me->reallyResumeAllNow();            break;
        case SFX_LISTENER:   me->reallyPositionListenerNow();     break;
        case SFX_UPDATE:     me->reallyUpdateNow(current);        break;
        case SFX_MUSIC_START:
        {
            if (!current->m_music_information->preStart())
                break;
            current->m_music_information->setDefaultVolume();
            current->m_music_information->startMusic();           break;
        }
        case SFX_MUSIC_STOP:
            current->m_music_information->stopMusic();            break;
        case SFX_MUSIC_PAUSE:
            current->m_music_information->pauseMusic();           break;
        case SFX_MUSIC_RESUME:
            current->m_music_information->resumeMusic();
            // This might be necessasary if the volume was changed
            // in the in-game menu
            current->m_music_information->setDefaultVolume();     break;
        case SFX_MUSIC_SWITCH_FAST:
            current->m_music_information->switchToFastMusic();    break;
        case SFX_MUSIC_SET_TMP_VOLUME:
        {
            MusicInformation *mi = current->m_music_information;
            mi->setTemporaryVolume(current->m_parameter.getX());  break;
        }
        case SFX_MUSIC_WAITING:
               current->m_music_information->preStart();
               current->m_music_information->setMusicWaiting();   break;
        case SFX_MUSIC_DEFAULT_VOLUME:
        {
            current->m_music_information->setDefaultVolume();
            break;
        }
        case SFX_CREATE_SOURCE:
            current->m_sfx->init(); break;
        default: assert("Not yet supported.");
        }
        delete current;
        current = NULL;
        PROFILER_POP_CPU_MARKER();
        PROFILER_PUSH_CPU_MARKER("yield", 0, 0, 255);
        if (empty_queue && me->sfxAllowed())
        {
            // Wait some time to let other threads run, then queue an
            // update event to keep music playing.
            uint64_t t = StkTime::getMonoTimeMs();
            StkTime::sleep(1);
            t = StkTime::getMonoTimeMs() - t;
            me->queue(SFX_UPDATE, (SFXBase*)NULL, float(t / 1000.0));
        }
        ul = me->m_sfx_commands.acquireMutex();
        PROFILER_POP_CPU_MARKER();
    }   // while

    // Signal that the sfx manager can now be deleted.
    // We signal this even before cleaning up memory, since there is no
    // need to keep the user waiting for STK to exit.
    me->setCanBeDeleted();

#ifndef __SWITCH__
    // Clean up memory to avoid leak detection
    while(!me->m_sfx_commands.getData().empty())
    {
        delete me->m_sfx_commands.getData().front();
        me->m_sfx_commands.getData().erase(me->m_sfx_commands.getData().begin());
    }
#endif // __SWITCH__
#endif // ENABLE_SOUD
    return;
}   // mainLoop

//----------------------------------------------------------------------------
/** Called when sound is globally switched on or off. It either pauses or
 *  resumes all sound effects. 
 *  \param on If sound is switched on or off.
 */
void SFXManager::toggleSound(const bool on)
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

        reallyResumeAllNow();
        m_all_sfx.lock();
        const int sfx_amount = (int)m_all_sfx.getData().size();
        for (int n=0; n<sfx_amount; n++)
        {
            m_all_sfx.getData()[n]->onSoundEnabledBack();
        }
        m_all_sfx.unlock();
    }
    else
    {
        // First stop all sfx that are not looped
        const int sfx_amount = (int)m_all_sfx.getData().size();
        m_all_sfx.lock();
        for (int i=0; i<sfx_amount; i++)
        {
            if(!m_all_sfx.getData()[i]->isLooped())
            {
                m_all_sfx.getData()[i]->reallyStopNow();
            }
        }
        m_all_sfx.unlock();
        pauseAll();
    }
}   // toggleSound

//----------------------------------------------------------------------------
/** Returns if sfx can be played. This means sfx are enabled and
 *  the manager is correctly initialised.
 */
bool SFXManager::sfxAllowed()
{
    if (STKProcess::getType() != PT_MAIN)
        return false;
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
    const int max = (int)m_all_sfx_types.size();
    SFXBuffer **array = new SFXBuffer *[max];
    i = 0;

    for (std::map<std::string, SFXBuffer*>::iterator it = m_all_sfx_types.begin();
         it != m_all_sfx_types.end(); it++)
    {
        SFXBuffer* const buffer = (*it).second;
        array[i++] = buffer;
    }

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
                                    float              max_dist,
                                    float              gain,
                                    const bool         load)
{

    SFXBuffer* buffer = new SFXBuffer(sfx_file, positional, rolloff, 
                                      max_dist, gain);

    m_all_sfx_types[sfx_name] = buffer;

    if (!m_initialized)
    {
        // Keep the buffer even if SFX is disabled, in case
        // SFX is enabled back later
        return NULL;
    }

    if (UserConfigParams::logMisc())
        Log::debug("SFXManager", "Loading SFX %s", sfx_file.c_str());

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
                "The 'filename' attribute is mandatory in the SFX XML file!");
        return NULL;
    }

    std::string sfx_name = StringUtils::removeExtension(filename);
    
    if(m_all_sfx_types.find(sfx_name)!=m_all_sfx_types.end())
    {
        Log::error("SFXManager",
                "There is already a sfx named '%s' installed - new one is ignored.",
                sfx_name.c_str());
        return NULL;
    }

    // Only use the filename if no full path is specified. This is used
    // to load terrain specific sfx.
    const std::string full_path = (path == "")
                                ? file_manager->getAsset(FileManager::SFX,filename)
                                : path+"/"+filename;

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

    if (RaceManager::get()->getNumLocalPlayers() < 2)
    {
        positional = buffer->isPositional();
    }

    SFXBase* sfx = NULL;
    
#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound && STKProcess::getType() == PT_MAIN)
    {
        //assert( alIsBuffer(buffer->getBufferID()) ); crashes on server
        sfx = new SFXOpenAL(buffer, positional, buffer->getGain(), owns_buffer);
    }
    else
#endif
    {
        sfx = new DummySFX(buffer, positional, buffer->getGain());
        
        if (owns_buffer)
        {
            delete buffer;
        }
    }

    sfx->setMasterVolume(m_master_gain);

    if (add_to_SFX_list) 
    {
        m_all_sfx.lock();
        m_all_sfx.getData().push_back(sfx);
        m_all_sfx.unlock();
    }

    return sfx;
}   // createSoundSource

//----------------------------------------------------------------------------
SFXBase* SFXManager::createSoundSource(const std::string &name,
                                       const bool add_to_SFXList)
{
    std::map<std::string, SFXBuffer*>::iterator i = m_all_sfx_types.find(name);
    if ( i == m_all_sfx_types.end() )
    {
        Log::error("SFXManager", 
                   "SFXManager::createSoundSource could not find the "
                   "requested sound effect : '%s'.", name.c_str());
        return NULL;
    }

    return createSoundSource( i->second, add_to_SFXList );
}  // createSoundSource

//----------------------------------------------------------------------------

SFXBuffer* SFXManager::getBuffer(const std::string &name)
{
    std::map<std::string, SFXBuffer*>::iterator i = m_all_sfx_types.find(name);
    if (i == m_all_sfx_types.end())
    {
        Log::error("SFXManager",
            "SFXManager::getBuffer could not find the "
            "requested sound effect : '%s'.", name.c_str());
        return NULL;
    }

    return i->second;
}

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
        Log::warn("SFXManager",
             "SFXManager::deleteSFXMapping : Warning: sfx not found in list.");
        return;
    }
    (*i).second->unload();

    m_all_sfx_types.erase(i);

}   // deleteSFXMapping

//----------------------------------------------------------------------------
/** Make sure that the sfx thread is started at least once per frame. It also
 *  adds an update command for the music manager.
 *  \param dt Time step size.
 */
void SFXManager::update()
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound)
        return;

    queue(SFX_UPDATE, (SFXBase*)NULL);
    // Wake up the sfx thread to handle all queued up audio commands.
    m_condition_variable.notify_one();

#ifdef __SWITCH__
    mainLoop(this);
#endif
#endif
}   // update

//----------------------------------------------------------------------------
/** Updates the status of all playing sfx (to test if they are finished).
 *  This function is executed once per frame (triggered by the audio thread).
 *  \param current The sfx command - used to get timestep information.
*/
void SFXManager::reallyUpdateNow(SFXCommand *current)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound)
        return;

    if (m_last_update_time == std::numeric_limits<uint64_t>::max())
    {
        // first time
        m_last_update_time = StkTime::getMonoTimeMs();
    }

    uint64_t previous_update_time = m_last_update_time;
    m_last_update_time = StkTime::getMonoTimeMs();
    float dt = float(m_last_update_time - previous_update_time) / 1000.0f;

    assert(current->m_command==SFX_UPDATE);
    if (music_manager->getCurrentMusic())
        music_manager->getCurrentMusic()->update(dt);
    m_all_sfx.lock();
    for (std::vector<SFXBase*>::iterator i =  m_all_sfx.getData().begin();
                                         i != m_all_sfx.getData().end(); i++)
    {
        if((*i)->getStatus()==SFXBase::SFX_PLAYING)
            (*i)->updatePlayingSFX(dt);
    }   // for i in m_all_sfx
    m_all_sfx.unlock();

    // We need to lock the quick sounds during update, since adding more
    // quick sounds by another thread could invalidate the iterator.
    m_quick_sounds.lock();
    std::map<std::string, SFXBase*>::iterator i = m_quick_sounds.getData().begin();
    for (; i != m_quick_sounds.getData().end(); i++)
    {
        if (i->second->getStatus() == SFXBase::SFX_PLAYING)
            i->second->updatePlayingSFX(dt);
    }   // for i in m_all_sfx
    m_quick_sounds.unlock();
#endif
}   // reallyUpdateNow

//----------------------------------------------------------------------------
/** Delete a sound effect object, and removes it from the internal list of
 *  all SFXs. This call deletes the object, and removes it from the list of
 *  all SFXs.
 *  \param sfx SFX object to delete.
 */
void SFXManager::deleteSFX(SFXBase *sfx)
{
    if(sfx) sfx->reallyStopNow();
    std::vector<SFXBase*>::iterator i;
    
    // The whole block needs to be locked, otherwise the iterator
    // could become invalid.
    m_all_sfx.lock();
    i=std::find(m_all_sfx.getData().begin(), m_all_sfx.getData().end(), sfx);

    if(i==m_all_sfx.getData().end())
    {
        Log::warn("SFXManager", 
                  "SFXManager::deleteSFX : Warning: sfx '%s' %lx not found in list.",
                  sfx->getBuffer()->getFileName().c_str(), sfx);
        m_all_sfx.unlock();
        return;
    }

    m_all_sfx.getData().erase(i);
    m_all_sfx.unlock();

    delete sfx;
}   // deleteSFX

//----------------------------------------------------------------------------
/** Pauses all looping SFXs. Non-looping SFX will be finished, since it's
 *  otherwise not possible to determine which SFX must be resumed (i.e. were
 *  actually playing at the time pause was called).
 */
void SFXManager::pauseAll()
{
    if (!sfxAllowed()) return;
    queue(SFX_PAUSE_ALL);
}   // pauseAll

//----------------------------------------------------------------------------
/** Pauses all looping SFXs. Non-looping SFX will be finished, since it's
 *  otherwise not possible to determine which SFX must be resumed (i.e. were
 *  actually playing at the time pause was called.
 */
void SFXManager::reallyPauseAllNow()
{
    m_all_sfx.lock();
    for (std::vector<SFXBase*>::iterator i= m_all_sfx.getData().begin();
                                         i!=m_all_sfx.getData().end(); i++)
    {
        (*i)->reallyPauseNow();
    }   // for i in m_all_sfx
    m_all_sfx.unlock();
}   // pauseAll

//----------------------------------------------------------------------------
/** Resumes all paused SFXs. If sound is disabled, does nothing.
  */
void SFXManager::resumeAll()
{
    // ignore unpausing if sound is disabled
    if (!sfxAllowed()) return;
    queue(SFX_RESUME_ALL);
}   // resumeAll

//----------------------------------------------------------------------------
/** Resumes all paused SFXs. If sound is disabled, does nothing.
  */
void SFXManager::reallyResumeAllNow()
{
    m_all_sfx.lock();
    for (std::vector<SFXBase*>::iterator i =m_all_sfx.getData().begin();
                                         i!=m_all_sfx.getData().end(); i++)
    {
        // Some buffer not added to m_all_sfx_types need to be loaded here
        // For example sound for entering challenge house in overworld
        if ((*i)->getBuffer())
            (*i)->getBuffer()->load();
        (*i)->reallyResumeNow();
    }   // for i in m_all_sfx
    m_all_sfx.unlock();
}   // resumeAll

//-----------------------------------------------------------------------------
/** Returns whether or not an openal error has occurred. If so, an error
 *  message is printed containing the given context.
 *  \param context Context to specify in the error message.
 *  \return True if no error happened.
 */
bool SFXManager::checkError(const std::string &context)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound)
        return true;

    // Check (and clear) the error flag
    int error = alGetError();

    if (error != AL_NO_ERROR)
    {
        Log::error("SFXManager", "SFXOpenAL OpenAL error while %s: %s",
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
        m_all_sfx.lock();
        for (std::vector<SFXBase*>::iterator i =m_all_sfx.getData().begin();
                                             i!=m_all_sfx.getData().end(); i++)
        {
            (*i)->setMasterVolume(m_master_gain);
        }   // for i in m_all_sfx
        m_all_sfx.unlock();
    }

    // quick SFX
    {
        m_quick_sounds.lock();
        std::map<std::string, SFXBase*>::iterator i = m_quick_sounds.getData().begin();
        for (; i != m_quick_sounds.getData().end(); i++)
        {
            (*i).second->setMasterVolume(m_master_gain);
        }
        m_quick_sounds.unlock();
    }

}   // setMasterSFXVolume

//-----------------------------------------------------------------------------
const std::string SFXManager::getErrorString(int err)
{
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound)
        return std::string("sound disabled");

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
#endif

    return std::string("sound disabled");
}   // getErrorString

//-----------------------------------------------------------------------------
/** Sets the position and orientation of the listener.
 *  \param position Position of the listener.
 *  \param front Which way the listener is facing.
 *  \param up The up direction of the listener.
 */
void SFXManager::positionListener(const Vec3 &position, const Vec3 &front,
                                  const Vec3 &up)
{
    m_listener_position.lock();
    m_listener_position.getData() = position;
    m_listener_front              = front;
    m_listener_up                 = up;
    m_listener_position.unlock();
    queue(SFX_LISTENER);
}   // positionListener

//-----------------------------------------------------------------------------
/** Sets the position and orientation of the listener.
 *  \param position Position of the listener.
 *  \param front Which way the listener is facing.
 */
void SFXManager::reallyPositionListenerNow()
{
#ifdef ENABLE_SOUND
    if (!sfxAllowed()) return;

    m_listener_position.lock();
    {

        //forward vector
        float orientation[6];
        orientation[0] = m_listener_front.getX();
        orientation[1] = m_listener_front.getY();
        orientation[2] = -m_listener_front.getZ();

        //up vector
        orientation[3] = m_listener_up.getX();
        orientation[4] = m_listener_up.getY();
        orientation[5] = -m_listener_up.getZ();

        const Vec3 &pos = m_listener_position.getData();
        alListener3f(AL_POSITION, pos.getX(), pos.getY(), -pos.getZ());
        alListenerfv(AL_ORIENTATION, orientation);
    }
    m_listener_position.unlock();

#endif
}   // reallyPositionListenerNow

//-----------------------------------------------------------------------------
/** Positional sound is cool, but creating a new object just to play a simple
 *  menu sound is not. This function allows for 'quick sounds' in a single call.
 *  \param sound_type Internal name of the sfx to play.
 *  \return a pointer to the sound, for instance to check when the sound finished.
 *          don't delete the returned pointer.
 */
SFXBase* SFXManager::quickSound(const std::string &sound_type)
{
#ifdef ENABLE_SOUND
    if (!sfxAllowed()) return NULL;

    std::unique_lock<std::mutex> ul = m_quick_sounds.acquireMutex();
    std::map<std::string, SFXBase*>::iterator sound = 
                                     m_quick_sounds.getData().find(sound_type);

    if (sound == m_quick_sounds.getData().end())
    {
        // sound not yet in our local list of quick sounds
        SFXBase* new_sound = createSoundSource(sound_type, false);
        if (new_sound == NULL) return NULL;
        new_sound->play();
        m_quick_sounds.getData()[sound_type] = new_sound;
        return new_sound;
    }
    else
    {
        SFXBase *base_sound = sound->second;
        if (base_sound->getStatus() != SFXBase::SFX_PLAYING)
            base_sound->play();
        return base_sound;
    }
#else
    return NULL;
#endif
}   // quickSound

