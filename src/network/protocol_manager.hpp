//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

/*! \file protocol_manager.hpp
 *  \brief Contains structures and enumerations related to protocol management.
 */

#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include "network/event.hpp"
#include "network/network_string.hpp"
#include "network/protocol.hpp"
#include "utils/singleton.hpp"
#include "utils/types.hpp"

#include <vector>

#define TIME_TO_KEEP_EVENTS 1.0

/*!
 * \enum PROTOCOL_STATE
 * \brief Defines the three states that a protocol can have.
 */
enum PROTOCOL_STATE
{
    PROTOCOL_STATE_RUNNING,     //!< The protocol is being updated everytime.
    PROTOCOL_STATE_PAUSED,      //!< The protocol is paused.
    PROTOCOL_STATE_TERMINATED   //!< The protocol is terminated/does not exist.
};

/*!
 * \enum PROTOCOL_REQUEST_TYPE
 * \brief Defines actions that can be done about protocols.
 * This enum is used essentially to keep the manager thread-safe and
 * to avoid protocols modifying directly their state.
 */
enum PROTOCOL_REQUEST_TYPE
{
    PROTOCOL_REQUEST_START,     //!< Start a protocol
    PROTOCOL_REQUEST_STOP,      //!< Stop a protocol
    PROTOCOL_REQUEST_PAUSE,     //!< Pause a protocol
    PROTOCOL_REQUEST_UNPAUSE,   //!< Unpause a protocol
    PROTOCOL_REQUEST_TERMINATE  //!< Terminate a protocol
};

/*!
* \struct ProtocolInfo
* \brief Stores the information needed to manage protocols
*/
typedef struct ProtocolInfo
{
    PROTOCOL_STATE  state;      //!< The state of the protocol
    Protocol*       protocol;   //!< A pointer to the protocol
    uint32_t        id;         //!< The unique id of the protocol
} ProtocolInfo;

/*!
* \struct ProtocolRequest
* \brief Represents a request to do an action about a protocol.
*/
typedef struct ProtocolRequest
{
    PROTOCOL_REQUEST_TYPE type; //!< The type of request
    ProtocolInfo protocol_info; //!< The concerned protocol information
} ProtocolRequest;

/*! \struct ProtocolRequest
 *  \brief Used to pass the event to protocols that need it
 */
typedef struct EventProcessingInfo
{
    Event* event;
    double arrival_time;
    std::vector<unsigned int> protocols_ids;
} EventProcessingInfo;

/*!
 * \class ProtocolManager
 * \brief Manages the protocols at runtime.
 *
 * This class is in charge of storing and managing protocols.
 * It is a singleton as there can be only one protocol manager per game
 * instance. Any game object that wants to start a protocol must create a
 * protocol and give it to this singleton. The protocols are updated in a
 * special thread, to ensure that they are processed independently from the
 * frames per second. Then, the management of protocols is thread-safe: any
 * object can start/pause/stop protocols whithout problems.
 */
class ProtocolManager : public AbstractSingleton<ProtocolManager>
{
    friend class AbstractSingleton<ProtocolManager>;
    friend void* protocolManagerAsynchronousUpdate(void* data);
    public:
        
        /*! \brief Stops the protocol manager. */
        virtual void            abort();
        /*!
         * \brief Function that processes incoming events.
         * This function is called by the network manager each time there is an
         * incoming packet.
         */
        virtual void            notifyEvent(Event* event);
        /*!
         * \brief WILL BE COMMENTED LATER
         */
        virtual void            sendMessage(Protocol* sender, const NetworkString& message, bool reliable = true);
        /*!
         * \brief WILL BE COMMENTED LATER
         */
        virtual void            sendMessage(Protocol* sender, STKPeer* peer, const NetworkString& message, bool reliable = true);
        /*!
         * \brief WILL BE COMMENTED LATER
         */
        virtual void            sendMessageExcept(Protocol* sender, STKPeer* peer, const NetworkString& message, bool reliable = true);

        /*!
         * \brief Asks the manager to start a protocol.
         * This function will store the request, and process it at a time it is
         * thread-safe.
         * \param protocol : A pointer to the protocol to start
         * \return The unique id of the protocol that is being started.
         */
        virtual uint32_t        requestStart(Protocol* protocol);
        /*!
         * \brief Asks the manager to stop a protocol.
         * This function will store the request, and process it at a time it is
         * thread-safe.
         * \param protocol : A pointer to the protocol to stop
         */
        virtual void            requestStop(Protocol* protocol);
        /*!
         * \brief Asks the manager to pause a protocol.
         * This function will store the request, and process it at a time it is
         * thread-safe.
         * \param protocol : A pointer to the protocol to pause
         */
        virtual void            requestPause(Protocol* protocol);
        /*!
         * \brief Asks the manager to unpause a protocol.
         * This function will store the request, and process it at a time it is
         * thread-safe.
         * \param protocol : A pointer to the protocol to unpause
         */
        virtual void            requestUnpause(Protocol* protocol);
        /*!
         * \brief Notifies the manager that a protocol is terminated.
         * This function will store the request, and process it at a time it is
         * thread-safe.
         * \param protocol : A pointer to the protocol that is finished
         */
        virtual void            requestTerminate(Protocol* protocol);

