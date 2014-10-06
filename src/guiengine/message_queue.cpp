//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Joerg Henrichs
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
/**
  \page addons Addons
  */

#include "guiengine/message_queue.hpp"

#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/skin.hpp"

#include "IGUIElement.h"

using namespace GUIEngine;

namespace MessageQueue
{

/** A small helper class to store and sort messages to be displayed. */
class Message
{
private:
    /** The type of the message. */
    MessageQueue::MessageType m_message_type;
    /** The message. */
    core::stringw m_message;

    /** The render type of the message: either achievement-message::neutral
     *  or friend-message::neutral. */
    std::string m_render_type;

public:
    Message(MessageQueue::MessageType mt, const core::stringw &message)
    {
        m_message_type = mt;
        m_message      = message;
        if(mt==MessageQueue::MT_ACHIEVEMENT)
            m_render_type = "achievement-message::neutral";
        else
            m_render_type = "friend-message::neutral";
    }   // Message
    // ------------------------------------------------------------------------
    /** Returns the message. */
    const core::stringw & getMessage() const { return m_message; }
    // ------------------------------------------------------------------------
    /** Returns the type of the message (achievement or friend). */
    MessageQueue::MessageType getMessageType() const { return m_message_type; }
    // ------------------------------------------------------------------------
    /** Returns the render type: either achievement-message::neutral or
     *  friend-message::neutral (see skin for details). */
    const std::string &getRenderType() const
    {
        return m_render_type;
    }
};   // class Message

// ============================================================================
/** A function class to compare messages, required for priority_queue. */
class CompareMessages
{
public:
    /** Used to sort messages by priority in the priority queue. Achievement
     * messages (1) need to have a higher priority than friend messages
     * (value 0). */
    bool operator() (const Message *a, const Message *b) const
    {
        return a->getMessageType() < b->getMessageType();
    }   // operator ()
};   // operator()


// ============================================================================
/** List of all messages. */
std::priority_queue<Message*, std::vector<Message*>,
                   CompareMessages> g_all_messages;

/** How long the current message has been displayed. The special value
 *  -1 indicates that a new message was added when the queue was empty. */
float        g_current_display_time = -1.0f;

/** How long the current message should be displaed. */
float        g_max_display_time     = 5.0f;

/** The label widget used to show the current message. */
SkinWidgetContainer *g_container    = NULL;
core::recti g_area;

// ============================================================================

void createLabel(const Message *message)
{
    if(!g_container)
        g_container = new SkinWidgetContainer();

    gui::ScalableFont *font = GUIEngine::getFont();
    core::dimension2du dim = font->getDimension(message->getMessage().c_str());
    g_current_display_time = 0.0f;
    // Maybe make this time dependent on message length as well?
    g_max_display_time     = 5.0f;
    const GUIEngine::BoxRenderParams &brp =
        GUIEngine::getSkin()->getBoxRenderParams(message->getRenderType());
    dim.Width +=brp.m_left_border + brp.m_right_border;
    int x = (UserConfigParams::m_width - dim.Width) / 2;
    int y = UserConfigParams::m_height - int(1.5f*dim.Height);
    g_area = irr::core::recti(x, y, x+dim.Width, y+dim.Height);
}   // createLabel

// ----------------------------------------------------------------------------
/** Adds a message to the message queue.
 *  \param mt The MessageType of the message.
 *  \param message The actual message.
 */
void add(MessageType mt, const irr::core::stringw &message)
{
    Message *m = new Message(mt, message);
    if(g_all_messages.size()==0)
    {
        // Indicate that there is a new message, which should
        // which needs a new label etc. to be computed.
        g_current_display_time =-1.0f;
    }
    g_all_messages.push(m);
}   // add

// ----------------------------------------------------------------------------
/** Update function called from the GUIEngine to handle displaying of the
 *  messages. It will make sure that each message is shown for a certain
 *  amount of time, before it is discarded and the next message (if any)
 *  is displayed.
 *  \param dt Time step size.
 */
void update(float dt)
{
    if(g_all_messages.size()==0) return;

    g_current_display_time += dt;
    if(g_current_display_time > g_max_display_time)
    {
        Message *last = g_all_messages.top();
        g_all_messages.pop();
        delete last;
        if(g_all_messages.size()==0) return;
        g_current_display_time = -1.0f;
    }

    // Create new data for the display.
    if(g_current_display_time < 0)
    {
        createLabel(g_all_messages.top());
    }

    Message *current = g_all_messages.top();
    GUIEngine::getSkin()->drawMessage(g_container, g_area,
                                      current->getRenderType());
    gui::ScalableFont *font = GUIEngine::getFont();
    
    video::SColor color(255, 0, 0, 0);
    font->draw(current->getMessage(), g_area, color, true, true);
    
}   // update

}   // namespace GUIEngine

// ----------------------------------------------------------------------------
