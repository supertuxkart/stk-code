//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_KARTPROPERTIES_H
#define HEADER_KARTPROPERTIES_H

#include <string>
#include <vector>
#include "plib/ssg.h"
#include "vec3.hpp"
#include "karts/kart_model.hpp"
#include "lisp/lisp.hpp"

class Material;
class ssgEntity;

/** This class stores the properties of a kart. This includes size, name, 
 *  identifier, physical properties etc. It is atm also the base class for
 *  STKConfig, which stores the default values for all physics constants.
 */
class KartProperties
{
private:

    Material                *m_icon_material;  /**< The icon texture to use. */
    /** The kart model and wheels. It is mutable since the wheels of the
     *  KartModel can rotate and turn, but otherwise the kart_properties 
     *  object is const. */
    mutable KartModel        m_kart_model;
    std::vector<std::string> m_groups;         /**< List of all groups the kart
                                                    belongs to. */
    static float UNDEFINED;

protected:
    // Display and gui
    // --------------- 
    std::string m_name;         /**< The human readable Name of the kart
                                 *   driver. */
    std::string m_ident;        /**< The computer readable-name of the kart
                                 *   driver. */
    std::string m_icon_file;    /**< Filename of icon that represents the kart
                                 *   in the statusbar and the character select
                                 *   screen. */
    std::string m_shadow_file;  /**< Filename of the image file that contains 
                                 *   the shadow for this kart. */
    Vec3 m_color;               /**< Color the represents the kart in the status
                                 *   bar and on the track-view. */

    // Physic properties
    // -----------------
    float m_mass;                     /**< Weight of kart.  */
    float m_engine_power;             /**< Maximum force from engine. */
    float m_brake_factor;             /**< Braking factor * engine_power = 
                                       *   braking force. */
    float m_time_full_steer;          /**< Time for player karts to reach full
                                       *   steer angle. */
    float m_wheelie_max_speed_ratio;  /**< Percentage of maximum speed for 
                                       *   wheelies. */
    float m_wheelie_max_pitch;        /**< Maximum pitch for wheelies. */
    float m_wheelie_pitch_rate;       /**< Rate/sec with which kart goes up. */
    float m_wheelie_restore_rate;     /**< Rate/sec with which kart does down.*/
    float m_wheelie_speed_boost;      /**< Speed boost while doing a wheelie. */
    float m_wheelie_power_boost;      /**< Increase in engine power. */

    float m_min_speed_turn, m_angle_at_min;  /**< Speed dependent steering: 
                                              *   maximum speed to use. */
    float m_max_speed_turn, m_angle_at_max;  /**< Turn angle at lowest speed 
                                              *   etc. */
    float m_speed_angle_increase;

    ssgEntity  *m_wheel_model[4];      /**< The four wheel models.           */
    std::string m_wheel_filename[4];   /**< Filename of the wheel models.    */
                                       /**  Radius of the graphical wheels.  */
    float       m_wheel_graphics_radius[4];  
    ssgTransform 
               *m_wheel_transform[4];  /**< The transform for the wheels, used
                                        *   to rotate the wheels and display
                                        *   the suspension in the race.      */
    // bullet physics data 
    // -------------------
    float m_suspension_stiffness;
    float m_wheel_damping_relaxation;
    float m_wheel_damping_compression;
    float m_friction_slip;
    float m_roll_influence;
    float m_wheel_radius;
    float m_chassis_linear_damping;
    float m_chassis_angular_damping;
    float m_maximum_speed;
    float m_max_speed_reverse_ratio;
    Vec3  m_gravity_center_shift;    /**< Shift of center of gravity. */
    float m_track_connection_accel;  /**< Artifical acceleration that pulls a 
                                      *   kart down onto the track if one axis
                                      *   loses contact with the track. */
    float m_suspension_rest;
	float m_suspension_travel_cm;
    float m_jump_velocity;            // z velocity set when jumping
    float m_upright_tolerance;
    float m_upright_max_force;

