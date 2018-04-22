//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_ATTACHMENT_MANAGER_HPP
#define HEADER_ATTACHMENT_MANAGER_HPP

namespace irr
{
    namespace scene { class IAnimatedMesh; }
}
class Material;

#include "items/attachment.hpp"
#include "utils/no_copy.hpp"

/**
  * \ingroup items
  */
class AttachmentManager: public NoCopy
{
private:
    scene::IAnimatedMesh *m_attachments[Attachment::ATTACH_MAX];
    Material             *m_all_icons [Attachment::ATTACH_MAX];
public:
               AttachmentManager() {};
              ~AttachmentManager();
    void       loadModels       ();
    // ------------------------------------------------------------------------
    /** Returns the mest for a certain attachment.
     *  \param type Type of the attachment needed. */
    scene::IAnimatedMesh *getMesh(Attachment::AttachmentType type) const
        {return m_attachments[type]; }
    // ------------------------------------------------------------------------
    /** Returns the icon to display in the race gui if a kart
     *  has an attachment. */
    Material* getIcon (int type) const { return m_all_icons [type]; }
    // ------------------------------------------------------------------------
};

extern AttachmentManager *attachment_manager;
#endif
