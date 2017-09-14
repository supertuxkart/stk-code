//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/skin.hpp"
#include "utils/synchronised.hpp"
#include "utils/translation.hpp"

#include "IGUIElement.h"
#include "IGUIEnvironment.h"
#include "IGUIStaticText.h"

#include <atomic>

using namespace GUIEngine;

namespace MessageQueue
{
// ============================================================================
/** The label widget used to show the current message. */
SkinWidgetContainer* g_container = NULL;

// ============================================================================
/** A base class for any messages. */
class Message
{
protected:
    /** The message. */
    core::stringw m_message;

    /** If this < 0, remove this message from queue. */
    float m_display_timer;

    /** The area which the message is drawn. */
    core::recti m_area;

private:
    /** Tell if this message has been initialized. */
    bool m_inited;

public:
    Message(float timer) : m_display_timer(timer), m_inited(false) {}
    // ------------------------------------------------------------------------
    virtual ~Message() {}
    // ------------------------------------------------------------------------
    virtual MessageQueue::MessageType getMessageType() const = 0;
    // ------------------------------------------------------------------------
    virtual void init() = 0;
    // ------------------------------------------------------------------------
    virtual void draw(float dt)
    {
        if (!m_inited)
        {
            m_inited = true;
            init();
        }
        m_display_timer -= dt;
    }
    // ------------------------------------------------------------------------
    bool canBeRemoved() const                { return m_display_timer < 0.0f; }
    // ------------------------------------------------------------------------
    virtual void remove() {}

};

// ============================================================================
/** A small helper class to store and sort text messages to be displayed. */
class TextMessage : public Message
{
private:
    /** The type of the message. */
    MessageQueue::MessageType m_message_type;

    /** The render type of the message: either achievement-message::neutral
     *  or friend-message::neutral. */
    std::string m_render_type;

    /** The text label, can do linebreak if needed. */
    gui::IGUIStaticText* m_text;

public:
    TextMessage(MessageQueue::MessageType mt, const core::stringw &message) :
        Message(5.0f)
    {
        m_message_type = mt;
        m_message      = message;
        m_text         = NULL;
        assert(mt != MessageQueue::MT_PROGRESS);
        if (mt == MessageQueue::MT_ACHIEVEMENT)
            m_render_type = "achievement-message::neutral";
        else if (mt == MessageQueue::MT_ERROR)
            m_render_type = "error-message::neutral";
        else if (mt == MessageQueue::MT_GENERIC)
            m_render_type = "generic-message::neutral";
        else
            m_render_type = "friend-message::neutral";
    }   // Message
    // ------------------------------------------------------------------------
    ~TextMessage()
    {
        assert(m_text != NULL);
        m_text->drop();
    }
    // ------------------------------------------------------------------------
    /** Returns the type of the message.*/
    virtual MessageQueue::MessageType getMessageType() const
                                                     { return m_message_type; }
    // ------------------------------------------------------------------------
    /** Init the message text, do linebreak as required. */
    virtual void init()
    {
        if (m_text)
            m_text->drop();
        const GUIEngine::BoxRenderParams &brp =
            GUIEngine::getSkin()->getBoxRenderParams(m_render_type);
        const unsigned width = irr_driver->getActualScreenSize().Width;
        const unsigned height = irr_driver->getActualScreenSize().Height;
        const unsigned max_width = width - (brp.m_left_border +
            brp.m_right_border);
        m_text =
            GUIEngine::getGUIEnv()->addStaticText(m_message.c_str(),
            core::recti(0, 0, max_width, height));
        m_text->setRightToLeft(translations->isRTLText(m_message));
        core::dimension2du dim(m_text->getTextWidth(),
            m_text->getTextHeight());
        dim.Width += brp.m_left_border + brp.m_right_border;
        int x = (width - dim.Width) / 2;
        int y = height - int(1.5f * dim.Height);
        m_area = irr::core::recti(x, y, x + dim.Width, y + dim.Height);
        m_text->setRelativePosition(m_area);
        m_text->setTextAlignment(gui::EGUIA_CENTER, gui::EGUIA_CENTER);
        m_text->grab();
        m_text->remove();
    }
    // ------------------------------------------------------------------------
    /** Draw the message. */
    virtual void draw(float dt)
    {
        Message::draw(dt);
        GUIEngine::getSkin()->drawMessage(g_container, m_area, m_render_type);
        assert(m_text != NULL);
        m_text->draw();
    }
    // ------------------------------------------------------------------------
    virtual void remove()                                      { delete this; }

};   // class TextMessage

// ============================================================================
/** A function class to compare messages, required for priority_queue. */
class CompareMessages
{
public:
    /** Used to sort messages by priority in the priority queue. Achievement
     * messages (1) need to have a higher priority than friend messages
     * (value 0), and errors (3) the highest priority. */
    bool operator() (const Message *a, const Message *b) const
    {
        return a->getMessageType() < b->getMessageType();
    }   // operator ()
};   // operator()

// ============================================================================
/** List of all messages. */
Synchronised<std::priority_queue<Message*, std::vector<Message*>,
                   CompareMessages> > g_all_messages;

// ============================================================================
/** Add any message to the message queue.
 *  \param message Any message.
 */
void privateAdd(Message* m)
{
    g_all_messages.lock();
    g_all_messages.getData().push(m);
    g_all_messages.unlock();
}   // privateAdd

// ============================================================================
/** A class which display a progress bar in game, only one can be displayed. */
class ProgressBarMessage : public Message
{
private:
    std::atomic_int_fast8_t m_progress_value;

