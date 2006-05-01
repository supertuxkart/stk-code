//  $Id: HookManager.h,v 1.2 2005/08/19 20:51:56 joh Exp $
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

#ifndef HEADER_HOOKMANAGER_H
#define HEADER_HOOKMANAGER_H

#include <plib/ssg.h>
#include <vector>
#define MODE_FORWARD     1
#define MODE_CYCLE       2
#define MODE_SHUTTLE     3
#define MODE_SINESHUTTLE 4

// A part of the scene graph, which is connected via a list
// maintained by hook_manager, and which needs some kind of
// update every time step.
class Hook : public ssgBase {
  //JH this should be properly declared as friend, so that
  //JH hookmanager can access the attributes here.
 public:
  ssgBranch *branch    ;
  void      *param ;
  void     (*hook)(ssgBranch *, void *) ;
  void     (*hit )(ssgBranch *, void *) ;
public:
  Hook (void     (*hook_)(ssgBranch  *, void *),
	void     (*hit_ )(ssgBranch  *, void *),
	ssgBranch *branch_,
	void      *param_ ) { branch = branch_; param  = param_;
                              hook   = hook_;   hit    = hit_; 
			      ref(); }
} ;

class HookManager {

  typedef std::vector<Hook*> HookList;
  HookList hookList;

 public:
  HookManager() {};
  void update ();
  void clearAll();
  void addHook(void  (*hook)(ssgBranch  *, void *),
	       void  (*hit )(ssgBranch  *, void *),
	       ssgBranch *b,
	       void      *param );
};

extern HookManager* hook_manager;
ssgBranch *process_userdata ( char *data ) ;

#endif
