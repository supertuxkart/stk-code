//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "physics_parameters.hpp"

PhysicsParameters* physicsParameters=0;

//-----------------------------------------------------------------------------
void PhysicsParameters::load(const std::string filename)
{

    KartProperties::load(filename, "physics");

    // Check that all necessary values are indeed set physics.data file


#define CHECK_NEG(  a,strA) if(a<-99) {                             \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n", \
                strA,filename.c_str());exit(-1);                    \
    }

    CHECK_NEG(m_corn_r,                  "m_corn_r"                     );
    CHECK_NEG(m_corn_f,                  "m_corn_f"                     );

    CHECK_NEG(m_mass,                    "mass"                         );
    CHECK_NEG(m_inertia,                 "m_inertia"                    );
    CHECK_NEG(m_tire_grip,               "tire-grip"                    );
    CHECK_NEG(m_height_cog,              "heightCOG"                    );
    CHECK_NEG(m_wheel_base,              "wheel-base"                   );
    CHECK_NEG(m_engine_power,            "engine-power"                 );
    CHECK_NEG(m_air_resistance,          "air-resistance"               );
    CHECK_NEG(m_max_steer_angle,         "max-steer-angle"              );
    CHECK_NEG(m_roll_resistance,         "roll-resistance"              );
    CHECK_NEG(m_magnet_range_sq,         "magnet-range"                 );
    CHECK_NEG(m_brake_factor,            "brake-factor"                 );
    CHECK_NEG(m_jump_impulse,            "jump-impulse"                 );

    CHECK_NEG(m_air_res_reduce[1],       "reduce-air-resistance-driver" );
    CHECK_NEG(m_air_res_reduce[2],       "reduce-air-resistance-racer"  );

    CHECK_NEG(m_wheelie_max_speed_ratio, "wheelie-max-speed-ratio"      );
    CHECK_NEG(m_wheelie_max_pitch,       "wheelie-max-pitch"            );
    CHECK_NEG(m_wheelie_pitch_rate,      "wheelie-pitch-rate"           );
    CHECK_NEG(m_wheelie_restore_rate,    "wheelie-restore-rate"         );
    CHECK_NEG(m_wheelie_speed_boost,     "wheelie-speed-boost"          );

    CHECK_NEG(m_magnet_min_range_sq,     "magnet-min-range"             );
    CHECK_NEG(m_parachute_friction,      "parachute-friction"           );
    CHECK_NEG(m_time_full_steer,         "time-full-steer"              );

    // Precompute some handy values to reduce work later
    m_magnet_range_sq    = m_magnet_range_sq   * m_magnet_range_sq;
    m_magnet_min_range_sq = m_magnet_min_range_sq * m_magnet_min_range_sq;
}   // load

/** Init all values with invalid defaults, which are tested later. This
 * guarantees that all parameters will indeed be initialised, and helps
 * finding typos.
 */
void PhysicsParameters::init_defaults()
{
    m_wheel_base  = m_height_cog      = m_magnet_min_range_sq = m_roll_resistance  = m_mass =
    m_corn_r      = m_air_resistance = m_tire_grip        = m_max_steer_angle  =
    m_corn_f      = m_inertia        = m_anvil_weight      = m_parachute_friction =
    m_engine_power = m_magnet_range_sq = m_jump_impulse      = m_brake_factor     =
    m_anvil_speed_factor = m_time_full_steer = m_wheelie_max_pitch =
    m_wheelie_max_speed_ratio = m_wheelie_pitch_rate = m_wheelie_restore_rate =
    m_wheelie_speed_boost = m_air_res_reduce[2] = m_air_res_reduce[1] = -99.9f;

    m_air_res_reduce[0]    = 1.0f;
}   // init_defaults

//-----------------------------------------------------------------------------
void PhysicsParameters::getAllData(const lisp::Lisp* lisp)
{

    // Get the values which are not part of the default KartProperties
    // ---------------------------------------------------------------
    lisp->get("anvil-weight",                 m_anvil_weight        );
    lisp->get("anvil-speed-factor",           m_anvil_speed_factor  );
    lisp->get("parachute-friction",           m_parachute_friction  );
    lisp->get("magnet-range",                 m_magnet_range_sq     );
    lisp->get("magnet-min-range",             m_magnet_min_range_sq );
    lisp->get("jump-impulse",                 m_jump_impulse        );
    lisp->get("reduce-air-resistance-racer",  m_air_res_reduce[2]   );
    lisp->get("reduce-air-resistance-driver", m_air_res_reduce[1]   );

    // Get the default KartProperties
    // ------------------------------
    KartProperties::getAllData(lisp->getLisp("kart-defaults"));
}   // getAllData
