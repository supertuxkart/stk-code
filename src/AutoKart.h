//  $Id: AutoKart.h,v 1.3 2005/08/17 22:36:30 joh Exp $
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

#ifndef HEADER_AUTOKART_H
#define HEADER_AUTOKART_H

#include "Kart.h"

class AutoKart : public Kart {
private:
  float time_since_last_shoot ;
  size_t future_hint;

  bool lane_change;
  float start_lane_pos;
  float target_lane_pos;

  struct CrashTypes
  {
      SGfloat curve;
      SGfloat kart;
      CrashTypes() : curve(0.0f), kart(0.0f) {};
  };

  bool do_wheelie(const int &STEPS);
  SGfloat change_lane(const size_t &NEXT_HINT);
  SGfloat find_steer_to_paralel(const size_t &NEXT_HINT);
  SGfloat find_steer_to_point(const sgVec2 POINT);
  void check_crashes(CrashTypes &crashes, const int &STEPS);
  float guess_accel (const float throttle);
public:
  AutoKart(const KartProperties *kart_properties, int position);

  void update (float delta) ;
  void reset  ();
};

#endif

/* EOF */