        /*!
         * \brief Updates the manager.
         *
         * This function processes the events queue, notifies the concerned
         * protocols that they have events to process. Then ask all protocols
         * to update themselves. Finally processes stored requests about
         * starting, stoping, pausing etc... protocols.
         * This function is called by the main loop.
         * This function IS FPS-dependant.
         */
        virtual void            update();
        /*!
         * \brief Updates the manager.
         *
         * This function processes the events queue, notifies the concerned
         * protocols that they have events to process. Then ask all protocols
         * to update themselves. Finally processes stored requests about
         * starting, stoping, pausing etc... protocols.
         * This function is called in a thread.
         * This function IS NOT FPS-dependant.
         */
        virtual void            asynchronousUpdate();

        /*!
         * \brief Get the number of protocols running.
         * \return The number of protocols that are actually running.
         */
        virtual int             runningProtocolsCount();
        /*!
         * \brief Get the state of a protocol using its id.
         * \param id : The id of the protocol you seek the state.
         * \return The state of the protocol.
         */
        virtual PROTOCOL_STATE  getProtocolState(uint32_t id);
        /*!
         * \brief Get the state of a protocol using a pointer on it.
         * \param protocol : A pointer to the protocol you seek the state.
         * \return The state of the protocol.
         */
        virtual PROTOCOL_STATE  getProtocolState(Protocol* protocol);
        /*!
         * \brief Get the id of a protocol.
         * \param protocol : A pointer to the protocol you seek the id.
         * \return The id of the protocol pointed by the protocol parameter.
         */
        virtual uint32_t        getProtocolID(Protocol* protocol);

        /*!
         * \brief Get a protocol using its id.
         * \param id : Unique ID of the seek protocol.
         * \return The protocol that has the ID id.
         */
        virtual Protocol*       getProtocol(uint32_t id);
        /*!
         * \brief Get a protocol using its type.
         * \param type : The type of the protocol.
         * \return The protocol that matches the given type.
         */
        virtual Protocol*       getProtocol(PROTOCOL_TYPE type);

        /*! \brief Know whether the app is a server.
         *  \return True if this application is in server mode, false elseway.
         */
        bool                    isServer();

        /*! \brief Tells if we need to stop the update thread. */
        int                     exit();

    protected:
        // protected functions
        /*!
         * \brief Constructor
         */
        ProtocolManager();
        /*!
         * \brief Destructor
         */
        virtual ~ProtocolManager();
        /*!
         * \brief Assign an id to a protocol.
         * This function will assign m_next_protocol_id as the protocol id.
         * This id starts at 0 at the beginning and is increased by 1 each time
         * a protocol starts.
         * \param protocol_info : The protocol info that needs an id.
         */
        void                    assignProtocolId(ProtocolInfo* protocol_info);

        /*!
         * \brief Starts a protocol.
         * Add the protocol info to the m_protocols vector.
         * \param protocol : ProtocolInfo to start.
         */
        virtual void            startProtocol(ProtocolInfo protocol);
        /*!
         * \brief Stops a protocol.
         * Coes nothing. Noone can stop running protocols for now.
         * \param protocol : ProtocolInfo to stop.
         */
        virtual void            stopProtocol(ProtocolInfo protocol);
        /*!
         * \brief Pauses a protocol.
         * Pauses a protocol and tells it that it's being paused.
         * \param protocol : ProtocolInfo to pause.
         */
        virtual void            pauseProtocol(ProtocolInfo protocol);
        /*!
         * \brief Unpauses a protocol.
         * Unpauses a protocol and notifies it.
         * \param protocol : ProtocolInfo to unpause.
         */
        virtual void            unpauseProtocol(ProtocolInfo protocol);
        /*!
         * \brief Notes that a protocol is terminated.
         * Remove a protocol from the protocols vector.
         * \param protocol : ProtocolInfo concerned.
         */
        virtual void            protocolTerminated(ProtocolInfo protocol);

        bool                    propagateEvent(EventProcessingInfo* event, bool synchronous);

        // protected members
        /*!
         * \brief Contains the running protocols.
         * This stores the protocols that are either running or paused, their
         * state and their unique id.
         */
        std::vector<ProtocolInfo>       m_protocols;
        /*!
         * \brief Contains the network events to pass to protocols.
         */
        std::vector<EventProcessingInfo>             m_events_to_process;
        /*!
         * \brief Contains the requests to start/stop etc... protocols.
         */
        std::vector<ProtocolRequest>    m_requests;
        /*! \brief The next id to assign to a protocol.
         * This value is incremented by 1 each time a protocol is started.
         * If a protocol has an id lower than this value, it means that it have
         * been formerly started.
         */
        uint32_t                        m_next_protocol_id;

        // mutexes:
        /*! Used to ensure that the event queue is used thread-safely.       */
        pthread_mutex_t                 m_events_mutex;
        /*! Used to ensure that the protocol vector is used thread-safely.   */
        pthread_mutex_t                 m_protocols_mutex;
        /*! Used to ensure that the protocol vector is used thread-safely.   */
        pthread_mutex_t                 m_asynchronous_protocols_mutex;
        /*! Used to ensure that the request vector is used thread-safely.    */
        pthread_mutex_t                 m_requests_mutex;
        /*! Used to ensure that the protocol id is used in a thread-safe way.*/
        pthread_mutex_t                 m_id_mutex;
        /*! Used when need to quit.*/
        pthread_mutex_t                 m_exit_mutex;

        /*! Update thread.*/
        pthread_t* m_update_thread;
        /*! Asynchronous update thread.*/
        pthread_t* m_asynchronous_update_thread;
        /*! True if the thread is running. */
        bool m_asynchronous_thread_running;

};

#endif // PROTOCOL_MANAGER_HPP
