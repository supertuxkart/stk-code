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
class XMLNode;

#include "items/attachment.hpp"
#include "utils/no_copy.hpp"

#include <map>
#include <string>

/**
  * \ingroup items
  */
class AttachmentManager: public NoCopy
{
private:
    std::map<std::string, scene::IAnimatedMesh*> m_attachments[Attachment::ATTACH_COUNT];
    std::map<std::string, std::string>           m_lib_id     [Attachment::ATTACH_COUNT];
    Material                                    *m_all_icons  [Attachment::ATTACH_COUNT];

    void loadAttachment(Attachment::AttachmentType type, const XMLNode &node);
public:
               AttachmentManager() {};
              ~AttachmentManager();
    void       loadModels       ();

    Attachment::AttachmentType getAttachmentType(const std::string &name) const;
    // ------------------------------------------------------------------------
    /** Returns the mesh for a certain attachment.
     *  \param type Type of the attachment needed.
     *  \parm id Check if there is an alternate mesh for this kart */
    scene::IAnimatedMesh *getMesh(Attachment::AttachmentType type,
                                  const std::string& id = "default") const;

    std::string getLibId(Attachment::AttachmentType type, const std::string& id) const;
    // ------------------------------------------------------------------------
    /** Returns the icon to display in the race gui if a kart
     *  has an attachment. */
    Material* getIcon (int type) const { return m_all_icons [type]; }
    // ------------------------------------------------------------------------
};

extern AttachmentManager *attachment_manager;
#endif
