//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

#ifndef HEADER_ATTACHMENT_PLUGIN_HPP
#define HEADER_ATTACHMENT_PLUGIN_HPP

#include "vector3d.h"

class AbstractKart;
class Attachment;
class BareNetworkString;

/**
  * \ingroup items
  *  This is the base class for a plugin into an attachment. Plugins are
  *  used to handle attachment specific data so that the attachment class
  *  that is used in every kart isn't overloaded. It could be done by
  *  inheriting from Attachment, but then every time an attachment is
  *  changed, we could delete and create a new SceneNode. To avoid this
  *  overhead, we use plugins to encapsulate additional code for some
  *  plugins.
  */
class AttachmentPlugin
{
protected:
    /** Kart the attachment is attached to. */
    AbstractKart* m_kart;

    Attachment* m_attachment;
public:
    /** Constructor for a plugin. */
    AttachmentPlugin(AbstractKart *kart, Attachment* attachment)
    {
        m_kart = kart;
        m_attachment = attachment;
    }

    virtual ~AttachmentPlugin() {}

    virtual void updateGraphics(float dt) = 0;

    // ------------------------------------------------------------------------
    /** Updates a plugin. This is called once each time frame. If the
     *  function returns true, the attachment is discarded. */
    virtual bool updateAndTestFinished() = 0;
    // ------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString *buffer) {}
    // ------------------------------------------------------------------------
    virtual void saveState(BareNetworkString *buffer) const {}

};   // AttachmentPlugin

#endif
