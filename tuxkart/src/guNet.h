//  $Id: guNet.h,v 1.2 2004/07/31 23:46:18 grumbel Exp $
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

#ifndef HEADER_GUNET_H
#define HEADER_GUNET_H

#define GU_UDP_DEFAULT_PORT_NUMBER  5100

class guUDPConnection
{
  struct sockaddr_in  *in_addr ;
  struct sockaddr_in *out_addr ;

  int sockfd ;
  int  port  ;
  int iport  ;
  int oport  ;

 public:

   guUDPConnection () ;
  ~guUDPConnection () ;
  
  void disconnect  () ;

  int connect      ( char *hostname = "localhost",
                     int   _port    = GU_UDP_DEFAULT_PORT_NUMBER ) ;

  int sendMessage  ( char *mesg, int length ) ;
  int recvMessage  ( char *mesg, int length ) ;
} ;


#endif

