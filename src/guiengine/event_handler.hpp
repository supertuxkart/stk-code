//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#ifndef HEADER_EVENT_HANDLER_HPP
#define HEADER_EVENT_HANDLER_HPP

#include <IEventReceiver.h>
#include "input/input.hpp"

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
    
    /**
      * \ingroup guiengine
      */
    enum EventPropagation
    {
        EVENT_BLOCK,
        EVENT_LET
    };
    
    class Widget;
    
    /**
     * \brief Class to handle irrLicht events (GUI and input as well)
     *
     * input events will be redirected to the input module in game mode.
     * In menu mode, input is mapped to game actions with the help of the input
     * module, then calls are made to move focus / trigger an event / etc.
     *
     * This is really only the irrLicht events bit, not to be confused with my own simple events dispatched
     * mainly through AbstractStateManager, and also to widgets (this class is some kind of bridge between
     * the base irrLicht GUI engine and the STK layer on top of it)
     *
     * \ingroup guiengine
     */
    class EventHandler : public irr::IEventReceiver
    {
        EventPropagation onGUIEvent(const irr::SEvent& event);
        EventPropagation onWidgetActivated(Widget* w, const int playerID);
        void navigateUp(const int playerID, Input::InputType type, const bool pressedDown);
        void navigateDown(const int playerID, Input::InputType type, const bool pressedDown);
        
        /** \brief          send an event to the GUI module user's event callback
          * \param widget   the widget that triggerred this event
          * \param name     the name/ID (PROP_ID) of the widget that triggerred this event
          * \param playerID ID of the player that triggerred this event
          */
        void sendEventToUser(Widget* widget, std::string& name, const int playerID);
        
    public:
        EventHandler();
        ~EventHandler();
        
        /**
         * All irrLicht events will go through this (input as well GUI; input events are
         * immediately delegated to the input module, GUI events are processed here)
         */
        bool OnEvent (const irr::SEvent &event);
        
        /**
         * When the input module is done processing an input and mapped it to an action,
         * and this action needs to be applied to the GUI (e.g. fire pressed, left
         * pressed, etc.) this method is called back by the input module.
         */
        void processGUIAction(const PlayerAction action, int deviceID, const unsigned int value,
                              Input::InputType type, const int playerID);
        
        /** singleton access */
        static EventHandler* get();
    };
    
}

#endif
