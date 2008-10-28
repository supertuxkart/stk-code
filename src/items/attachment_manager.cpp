//  $Id: attachment_manager.cpp 808 2006-10-03 20:17:37Z coz $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "items/attachment_manager.hpp"
#include "file_manager.hpp"
#include "loader.hpp"

AttachmentManager *attachment_manager = 0;

struct  initAttachmentType {attachmentType attachment; const char *file;};

/* Some explanations to the attachments:
   Parachute: This will increase the air friction, reducing the maximum speed.
              It will not have too much of an effect on slow speeds, since air
              friction only becomes important at higher speeds.
   Anvil:     It increases the weight of the kart.But this will NOT have any
              effect on karts already driving at highest speed: the accelerating
       force is independent of the mass, so it is 0 at highest speed 
       (engine force = air- plus system-force) and only this value gets
       divided by the mass later --> at highest speed there would be no 
       effect when the mass is changed, only at lower speeds the acting 
       acceleration will be lower.Reducing the power slows the kart down,
       but doesn't give the feeling of a sudden weight increase. 
       Therefore the anvil will reduce by a certain factor (see physics
       parameters) once when it is attached. Together with the mass 
       increase (lower acceleration) it's sufficient negative.
*/

initAttachmentType iat[]=
{
    {ATTACH_PARACHUTE,   "parachute.ac"},
    {ATTACH_BOMB,        "bomb.ac"},
    {ATTACH_ANVIL,       "anvil.ac"},
    {ATTACH_TINYTUX,     "tinytux_magnet.ac"},
    {ATTACH_MAX,         ""},
};

//-----------------------------------------------------------------------------
void AttachmentManager::removeTextures()
{
    for(int i=0; iat[i].attachment!=ATTACH_MAX; i++)
    {
        ssgDeRefDelete(m_attachments[iat[i].attachment]);
    }   // for
    callback_manager->clear(CB_ATTACHMENT);
}   // removeTextures

//-----------------------------------------------------------------------------
void AttachmentManager::loadModels()
{
    for(int i=0; iat[i].attachment!=ATTACH_MAX; i++)
    {
        m_attachments[iat[i].attachment]=loader->load(iat[i].file, CB_ATTACHMENT);
        m_attachments[iat[i].attachment]->ref();
    }   // for
}   // reInit

