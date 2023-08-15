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

#include "guiengine/message_queue.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/skin.hpp"
#include "utils/synchronised.hpp"

#include "GlyphLayout.h"
#include <IrrlichtDevice.h>
#include <atomic>
#include <memory>

// Can be removed later when android STK uses SDL2
#ifdef IOS_STK
#include "SDL_keyboard.h"
#endif

using namespace GUIEngine;

namespace MessageQueue
{
// ============================================================================
/** The label widget used to show the current message. */
std::unique_ptr<SkinWidgetContainer> g_container;
std::unique_ptr<SkinWidgetContainer> g_static_container;

/** Stores the height of an static container to allow showing both. */
int s_msg_raise = 0;

// ============================================================================
/** A base class for any messages. */
class Message
{
protected:
    /** If this < 0, remove this message from queue. */
    float m_display_timer;

    /** The message. */
    core::stringw m_message;

    /** The area which the message is drawn. */
    core::recti m_area;

private:
    /** Tell if this message has been initialized. */
    bool m_inited;

public:
    Message(float timer, const core::stringw& msg) :
    m_display_timer(timer), m_message(msg), m_inited(false) {}
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

};

// ============================================================================
/** A small helper class to store and sort text messages to be displayed. */
class TextMessage : public Message
{
protected:
    /** The type of the message. */
    MessageQueue::MessageType m_message_type;

    /** The render type of the message: either achievement-message::neutral
     *  or friend-message::neutral. */
    std::string m_render_type;

    /** The text layout, can do linebreak if needed. */
    std::vector<gui::GlyphLayout> m_gls;

    /** Drawing rectangle of text layout. */
    core::recti m_text_rect;

    /** A pointer to the Container for the Message */
    SkinWidgetContainer* m_container;

    /** Font and screen size used to detect if re-initialization is needed */
    gui::IGUIFont* m_font;
    core::dimension2du m_screen_size;
public:
    TextMessage(MessageQueue::MessageType mt, const core::stringw &message) :
        Message(5.0f, message)
    {
        m_font = NULL;
        m_container = g_container.get();
        m_message_type = mt;
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
    }
    // ------------------------------------------------------------------------
    /** Returns the type of the message.*/
    virtual MessageQueue::MessageType getMessageType() const
                                                     { return m_message_type; }
    // ------------------------------------------------------------------------
    /** Init the message text, do linebreak as required. */
    virtual void init()
    {
        const GUIEngine::BoxRenderParams &brp =
            GUIEngine::getSkin()->getBoxRenderParams(m_render_type);
        m_screen_size = irr_driver->getActualScreenSize();
        const unsigned width = m_screen_size.Width;
        const unsigned height = m_screen_size.Height;
        m_font = GUIEngine::getFont();
        m_font->initGlyphLayouts(m_message, m_gls);
        gui::removeHighlightedURL(m_gls);
        // Reserve space for 3 lines of text, it will occupy the circle
        const int max_width = width - (brp.m_left_border +
            brp.m_right_border) - (m_font->getHeightPerLine() * 3);
        if (max_width < 0)
        {
            m_display_timer = -1;
            return;
        }

        gui::breakGlyphLayouts(m_gls, (float)max_width,
            m_font->getInverseShaping(), m_font->getScale());
        core::dimension2du dim = gui::getGlyphLayoutsDimension(m_gls,
            m_font->getHeightPerLine(), m_font->getInverseShaping(),
            m_font->getScale());

        if ((int)dim.Height > m_font->getHeightPerLine() * 3)
        {
            // Max 3 lines to prevent too long message from network chat
            int newline_count = 0;
            for (unsigned i = 0; i < m_gls.size(); i++)
            {
                if (m_gls[i].flags & gui::GLF_NEWLINE)
                {
                    if (++newline_count >= 3)
                    {
                        m_gls.erase(m_gls.begin() + i, m_gls.end());
                        dim.Height = m_font->getHeightPerLine() * 3;
                        break;
                    }
                }
            }
        }

        int left_icon_size = dim.Height;
        int total_width = dim.Width + brp.m_left_border + brp.m_right_border +
            left_icon_size;
        int x = (width - total_width) / 2;
        int y = height - int(dim.Height) - m_font->getHeightPerLine() / 2;

        if (x < 0 || y < 0)
        {
            m_gls.clear();
            m_display_timer = -1;
            return;
        }

        m_area = irr::core::recti(x, y, x + total_width, y + dim.Height);
        m_text_rect = core::recti(x, y, x + left_icon_size + total_width,
            y + dim.Height);

        if (m_container == g_static_container.get())
            s_msg_raise = int(dim.Height) + m_font->getHeightPerLine() / 10;
    }
    // ------------------------------------------------------------------------
    /** Draw the message. */
    virtual void draw(float dt)
    {
        if (m_gls.empty())
        {
            Message::draw(dt);
            return;
        }
        Message::draw(dt);
        // This happen when GUIEngine is reset (when font size changed for
        // example)
        if (m_font != GUIEngine::getFont() ||
            m_screen_size != irr_driver->getActualScreenSize())
            init();
        int pos_transform = 0;
        if (m_container == g_container.get())
            pos_transform = s_msg_raise;
        int moved_height = irr_driver->getDevice()->getMovedHeight();
        core::position2di raise = core::position2di(0,
            irr_driver->getDevice()->getOnScreenKeyboardHeight() -
            moved_height + pos_transform);
        GUIEngine::getSkin()->drawMessage(m_container, m_area - raise,
            m_render_type);
        GUIEngine::getFont()->draw(m_gls, m_text_rect - raise,
            GUIEngine::getSkin()->getColor("text::neutral"), true/*hcenter*/);
    }

};   // class TextMessage

class StaticTextMessage : public TextMessage
{
public:
    StaticTextMessage(MessageQueue::MessageType mt,
                      const core::stringw& message) :
        TextMessage(mt, message)
    {
        m_container = g_static_container.get();
    }   // StaticTextMessage
    // ------------------------------------------------------------------------
    ~StaticTextMessage()
    {
    }
    // ------------------------------------------------------------------------
    /** Draw the message. */
    virtual void draw(float dt)
    {
        TextMessage::draw(dt);
        m_display_timer = 9999999.9f;
    }
};

// ============================================================================
/** A function class to compare messages, required for priority_queue. */
class CompareMessages
{
public:
    /** Used to sort messages by priority in the priority queue. Achievement
     * messages (1) need to have a higher priority than friend messages
     * (value 0), and errors (3) the highest priority. */
    bool operator() (const std::unique_ptr<Message>& a,
                     const std::unique_ptr<Message>& b) const
    {
        return a->getMessageType() < b->getMessageType();
    }   // operator ()
};   // operator()

// ============================================================================
/** List of all messages. */
Synchronised<std::priority_queue<std::unique_ptr<Message>,
    std::vector<std::unique_ptr<Message> >, CompareMessages> > g_all_messages;

// ============================================================================
/** The Storage for a Static Message */
std::unique_ptr<StaticTextMessage> g_static_message;

// ============================================================================
/** Add any message to the message queue.
 *  \param message Any message.
 */
void privateAdd(Message* m)
{
    g_all_messages.lock();
    g_all_messages.getData().emplace(std::unique_ptr<Message>(m));
    g_all_messages.unlock();
}   // privateAdd

// ============================================================================
/** Global progress bar percentage. */
std::atomic<int> g_progress(-1);

// ============================================================================
/** A class which display a progress bar in game, only one can be displayed. */
class ProgressBarMessage : public Message
{
private:
    SkinWidgetContainer m_swc;
public:
    ProgressBarMessage(const core::stringw& msg) :
               Message(9999999.9f, msg)
    {
        m_swc.m_next = new SkinWidgetContainer;
    }   // ProgressBarMessage
    // ------------------------------------------------------------------------
    ~ProgressBarMessage()                             { g_progress.store(-1); }
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
        int moved_height = irr_driver->getDevice()->getMovedHeight();
        core::position2di raise = core::position2di(0,
            irr_driver->getDevice()->getOnScreenKeyboardHeight() -
            moved_height);
        GUIEngine::getSkin()->drawProgressBarInScreen(&m_swc, m_area - raise,
            (float)g_progress.load() / 100.0f);
        video::SColor color(255, 0, 0, 0);
        GUIEngine::getFont()->draw(m_message, m_area - raise, color, true,
            true);
        if (g_progress.load() >= 100)
            m_display_timer = -1.0f;
    }

};   // class ProgressBarMessage

