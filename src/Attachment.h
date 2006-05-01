//  $Id: Attachment.h,v 1.5 2005/08/19 20:51:56 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_ATTACHMENT_H
#define HEADER_ATTACHMENT_H

class Kart;
class ssgEntity;

// Some loop in Attachment.cxx depend on PARACHUTE being the first element,
// and TINYTUX being the last one. So if new elemts are added, make sure
// to add them in between those values.
enum attachmentType { ATTACH_PARACHUTE, ATTACH_MAGNET,
		      ATTACH_MAGNET_BZZT, ATTACH_ANVIL, ATTACH_TINYTUX,
                      ATTACH_MAX, ATTACH_NOTHING};


// The attachment manager is only used as a static member in
// attachment.
class AttachmentManager {
 private:
  ssgEntity *attachments[ATTACH_MAX];
 public:
  AttachmentManager();
  ssgEntity *getModel(attachmentType type) {return attachments[type];}
};

class Attachment {
 private:
  static AttachmentManager *attachment_manager;
  attachmentType  type;
  Kart           *kart;
  float           time_left;
  ssgSelector    *holder;    // where the attachment is put on the kart
 public:
  Attachment(Kart* _kart);
  void           set            (attachmentType _type, float time);
  void           set            (attachmentType _type)
                                   {set(_type, time_left);             }
  void           clear          () {type=ATTACH_NOTHING; time_left=0.0;
                                    holder->select(0);                 }
  attachmentType getType        () {return type;                       }
  float          getTimeLeft    () {return time_left;                  }
  void           hitGreenHerring();
  void           update         (float dt, sgCoord *velocity);
  ~Attachment();
};

#endif