    // Camera related setting
    // ----------------------
    float m_camera_max_accel;         // maximum acceleration of camera
    float m_camera_max_brake;         // maximum braking of camera
    float m_camera_distance;          // distance of normal camera from kart
    
    /** The following two vectors define at what ratio of the maximum speed what
     * gear is selected.  E.g. 0.25 means: if speed <=0.25*maxSpeed --> gear 1,
     *                         0.5  means: if speed <=0.5 *maxSpeed --> gear 2
     * The next vector contains the increase in max power (to simulate different
     * gears), e.g. 2.5 as first entry means: 2.5*maxPower in gear 1 */
    std::vector<float> m_gear_switch_ratio,
                       m_gear_power_increase;


public:
          KartProperties    ();
         ~KartProperties    ();
    void  getAllData        (const lisp::Lisp* lisp);
    void  load              (const std::string &filename,
                             const std::string &node="tuxkart-kart",
                             bool dont_load_models=false);
    void  checkAllSet(const std::string &filename);

    float getMaxSteerAngle          (float speed) const;
    Material*     getIconMaterial   () const {return m_icon_material;          }
    /** Returns a pointer to the KartModel object. */
    KartModel*    getKartModel      () const {return &m_kart_model;            }
    const std::string& getName      () const {return m_name;                   }
    const std::string& getIdent     () const {return m_ident;                  }
    const std::string& getShadowFile() const {return m_shadow_file;            }
    const std::string& getIconFile  () const {return m_icon_file;              }
    const Vec3   &getColor          () const {return m_color;                  }
    const std::vector<std::string>&
                  getGroups         () const {return m_groups;                   }
    float getMass                   () const {return m_mass;                     }
    float getMaxPower               () const {return m_engine_power;             }
    float getTimeFullSteer          () const {return m_time_full_steer;          }
    float getBrakeFactor            () const {return m_brake_factor;             }
    float getMaxSpeedReverseRatio   () const {return m_max_speed_reverse_ratio;  }
    float getWheelieMaxSpeedRatio   () const {return m_wheelie_max_speed_ratio;  }
    float getWheelieMaxPitch        () const {return m_wheelie_max_pitch;        }
    float getWheeliePitchRate       () const {return m_wheelie_pitch_rate;       }
    float getWheelieRestoreRate     () const {return m_wheelie_restore_rate;     }
    float getWheelieSpeedBoost      () const {return m_wheelie_speed_boost;      }
    float getWheeliePowerBoost      () const {return m_wheelie_power_boost;      }

    //bullet physics get functions
    float getSuspensionStiffness    () const {return m_suspension_stiffness;     }
    float getWheelDampingRelaxation () const {return m_wheel_damping_relaxation; }
    float getWheelDampingCompression() const {return m_wheel_damping_compression;}
    float getFrictionSlip           () const {return m_friction_slip;            }
    float getRollInfluence          () const {return m_roll_influence;           }
    float getWheelRadius            () const {return m_wheel_radius;             }
    float getChassisLinearDamping   () const {return m_chassis_linear_damping;   }
    float getChassisAngularDamping  () const {return m_chassis_angular_damping;  }
    float getMaximumSpeed           () const {return m_maximum_speed;            }
    const Vec3& getGravityCenterShift() const {return m_gravity_center_shift;    }
    float getSuspensionRest         () const {return m_suspension_rest;          }
    float getSuspensionTravelCM     () const {return m_suspension_travel_cm;     }
    float getJumpVelocity           () const {return m_jump_velocity;            }
    float getUprightTolerance       () const {return m_upright_tolerance;        }
    float getUprightMaxForce        () const {return m_upright_max_force;        }
    float getTrackConnectionAccel   () const {return m_track_connection_accel;   }
    const std::vector<float>& 
          getGearSwitchRatio        () const {return m_gear_switch_ratio;        }
    const std::vector<float>& 
          getGearPowerIncrease      () const {return m_gear_power_increase;      }
    float getCameraMaxAccel         () const {return m_camera_max_accel;         }
    float getCameraMaxBrake         () const {return m_camera_max_brake;         }
    float getCameraDistance         () const {return m_camera_distance;          }
};

#endif

/* EOF */