// ============================================================================
/** Called when the screen resolution is changed to compute the new
 *  position of the message. */
void updatePosition()
{
    if (g_static_message)
        g_static_message->init();
}   // updatePosition

// ----------------------------------------------------------------------------
/** Adds a Text message to the message queue.
 *  \param mt The MessageType of the message.
 *  \param message The actual message.
 */
void add(MessageType mt, const irr::core::stringw &message)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
    Message *m = new TextMessage(mt, message);
    privateAdd(m);
#endif
}   // add

// ----------------------------------------------------------------------------
/** Adds a Text message which has to be discarded or overridden manually.
 *  It is only possible to show one static Message at a time!
 *  \param mt The MessageType of the message.
 *  \param message The actual message.
 */
void addStatic(MessageType mt, const irr::core::stringw &message)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
    g_static_message.reset(new StaticTextMessage(mt, message));
#endif
}   // addStatic

// ----------------------------------------------------------------------------
/** Update function called from the GUIEngine to handle displaying of the
 *  messages. It will make sure that each message is shown for a certain
 *  amount of time, before it is discarded and the next message (if any)
 *  is displayed.
 *  \param dt Time step size.
 */
void update(float dt)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    if (g_static_message)
        g_static_message->draw(dt);

    g_all_messages.lock();
    if (!g_all_messages.getData().empty())
    {
        auto& current = g_all_messages.getData().top();
        current->draw(dt);

        if (current->canBeRemoved())
            g_all_messages.getData().pop();
    }
    g_all_messages.unlock();
#endif
}   // update

// ----------------------------------------------------------------------------
/** The time a user firstly call this function with a zero progress, a progress
 *  bar will display in the game, the time the value of progress become 100,
 *  the progress bar will disappear. So make sure only 1 progress bar is being
 *  used each time.
 *  \param progress Progress from 0 to 100, -1 to reset
 */
void showProgressBar(int progress, const core::stringw& msg)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    if (g_progress.load() == -1 && progress >= 0)
    {
        g_progress.store(progress);
        privateAdd(new ProgressBarMessage(msg));
    }
    else
        g_progress.store(progress);
#endif
}   // showProgressBar

// ----------------------------------------------------------------------------
/** Remove the static Message from display.
 */
void discardStatic()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    g_static_message.reset();
    s_msg_raise = 0;
#endif
}    // discardStatic

// ----------------------------------------------------------------------------
/** Reset global variables (for clear starting in android).
 */
void resetGlobalVariables()
{
#ifndef SERVER_ONLY
    g_container.reset(new SkinWidgetContainer());
    g_static_container.reset(new SkinWidgetContainer());
    g_static_message.reset();
    s_msg_raise = 0;
    while (!g_all_messages.getData().empty())
        g_all_messages.getData().pop();
#endif
}

}   // namespace GUIEngine

// ----------------------------------------------------------------------------
