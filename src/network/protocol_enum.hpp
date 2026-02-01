#ifndef PROTOCOL_ENUM_HPP
#define PROTOCOL_ENUM_HPP

/** \enum ProtocolType
 *  \brief The types that protocols can have. This is used to select which 
 *   protocol receives which event.
 *  \ingroup network
 */
enum ProtocolType
{
    PROTOCOL_NONE              = 0x00,  //!< No protocol type assigned.
    PROTOCOL_CONNECTION        = 0x01,  //!< Protocol that deals with client-server connection.
    PROTOCOL_LOBBY_ROOM        = 0x02,  //!< Protocol that is used during the lobby room phase.
    PROTOCOL_GAME_EVENTS       = 0x03,  //!< Protocol to communicate the game events.
    PROTOCOL_CONTROLLER_EVENTS = 0x04,  //!< Protocol to transfer controller modifications
    PROTOCOL_SILENT            = 0x05,  //!< Used for protocols that do not subscribe to any network event.
    PROTOCOL_MAX                     ,  //!< Maximum number of different protocol types
    PROTOCOL_SYNCHRONOUS       = 0x80,  //!< Flag, indicates synchronous delivery
};   // ProtocolType

// ----------------------------------------------------------------------------
/** \enum ProtocolState
 *  \brief Defines the three states that a protocol can have.
 */
enum ProtocolState
{
    PROTOCOL_STATE_INITIALISING, //!< The protocol is waiting to be started
    PROTOCOL_STATE_RUNNING,      //!< The protocol is being updated everytime.
    PROTOCOL_STATE_PAUSED,       //!< The protocol is paused.
    PROTOCOL_STATE_TERMINATED    //!< The protocol is terminated/does not exist.
};   // ProtocolState

#endif // PROTOCOL_ENUM_HPP
