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

#ifndef HEADER_KART_PROPERTIES_HPP
#define HEADER_KART_PROPERTIES_HPP

#include <string>
#include <vector>

#include "irrlicht.h"
using namespace irr;

#include "audio/sfx_manager.hpp"
#include "karts/kart_model.hpp"
#include "lisp/lisp.hpp"
#include "io/xml_node.hpp"
#include "race/race_manager.hpp"
#include "utils/vec3.hpp"

class Material;
class XMLNode;

/** This class stores the properties of a kart. This includes size, name,
 *  identifier, physical properties etc. It is atm also the base class for
 *  STKConfig, which stores the default values for all physics constants.
 */
class KartProperties
{
private:
    /** Base directory for this kart. */
    std::string  m_root;
    Material                *m_icon_material;  /**< The icon texture to use. */
    /** The kart model and wheels. It is mutable since the wheels of the
     *  KartModel can rotate and turn, but otherwise the kart_properties
     *  object is const. */
    mutable KartModel        m_kart_model;
    std::vector<std::string> m_groups;         /**< List of all groups the kart
                                                    belongs to. */
    static float UNDEFINED;
    float m_speed_angle_increase;     /**< Increase of turn angle with speed. */
    int   m_version;                  /**< Version of the .kart file.         */

    // SFX files
    // ---------------
    std::vector<int> m_custom_sfx_id;     /**< Vector of custom SFX ids */

    // Display and gui
    // ---------------
    std::string m_name;               /**< The human readable Name of the kart
                                       *   driver. */
    std::string m_ident;              /**< The computer readable-name of the
                                       *   kart driver. */
    std::string m_icon_file;          /**< Filename of icon that represents the
                                       *   kart in the statusbar and the
                                       *   character select screen. */
    std::string m_shadow_file;        /**< Filename of the image file that
                                       *   contains the shadow for this kart.*/
    video::SColor m_color;            /**< Color the represents the kart in the
                                       *   status bar and on the track-view. */
    int  m_shape;                     /**< Number of vertices in polygon when
                                       *   drawing the dot on the mini map. */
    // Physic properties
    // -----------------
    float m_mass;                     /**< Weight of kart.  */
    float m_engine_power[3];          /**< Maximum force from engine for each
                                       *   difficulty. */
    float m_brake_factor;             /**< Braking factor * engine_power =
                                       *   braking force. */
    float m_time_full_steer;          /**< Time for player karts to reach full
                                       *   steer angle. */
    float m_time_full_steer_ai;       /**< Time for  AI karts to reach full
                                       *   steer angle (used to reduce shaking
                                       *   of karts). */
    float m_min_speed_turn,           /**< Speed for smallest turn radius. */
          m_angle_at_min,             /**< Steering angle for minimal turn
                                           radius. Computed from radius and
                                           kart length. */
          m_min_radius;               /**< Smallest turn radius. */
    float m_max_speed_turn,           /**< Speed for largest turn radius. */
          m_angle_at_max,             /**< Steering angle for maximum turn
                                           radius. Computed from radius and
                                           kart length. */
          m_max_radius;               /**< Largest turn radius. */

    std::string m_wheel_filename[4];   /**< Filename of the wheel models.    */
                                       /**  Radius of the graphical wheels.  */
    float       m_wheel_graphics_radius[4];
    float       m_rubber_band_max_length;/**< Max. length of plunger rubber band.*/
    float       m_rubber_band_force;   /**< Force of an attached rubber band.*/
    float       m_rubber_band_duration;/**< Duration a rubber band works.    */
    float       m_wheel_base;          /**< Wheel base of the kart.          */
    float       m_nitro_power_boost;   /**< Nitro power boost.               */
    SFXManager::SFXType
                m_engine_sfx_type;     /**< Engine sound effect.         */

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
    float m_max_speed[3];
    float m_max_speed_reverse_ratio;
    Vec3  m_gravity_center_shift;    /**< Shift of center of gravity. */
    float m_track_connection_accel;  /**< Artifical acceleration that pulls a
                                      *   kart down onto the track if one axis
                                      *   loses contact with the track. */
    float m_suspension_rest;
	float m_suspension_travel_cm;
    float m_jump_velocity;            // z velocity set when jumping
    float m_z_rescue_offset;          // z offset after rescue
    float m_upright_tolerance;
    float m_upright_max_force;

    float m_skid_visual;              /**< Additional rotation of 3d model
                                       *   when skidding. */
    float m_slipstream_length;        /**< How far behind a kart slipstreaming
                                       *   is effective. */
    float m_slipstream_time;          /**< Time after which sstream has maxium
                                       *   benefit. */
    float m_slipstream_add_power;     /**< Additional power due to sstreaming. */
    float m_skid_max;                 /**< Maximal increase of steering when
                                       *   skidding. */
    float m_skid_increase;            /**< Skidding is multiplied by this when
                                       *   skidding to increase to
                                       *   m_skid_increase. */
    float m_skid_decrease;            /**< Skidding is multiplied by this when
                                       *   not skidding to decrease to 1.0. */
    float m_time_till_max_skid;       /**< Time till maximum skidding is
                                       *   reached. */
    bool  m_has_skidmarks;            /**< Kart leaves skid marks. */

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

    void  load              (const std::string &filename,
                             const std::string &node="tuxkart-kart");

public:
          KartProperties    (const std::string &filename="");
         ~KartProperties    ();
    void  getAllData        (const lisp::Lisp* lisp);
    void  getAllData        (const XMLNode * root);
    void  checkAllSet(const std::string &filename);

