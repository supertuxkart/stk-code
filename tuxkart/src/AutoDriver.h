//  $Id: AutoDriver.h,v 1.2 2004/08/15 13:57:55 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#ifndef HEADER_AUTODRIVER_H
#define HEADER_AUTODRIVER_H

#include "Controller.h"

class NetworkDriver : public Controller
{
public:
  NetworkDriver()
  {
  }

  virtual ~NetworkDriver() {}

  virtual void update (float delta) ;
};

class AutoDriver : public Controller
{
private:
  float time_since_last_shoot ;
public:
  AutoDriver()
  {
    time_since_last_shoot = 0.0f ;
  }

  virtual ~AutoDriver() {}

  virtual void update (float delta) ;
};

#endif

/* EOF */