    bool m_showing;

    SkinWidgetContainer m_swc;
public:
    ProgressBarMessage() :
               Message(9999999.9f)
    {
        m_progress_value.store(0);
        m_showing = false;
    }   // ProgressBarMessage
    // ------------------------------------------------------------------------
    ~ProgressBarMessage() {}
    // ------------------------------------------------------------------------
    /** Returns the type of the message.*/
    virtual MessageQueue::MessageType getMessageType() const
                                                        { return MT_PROGRESS; }
    // ------------------------------------------------------------------------
    virtual void init()
    {
        const unsigned width = irr_driver->getActualScreenSize().Width;
        const unsigned height = irr_driver->getActualScreenSize().Height;
        core::dimension2du dim(int(width * 0.75f), int(height * 0.05f));
        int x = (width - dim.Width) / 2;
        int y = height - int(1.5f * dim.Height);
        m_area = irr::core::recti(x, y, x + dim.Width,
            y + dim.Height);
    }
    // ------------------------------------------------------------------------
    virtual void draw(float dt)
    {
        Message::draw(dt);
        m_display_timer = 9999999.9f;
        GUIEngine::getSkin()->drawProgressBarInScreen(&m_swc, m_area,
            m_progress_value.load());
        video::SColor color(255, 0, 0, 0);
        GUIEngine::getFont()->draw(m_message, m_area, color, true, true);
        if (m_progress_value.load() >= 100)
        {
            m_display_timer = -1.0f;
            m_showing = false;
        }
    }
    // ------------------------------------------------------------------------
    void setProgress(int progress, const wchar_t* msg)
    {
        if (progress < 0)
            return;
        if (progress > 100)
            progress = 100;
        m_progress_value.store((int_fast8_t)progress);
        if (!m_showing && progress == 0)
        {
            m_showing = true;
            m_message = msg;
            privateAdd(this);
        }
    }
};   // class ProgressBarMessage

// ============================================================================
/** One instance of progress bar. */
ProgressBarMessage g_progress_bar_msg;
// ============================================================================
/** Called when the screen resolution is changed to compute the new
 *  position of the message. */
void updatePosition()
{
    g_all_messages.lock();
    bool empty = g_all_messages.getData().empty();
    if (empty)
    {
        g_all_messages.unlock();
        return;
    }
    g_all_messages.getData().top()->init();
    g_all_messages.unlock();
}   // updatePosition

// ----------------------------------------------------------------------------
/** Adds a Text message to the message queue.
 *  \param mt The MessageType of the message.
 *  \param message The actual message.
 */
void add(MessageType mt, const irr::core::stringw &message)
{
    Message *m = new TextMessage(mt, message);
    privateAdd(m);
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
    if (!g_container)
        g_container = new SkinWidgetContainer();

    g_all_messages.lock();
    bool empty = g_all_messages.getData().empty();
    if (empty)
    {
        g_all_messages.unlock();
        return;
    }

    Message* current = g_all_messages.getData().top();
    current->draw(dt);

    if (current->canBeRemoved())
    {
        g_all_messages.getData().pop();
        current->remove();
    }
    g_all_messages.unlock();

}   // update

// ----------------------------------------------------------------------------
/** The time a user firstly call this function with a zero progress, a progress
 *  bar will display in the game, the time the value of progress become 100,
 *  the progress bar will disappear. So make sure only 1 progress bar is being
 *  used each time.
 *  \param progress Progress from 0 to 100.
 */
void showProgressBar(int progress, const wchar_t* msg)
{
    g_progress_bar_msg.setProgress(progress, msg);
}   // showProgressBar

}   // namespace GUIEngine

// ----------------------------------------------------------------------------
