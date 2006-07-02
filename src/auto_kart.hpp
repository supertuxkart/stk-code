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

#include "kart.hpp"

class AutoKart : public Kart {
private:
  float time_since_last_shoot ;
  size_t future_hint;

  float starting_delay;

  int next_curve_hint;
  int next_straight_hint;
  bool on_curve;
  bool handle_curve;

  enum DIRECTION{LEFT, RIGHT};
  DIRECTION curve_direction;

  struct CrashTypes
  {
      bool curve; //true if we are going to 'crash' with a curve
      int kart; //-1 if no crash, pos numbers are the kart it crashes with
      CrashTypes() : curve(false), kart(-1) {};
      void clear() {curve = false; kart = -1;}
  } crashes;


  float steer_to_angle(const size_t& NEXT_HINT, const float& ANGLE);
  float steer_to_point(const sgVec2 POINT);
  float steer_for_tight_curve();

  bool do_wheelie(const int& STEPS);
  void check_crashes(const int& STEPS);
  int find_non_crashing_hint();
  bool handle_tight_curves();

  float guess_accel (const float throttle);
  void remove_angle_excess (float& angle);
  int calc_steps();
  bool hint_is_behind(const int& HINT);
  int find_curve(const int& HINT);
  int find_check_hint();
  void setup_curve_handling();

public:
  AutoKart(const KartProperties *kart_properties, int position);

  void update (float delta) ;
  void reset  ();
};

#endif

/* EOF */
