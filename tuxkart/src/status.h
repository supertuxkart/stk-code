//  $Id: status.h,v 1.5 2004/08/04 16:35:39 jamesgregory Exp $
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

#ifndef HEADER_STATUS_H
#define HEADER_STATUS_H

void drawStatusText () ;

void stPrintf ( char *fmt, ... ) ;
void stToggle () ;
void fpsToggle () ;

void memorial    () ;
void about       () ;
void credits     () ;
void versions    () ;
void help        () ;
void hide_status () ;
void initStatusDisplay () ;

void start_stopwatch () ;

extern int time_out ;

#endif

/* EOF */