    float getMaxSteerAngle          (float speed) const;
    Material*     getIconMaterial   () const {return m_icon_material;            }
    /** Returns a pointer to the KartModel object. */
    KartModel*    getKartModel      () const {return &m_kart_model;              }
    const std::string& getName      () const {return m_name;                     }
    const std::string& getIdent     () const {return m_ident;                    }
    const std::string& getShadowFile() const {return m_shadow_file;              }
    const std::string& getIconFile  () const {return m_icon_file;                }
    const int          getCustomSfxId (SFXManager::CustomSFX type) 
                                       const {return m_custom_sfx_id[type];      }

    /** Returns the version of the .kart file. */
    int   getVersion                () const {return m_version;                  }
    /** Returns the dot color to use for this kart in the race gui. */
    const video::SColor &getColor   () const {return m_color;                    }
    /** Returns the number of edges for the polygon used to draw the dot of
     *  this kart on the mini map of the race gui. */
    int   getShape                  () const {return m_shape;                    }
    const std::vector<std::string>&
                  getGroups         () const {return m_groups;                   }
    float getMass                   () const {return m_mass;                     }
    float getMaxPower               () const {return m_engine_power[race_manager->getDifficulty()];}
    float getTimeFullSteer          () const {return m_time_full_steer;          }
    float getTimeFullSteerAI        () const {return m_time_full_steer_ai;       }
    float getBrakeFactor            () const {return m_brake_factor;             }
    float getMaxSpeedReverseRatio   () const {return m_max_speed_reverse_ratio;  }
    SFXManager::SFXType getEngineSfxType()
                                       const {return m_engine_sfx_type;          }

    //bullet physics get functions
    float getSuspensionStiffness    () const {return m_suspension_stiffness;     }
    float getWheelDampingRelaxation () const {return m_wheel_damping_relaxation; }
    float getWheelDampingCompression() const {return m_wheel_damping_compression;}
    float getFrictionSlip           () const {return m_friction_slip;            }
    float getRollInfluence          () const {return m_roll_influence;           }
    float getWheelRadius            () const {return m_wheel_radius;             }
    float getWheelBase              () const {return m_wheel_base;               }
    float getChassisLinearDamping   () const {return m_chassis_linear_damping;   }
    float getChassisAngularDamping  () const {return m_chassis_angular_damping;  }
    /** Returns the maximum speed dependent on the difficult level. */
    float getMaxSpeed               () const {return
                                      m_max_speed[race_manager->getDifficulty()];}
    /** Returns the nitro power boost. */
    float getNitroPowerBoost        () const {return m_nitro_power_boost;        }
    const Vec3&getGravityCenterShift() const {return m_gravity_center_shift;     }
    float getSuspensionRest         () const {return m_suspension_rest;          }
    float getSuspensionTravelCM     () const {return m_suspension_travel_cm;     }
    float getJumpVelocity           () const {return m_jump_velocity;            }
    float getZRescueOffset          () const {return m_z_rescue_offset;          }
    float getUprightTolerance       () const {return m_upright_tolerance;        }
    float getUprightMaxForce        () const {return m_upright_max_force;        }
    float getTrackConnectionAccel   () const {return m_track_connection_accel;   }
    /** Returns the maximum length of a rubber band before it breaks. */
    float getRubberBandMaxLength    () const {return m_rubber_band_max_length;   }
    /** Returns force a rubber band has when attached to a kart. */
    float getRubberBandForce        () const {return m_rubber_band_force;        }
    /** Returns the duration a rubber band is active for. */
    float getRubberBandDuration     () const {return m_rubber_band_duration;     }
    /** Returns additional rotation of 3d model when skidding. */
    float getSkidVisual             () const {return m_skid_visual;              }
    /** Returns how far behind a kart slipstreaming works. */
    float getSlipstreamLength       () const {return m_slipstream_length;        }
    /** Returns time after which slipstream has maximum effect. */
    float getSlipstreamTime         () const {return m_slipstream_time;          }
    /** Returns additional power due to slipstreaming. */
    float getSlipstreamAddPower     () const {return m_slipstream_add_power;     }
    /** Returns the maximum factor by which the steering angle can be increased. */
    float getMaxSkid                () const {return m_skid_max;                 }
    /** Returns the factor by which m_skidding is multiplied when the kart is
     *  skidding to increase it to the maximum. */
    float getSkidIncrease           () const {return m_skid_increase;            }
    /** Returns the factor by which m_skidding is multiplied when the kart is
     *  not skidding to decrease it back to 1.0f . */
    float getSkidDecrease           () const {return m_skid_decrease;            }
    /** Returns the time (in seconds) of drifting till the maximum skidding
     *  is reached. */
    float getTimeTillMaxSkid        () const {return m_time_till_max_skid;       }
    /** Returns if the kart leaves skidmarks or not. */
    bool hasSkidmarks               () const {return m_has_skidmarks;            }
    const std::vector<float>&
          getGearSwitchRatio        () const {return m_gear_switch_ratio;        }
    const std::vector<float>&
          getGearPowerIncrease      () const {return m_gear_power_increase;      }
    float getCameraMaxAccel         () const {return m_camera_max_accel;         }
    float getCameraMaxBrake         () const {return m_camera_max_brake;         }
    float getCameraDistance         () const {return m_camera_distance;          }
    /** Returns the full path where the files for this kart are stored. */
    const std::string& getKartDir   () const {return m_root;                     }
};

#endif

/* EOF */
