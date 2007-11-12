//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_KARTPROPERTIES_H
#define HEADER_KARTPROPERTIES_H

#include <string>
#include <vector>
#include "lisp/lisp.hpp"
#include "no_copy.hpp"

class Material;
class ssgEntity;

class KartProperties : public NoCopy
{
private:

    Material* m_icon_material;
    ssgEntity* m_model;

protected:
    /* Display and gui */
    /* --------------- */
    std::string m_name;         // The human readable Name of the karts driver
    std::string m_ident;        // The computer readable-name of the karts driver
    std::string m_model_file;   // Filename of 3d model that is used for kart
    std::string m_icon_file;    // Filename of icon that represents the kart in
    // the statusbar and the character select screen
    std::string m_shadow_file;  // Filename of the image file that contains the
    // shadow for this kart
    float m_color[3];           // Color the represents the kart in the status
    // bar and on the track-view

    /* Physic properties */
    /* ----------------- */
    float m_kart_width;               // width of kart
    float m_kart_length;              // length of kart
    float m_mass;                     // weight of kart
    float m_air_resistance;           // air resistance
    float m_roll_resistance;          // rolling resistance etc
    float m_wheel_base;               // distance between front and read wheels
    float m_height_cog;               // height of center of gravity
    float m_engine_power;             // maximum force from engine
    float m_brake_factor;             // braking factor * engine_power = braking force
    float m_brake_force;              // braking force
    float m_tire_grip;                // grip of tires in longitudinal direction
    float m_max_steer_angle;          // maximum steering angle
    float m_time_full_steer;          // time for player karts to reach full steer angle
    float m_corn_f;
    float m_corn_r;
    float m_inertia;
    float m_wheelie_max_speed_ratio;  // percentage of maximum speed for wheelies
    float m_wheelie_max_pitch;        // maximum pitch for wheelies
    float m_wheelie_pitch_rate;       // rate/sec with which kart goes up
    float m_wheelie_restore_rate;     // rate/sec with which kart does down
    float m_wheelie_speed_boost;      // speed boost while doing a wheelie
    float m_wheelie_lean_recovery;
    float m_wheelie_balance_recovery;
    float m_wheelie_step;
    float m_wheelie_power_boost;      // increase in engine power

    //bullet physics data
    float m_suspension_stiffness;
    float m_wheel_damping_relaxation;
    float m_wheel_damping_compression;
    float m_friction_slip;
    float m_roll_influence;
    float m_wheel_radius;
    float m_wheel_width;
    float m_chassis_linear_damping;
    float m_chassis_angular_damping;
    float m_maximum_speed;
    float m_max_speed_reverse_ratio;
    float m_gravity_center_shift;
    float m_suspension_rest;
    // The following two vectors define at what ratio of the maximum speed what
    // gear is selected, e.g. 0.25 means: if speed <=0.25*maxSpeed --> gear 1,
    //                        0.5  means: if speed <=0.5 *maxSpeed --> gear 2
    // The next vector contains the increase in max power (to simulate different
    // gears), e.g. 2.5 as first entry means: 2.5*maxPower in gear 1
    std::vector<float> m_gear_switch_ratio;
    std::vector<float> m_gear_power_increase;


public:
    KartProperties   ();
    virtual      ~KartProperties   ();

    virtual void  init_defaults    ();
    virtual void  getAllData       (const lisp::Lisp* lisp);
    virtual void  load             (const std::string filename,
                                    const std::string node="tuxkart-kart",
                                    bool dont_load_models=false);

    Material*     getIconMaterial        () const {return m_icon_material;          }
    ssgEntity*    getModel               () const {return m_model;                  }
    const std::string& getName           () const {return m_name;                   }
    const char*   getIdent               () const {return m_ident.c_str();          }
    const char*   getShadowFile          () const {return m_shadow_file.c_str();    }
    const char*   getIconFile            () const {return m_icon_file.c_str();      }
    const sgVec3* getColor               () const {return &m_color;                 }
    float         getMass                () const {return m_mass;                   }
    float         getAirResistance       () const {return m_air_resistance;         }
    float         getRollResistance      () const {return m_roll_resistance;        }
    float         getMaxPower            () const {return m_engine_power;           }
    float         getTimeFullSteer       () const {return m_time_full_steer;        }
    float         getBrakeFactor         () const {return m_brake_factor;           }
    float         getBrakeForce          () const {return m_brake_force;            }
    float         getWheelBase           () const {return m_wheel_base;             }
    float         getHeightCOG           () const {return m_height_cog;             }
    float         getTireGrip            () const {return m_tire_grip;              }
    float         getMaxSteerAngle       () const {return m_max_steer_angle;        }
    float         getCornerStiffF        () const {return m_corn_f;                 }
    float         getCornerStiffR        () const {return m_corn_r;                 }
    float         getInertia             () const {return m_inertia;                }
    float         getMaxSpeedReverseRatio() const {return m_max_speed_reverse_ratio;}
    float         getWheelieMaxSpeedRatio() const {return m_wheelie_max_speed_ratio;}
    float         getWheelieMaxPitch     () const {return m_wheelie_max_pitch;      }
    float         getWheeliePitchRate    () const {return m_wheelie_pitch_rate;     }
    float         getWheelieRestoreRate  () const {return m_wheelie_restore_rate;   }
    float         getWheelieSpeedBoost   () const {return m_wheelie_speed_boost;    }
    float         getWheelieLeanRecovery () const {return m_wheelie_lean_recovery;  }
    float         getWheelieBalanceRecovery()const{return m_wheelie_balance_recovery;}
    float         getWheelieStep         () const {return m_wheelie_step;           }
    float         getWheeliePowerBoost   () const {return m_wheelie_power_boost;    }
    float         getKartLength          () const {return m_kart_length;            }
    float         getKartWidth           () const {return m_kart_width;             }

    //bullet physics get functions
    float getSuspensionStiffness    () const {return m_suspension_stiffness;     }
    float getWheelDampingRelaxation () const {return m_wheel_damping_relaxation; }
    float getWheelDampingCompression() const {return m_wheel_damping_compression;}
    float getFrictionSlip           () const {return m_friction_slip;            }
    float getRollInfluence          () const {return m_roll_influence;           }
    float getWheelRadius            () const {return m_wheel_radius;             }
    float getWheelWidth             () const {return m_wheel_width;              }
    float getChassisLinearDamping   () const {return m_chassis_linear_damping;   }
    float getChassisAngularDamping  () const {return m_chassis_angular_damping;  }
    float getMaximumSpeed           () const {return m_maximum_speed;            }
    float getGravityCenterShift     () const {return m_gravity_center_shift;     }
    float getSuspensionRest         () const {return m_suspension_rest;          }
    const std::vector<float>& 
          getGearSwitchRatio        () const {return m_gear_switch_ratio;        }
    const std::vector<float>& 
          getGearPowerIncrease      () const {return m_gear_power_increase;      }
};

#endif

/* EOF */
