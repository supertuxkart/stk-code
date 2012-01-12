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

#include <SColor.h>
#include <irrString.h>
namespace irr
{
    namespace video { class ITexture; }
}
using namespace irr;

#include "audio/sfx_manager.hpp"
#include "karts/kart_model.hpp"
#include "io/xml_node.hpp"
#include "race/race_manager.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class Material;
class XMLNode;

/** 
 *  \brief This class stores the properties of a kart.
 *  This includes size, name, identifier, physical properties etc.
 *  It is atm also the base class for STKConfig, which stores the default values
 *  for all physics constants.
 *  Note that KartProperies is copied (when setting the default values from 
 *  stk_config.
 *
 * \ingroup karts
 */
class KartProperties
{
private:
    /** Base directory for this kart. */
    std::string              m_root;

    /** The absolute path of the icon texture to use. */
    Material                *m_icon_material;

    /** The minimap icon file. */
    std::string              m_minimap_icon_file;

    /** The texture to use in the minimap. If not defined, a simple
     *  color dot is used. */
    video::ITexture         *m_minimap_icon;

    /** The kart model and wheels. It is mutable since the wheels of the
     *  KartModel can rotate and turn, and animations are played, but otherwise
     *  the kart_properties object is const. */
    mutable KartModel       *m_kart_model;

    /** List of all groups the kart belongs to. */
    std::vector<std::string> m_groups;
    static float UNDEFINED;

    /** Version of the .kart file. */
    int   m_version;

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
    float m_shadow_scale;             /**< Scale of the shadow plane
                                       *   for this kart.*/
    float m_shadow_x_offset;          /**< X offset of the shadow plane
                                       *   for this kart.*/
    float m_shadow_y_offset;          /**< Y offset of the shadow plane
                                       *   for this kart.*/
    video::ITexture *m_shadow_texture;/**< The texture with the shadow. */
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
    /** Stores the speeds at which the turn angle changes. */
    std::vector<float> m_turn_speed;

    /** Stores the turn angle at the corresponding turn speed. */
    std::vector<float> m_turn_angle_at_speed;

    /** Increase of turn angle with speed. */
    std::vector<float> m_speed_angle_increase;

    /** If != 0 a bevelled box shape is used by using a point cloud as a 
     *  collision shape. */
    Vec3  m_bevel_factor;

    /** Time a kart is moved upwards after when it is rescued. */
    float m_rescue_time;

    /** Distance the kart is raised before dropped. */
    float m_rescue_height;

    /** Time an animated explosion is shown. Longer = more delay for kart. */
    float m_explosion_time;

    /** How far away from an explosion karts will still be affected. */
    float m_explosion_radius;

    /** How long a kart is invulnerable after it is hit by an explosion. */
    float m_explosion_invulnerability_time;

    /** Duration a zipper is active. */
    float m_zipper_time;
    
    /** Fade out time for a zipper. */
    float m_zipper_fade_out_time;

    /** Additional force added to the acceleration. */
    float m_zipper_force;
    
    /** Initial one time speed gain. */
    float m_zipper_speed_gain;
    
    /** Absolute increase of the kart's maximum speed (in m/s). */
    float m_zipper_max_speed_increase;

    /** Vertical offset after rescue. */
    float m_rescue_vert_offset;

    /** Filename of the wheel models. */
    std::string m_wheel_filename[4];
    /**  Radius of the graphical wheels.  */
    float       m_wheel_graphics_radius[4];
    /** Max. length of plunger rubber band. */
    float       m_rubber_band_max_length;
    /** Force of an attached rubber band. */
    /** Duration a rubber band works. */
    float       m_rubber_band_force;
    /** How long the rubber band will fly. */
    float       m_rubber_band_duration;
    /** Increase of maximum speed of the kart when the rubber band pulls. */
    float       m_rubber_band_speed_increase;
    /** Fade out time when the rubber band is removed. */
    float       m_rubber_band_fade_out_time;
     /**Duration of plunger in face depending on difficulty. */
    float       m_plunger_in_face_duration[3];
    /** Wheel base of the kart. */
    float       m_wheel_base;
    /** Nitro power boost. */
    float       m_nitro_power_boost;
    /** Nitro consumption. */
    float       m_nitro_consumption;
    /** Nitro amount for small bottle. */
    float       m_nitro_small_container;
    /** Nitro amount for big bittle. */
    float       m_nitro_big_container;
    /* How much the speed of a kart might exceed its maximum speed (in m/s). */
    float       m_nitro_max_speed_increase;
    /**  How long the increased nitro max speed will be valid after 
     *  the kart stops using nitro (and the fade-out-time starts). */
    float       m_nitro_duration;
    /** Duration during which the increased maximum speed 
     *  due to nitro fades out. */
    float       m_nitro_fade_out_time;
    /** Square of the maximum distance a swatter can operate. */
    float       m_swatter_distance2;
    /** How long the swatter lasts. */
    float       m_swatter_duration;
    /** How long a kart will remain squashed. */
    float       m_squash_duration;
    /** The slowdown to apply while a kart is squashed. The new maxspeed
     *  is max_speed*m_squash_slowdown. */
    float       m_squash_slowdown;

    /** Engine sound effect. */
    std::string m_engine_sfx_type;

    // bullet physics data
    // -------------------
    float m_suspension_stiffness;
    float m_wheel_damping_relaxation;
    float m_wheel_damping_compression;
    float m_max_suspension_force;
    float m_friction_slip;
    float m_roll_influence;
    float m_wheel_radius;

    /** An impulse pushing the kart down which is proportional to speed. So
     *  the actual impulse is  speed * m_downward_impulse_factor. Set it to
     *  0 to disable completely. Based on
     *  http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=6059\
     *  &p=21240&hilit=vehicle#p21240  */
    float m_downward_impulse_factor;

    /** Artifical acceleration that pulls a kart down onto the track if one 
     *  axis loses contact with the track. */
    float m_track_connection_accel;

    /** Linear damping of the chassis to prevent it from toppling over. */
    float m_chassis_linear_damping;

    /** Angular damping to prevent it from turning too easily. */
    float m_chassis_angular_damping;

    float m_max_speed[3];
    float m_max_speed_reverse_ratio;

    /** Shift of center of gravity. */
    Vec3  m_gravity_center_shift;

    /** The suspension reaction is dampened to reach an exponential behaviour.
     *  See http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=7369\
     *  &p=25236&hilit=vehicle#p25236  for details. */
    bool  m_exp_spring_response;

    float m_suspension_rest;
    float m_suspension_travel_cm;

    /** An additional artifical side-impulse that pushes the slower kart
     *  out of the way of the faster kart in case of a frontal collision. */
    float m_collision_side_impulse;

    /** An additiojnal artificial impulse that pushes two karts in a 
     *  side-side collision away from each other. */
    float m_collision_impulse;

    /** How long the collision impulse should be applied. */
    float m_collision_impulse_time;

    /** The restitution factor to be used in collsions for this kart. */
    float m_restitution;

    float m_upright_tolerance;
    float m_upright_max_force;

    /** How far behind a kart slipstreaming is effective. */
    float m_slipstream_length;
    /** Time after which sstream gives a bonus. */
    float m_slipstream_collect_time;
    /** Time slip-stream bonus is effective. */
    float m_slipstream_use_time;
    /** Additional power due to sstreaming. */
    float m_slipstream_add_power;
    /** Minimum speed for slipstream to take effect. */
    float m_slipstream_min_speed;
    /** How much the speed of the kart might exceed its 
     *  normal maximum speed. */
    float m_slipstream_max_speed_increase;
    /** How long the higher speed lasts after slipstream stopped working. */
    float m_slipstream_duration;
    /** How long the slip stream speed increase will gradually be reduced. */
    float m_slipstream_fade_out_time;

    /** Maximal increase of steering when skidding. */
    float m_skid_max;
    /** Skidding is multiplied by this when skidding 
     *  to increase to m_skid_increase. */
    float m_skid_increase;
    /** Skidding is multiplied by this when not skidding to decrease to 1.0. */
    float m_skid_decrease;
    /**< Time till maximum skidding is reached. */
    float m_time_till_max_skid;
    /** Additional rotation of 3d model when skidding. */
    float m_skid_visual;              
    /** How long it takes for visual skid to reach maximum. */
    float m_skid_visual_time;
    /** Angular velocity to be applied when skidding. */
    float m_skid_angular_velocity;

    /** This factor is used to determine how much the chassis of a kart
     *  should rotate to match the graphical view. A factor of 1 is 
     *  identical, a smaller factor will rotate the kart less (which might
     *  feel better). */
    float m_post_skid_rotate_factor;

    /** Kart leaves skid marks. */
    bool  m_has_skidmarks;

    /** Time of skidding before you get a bonus boost. It's possible to
     *  define more than one time, i.e. longer skidding gives more bonus. */
    std::vector<float> m_skid_time_till_bonus;

    /** How much additional speed a kart gets when skidding. It's possible to
     *  define more than one force, i.e. longer skidding gives more bonus. */
    std::vector<float> m_skid_bonus_force;

    /** How long the bonus will last. It's possible to define more than one 
    *   time, i.e. longer skidding gives more bonus. */
    std::vector<float> m_skid_bonus_time;

    /** Make the AI to steer at slightly different points to make it less
     *  likely that the AI creates 'trains' - the kart behind getting 
     *  slipstream. The variation should be a value between 0 (no variation,
     *  all karts steer to the same driveline points) and 1 (karts will aim
     *  all the way to the very edge of the drivelines). */
    float m_ai_steering_variation;

    float m_camera_distance;          /**< Distance of normal camera from kart.*/
    float m_camera_forward_up_angle;  /**< Up angle of the camera in relation to
                                           the pitch of the kart when driving 
                                           forwards.                      */
    float m_camera_backward_up_angle; /**< Up angle of the camera in relation to
                                           the pitch of the kart when driving 
                                           backwards.                      */

    /** The following two vectors define at what ratio of the maximum speed what
     * gear is selected.  E.g. 0.25 means: if speed <=0.25*maxSpeed --> gear 1,
     *                         0.5  means: if speed <=0.5 *maxSpeed --> gear 2 */
    std::vector<float> m_gear_switch_ratio;
    /** This vector contains the increase in max power (to simulate different
     *  gears), e.g. 2.5 as first entry means: 2.5*maxPower in gear 1. See
       m_gear_switch_ratio). */
    std::vector<float> m_gear_power_increase;

    /** If the kart starts within the specified time at index I after 'go', 
     *  it receives the speed boost from m_startup_boost[I]. */
    std::vector<float> m_startup_times;

    /** The startup boost is the kart starts fast enough. */
    std::vector<float> m_startup_boost;


    void  load              (const std::string &filename,
                             const std::string &node);
	/** If the kart is supposed to have random wheel rotation at start. */
	bool m_has_rand_wheels;

public:
          KartProperties    (const std::string &filename="");
         ~KartProperties    ();
    void  getAllData        (const XMLNode * root);
    void  checkAllSet       (const std::string &filename);
    float getStartupBoost   () const;

    /** Returns the maximum steering angle (depending on speed). */
    float getMaxSteerAngle  (float speed) const;
    void  getSkidBonus(float t, float *bonus_time, float *bonus_force) const;

    /** Returns the material for the kart icons. */
    Material*     getIconMaterial    () const {return m_icon_material;        }

    /** Returns the texture to use in the minimap, or NULL if not defined. */
    video::ITexture *getMinimapIcon  () const {return m_minimap_icon;         }

    /** Returns a pointer to the KartModel object. */
    KartModel*    getKartModelCopy   () const 
                                            {return m_kart_model->makeCopy(); }

    /** Returns a pointer to the main KartModel object. This copy
     *  should not be modified, not attachModel be called on it. */
    const KartModel& getMasterKartModel() const {return *m_kart_model;        }

    /** Returns the name of this kart. 
        \note Pass it through fridibi as needed, this is the LTR name
      */
    const wchar_t* getName() const {return translations->w_gettext(m_name.c_str()); }

    const std::string getNonTranslatedName() const {return m_name;}

    /** Returns the internal identifier of this kart. */
    const std::string& getIdent      () const {return m_ident;                }

    /** Returns the shadow texture to use. */
    video::ITexture *getShadowTexture() const {return m_shadow_texture;       }

    /** Returns the absolute path of the icon file of this kart. */
    const std::string& getAbsoluteIconFile() const      { return m_icon_file; }

    /** Returns custom sound effects for this kart. */
    const int          getCustomSfxId (SFXManager::CustomSFX type) 
                                       const  {return m_custom_sfx_id[type];  }

    /** Returns the version of the .kart file. */
    int   getVersion                () const {return m_version;               }

    /** Returns the dot color to use for this kart in the race gui. */
    const video::SColor &getColor   () const {return m_color;                 }

    /** Returns the number of edges for the polygon used to draw the dot of
     *  this kart on the mini map of the race gui. */
    int   getShape                  () const {return m_shape;                 }

    /** REturns the list of groups this kart belongs to. */
    const std::vector<std::string>&
                  getGroups         () const {return m_groups;                }
    /** Returns the mass of this kart. */
    float getMass                   () const {return m_mass;                  }
    /** Returns the maximum engine power depending on difficulty. */
    float getMaxPower               () const 
                        {return m_engine_power[race_manager->getDifficulty()];}

    /** Returns the time the kart needs to fully steer in one direction from 
     *  steering straight. */
    float getTimeFullSteer          () const {return m_time_full_steer;       }

    /** Returns how long the AI should need to steer in a direction. This 
     *  avoids that the AI has an advantage by being able to change steering 
     *  to quickly (e.g. counteracting pushes). */
    float getTimeFullSteerAI        () const {return m_time_full_steer_ai;    }

    /** Get braking information. */
    float getBrakeFactor            () const {return m_brake_factor;          }

    /** Get maximum reverse speed ratio. */
    float getMaxSpeedReverseRatio   () const 
                                          {return m_max_speed_reverse_ratio;  }

    /** Returns the engine type (used to change sfx depending on kart size). */
    const std::string& getEngineSfxType    () const {return m_engine_sfx_type;}

    // Bullet physics get functions
    //-----------------------------
    /** Returns the suspension stiffness. */
    float getSuspensionStiffness    () const {return m_suspension_stiffness;  }

    /** Returns damping relaxation. */
    float getWheelDampingRelaxation () const 
                                          {return m_wheel_damping_relaxation; }

    /** Returns the wheel damping compression. */
    float getWheelDampingCompression() const 
                                          {return m_wheel_damping_compression;}

    /** Returns maximum suspension force. */
    float getMaxSuspensionForce() const {return m_max_suspension_force; }

    /** Returns friction slip. */
    float getFrictionSlip           () const {return m_friction_slip;         }

    /** Returns roll influence. */
    float getRollInfluence          () const {return m_roll_influence;        }

    /** Returns wheel radius. */
    float getWheelRadius            () const {return m_wheel_radius;          }

    /** Returns the wheel base (distance front to rear axis). */
    float getWheelBase              () const {return m_wheel_base;            }

    /** Returns linear damping of chassis. */
    float getChassisLinearDamping   () const {return m_chassis_linear_damping;}

    /** Returns angular damping of chassis. */
    float getChassisAngularDamping  () const 
                                           {return m_chassis_angular_damping; }

    /** Artifical downward impulse every frame. */
    float getDownwardImpulseFactor() const { return m_downward_impulse_factor;}

    /** Returns artificial acceleration to keep wheels on track. */
    float getTrackConnectionAccel   () const {return m_track_connection_accel;}

    /** Returns the maximum speed dependent on the difficult level. */
    float getMaxSpeed               () const {return
                                   m_max_speed[race_manager->getDifficulty()];}

    /** Returns the nitro power boost. */
    float getNitroPowerBoost        () const {return m_nitro_power_boost;     }

    /** Returns the nitro consumption. */
    float getNitroConsumption       () const {return m_nitro_consumption;     }

    /** Returns the amount of nitro for a small container. */
    float getNitroSmallContainer    () const {return m_nitro_small_container; }

    /** Returns the amount of nitro for a big container. */
    float getNitroBigContainer      () const {return m_nitro_big_container;   }

    /** Returns the increase of maximum speed due to nitro. */
    float getNitroMaxSpeedIncrease  () const 
                                          {return m_nitro_max_speed_increase; }

    /** Returns how long the increased nitro max speed will be valid after
     *  the kart stops using nitro (and the fade-out-time starts). */
    float getNitroDuration          () const {return m_nitro_duration;        }

    /** Returns the duration during which the increased maximum speed 
     *  due to nitro fades out. */
    float getNitroFadeOutTime       () const {return m_nitro_fade_out_time;   }

    /** Returns a shift of the center of mass (lowering the center of mass 
     *  makes the karts more stable. */
    const Vec3&getGravityCenterShift() const {return m_gravity_center_shift;  }

    /** Retusn suspension rest length. */
    float getSuspensionRest         () const {return m_suspension_rest;       }

    /** Returns the amount the suspension can extend. */
    float getSuspensionTravelCM     () const {return m_suspension_travel_cm;  }

    /** Returns if the spring should be exponentially dampened. */
    bool getExpSpringResponse() const {return m_exp_spring_response; }

    /** Returns the (artificial) collision side impulse this kart will apply
     *  to a slower kart in case of a collision. */
    float getCollisionSideImpulse   () const {return m_collision_side_impulse;}

    /** Returns the (artificial) collision impulse this kart will apply
     *  to another kart in case of a non-frontal collision. */
    float getCollisionImpulse       () const {return m_collision_impulse;}

    /** Returns how long the collision impulse should be applied. */
    float getCollisionImpulseTime() const { return m_collision_impulse_time;}

    /** Returns the restitution factor for this kart. */
    float getRestitution            () const { return m_restitution; }

    /** Returns the vertical offset when rescuing karts to avoid karts being
     *  rescued in (or under) the track. */
    float getVertRescueOffset       () const {return m_rescue_vert_offset;    }

    /** Returns the time a kart is rised during a rescue. */
    float getRescueTime             () const {return m_rescue_time;           }

    /** Returns the height a kart is moved to during a rescue. */
    float getRescueHeight           () const {return m_rescue_height;         }

    /** Returns the time an explosion animation is shown. */
    float getExplosionTime          () const {return m_explosion_time;        }

    /** Returns the height of the explosion animation. */
    float getExplosionRadius        () const {return m_explosion_radius;      }

    /** Returns how long a kart is invulnerable after being hit by an 
        explosion. */
    float getExplosionInvulnerabilityTime() const 
                                   { return m_explosion_invulnerability_time; }

    /** Returns how much a kart can roll/pitch before the upright constraint
     *  counteracts. */
    float getUprightTolerance       () const {return m_upright_tolerance;     }

    /** Returns the maximum value of the upright counteracting force. */
    float getUprightMaxForce        () const {return m_upright_max_force;     }

    /** Returns the maximum length of a rubber band before it breaks. */
    float getRubberBandMaxLength    () const {return m_rubber_band_max_length;}

    /** Returns force a rubber band has when attached to a kart. */
    float getRubberBandForce        () const {return m_rubber_band_force;     }

    /** Returns the duration a rubber band is active for. */
    float getRubberBandDuration     () const {return m_rubber_band_duration;  }

    /** Returns the increase of maximum speed while a rubber band is 
     *  pulling. */
    float getRubberBandSpeedIncrease() const {return m_rubber_band_speed_increase;}

    /** Return the fade out time once a rubber band is removed. */
    float getRubberBandFadeOutTime  () const {return m_rubber_band_fade_out_time;}

    /** Returns duration of a plunger in your face. */
    float getPlungerInFaceTime      () const 
            {return m_plunger_in_face_duration[race_manager->getDifficulty()];}

    /** Returns the time a zipper is active. */
    float getZipperTime             () const {return m_zipper_time;           }

    /** Returns the time a zipper is active. */
    float getZipperFadeOutTime     () const {return m_zipper_fade_out_time;   }

    /** Returns the additional force added applied to the kart. */
    float getZipperForce            () const { return m_zipper_force;         }

    /** Returns the initial zipper speed gain. */
    float getZipperSpeedGain        () const { return m_zipper_speed_gain;    }

    /** Returns the increase of the maximum speed of the kart 
     *  if a zipper is active. */
    float getZipperMaxSpeedIncrease () const 
                                         { return m_zipper_max_speed_increase;}

    /** Returns how far behind a kart slipstreaming works. */
    float getSlipstreamLength       () const {return m_slipstream_length;     }

    /** Returns time after which slipstream has maximum effect. */
    float getSlipstreamCollectTime  () const 
                                          {return m_slipstream_collect_time;  }

    /** Returns time after which slipstream has maximum effect. */
    float getSlipstreamUseTime      () const {return m_slipstream_use_time;   }

    /** Returns additional power due to slipstreaming. */
    float getSlipstreamAddPower     () const {return m_slipstream_add_power;  }

    /** Returns the minimum slipstream speed. */
    float getSlipstreamMinSpeed     () const {return m_slipstream_min_speed;  }

    /** Returns the increase of the maximum speed of a kart 
     *  due to slipstream. */
    float getSlipstreamMaxSpeedIncrease() const 
                                    { return m_slipstream_max_speed_increase; }
    /** Returns how long the higher speed lasts after slipstream 
     *  stopped working. */
    float getSlipstreamDuration     () const { return m_slipstream_duration;  }

    /** Returns how long the slip stream speed increase will gradually 
     *  be reduced. */
    float getSlipstreamFadeOutTime  () const 
                                         { return m_slipstream_fade_out_time; }

    /** Returns the scale factor by which the shadow plane
     *  had to be set. */
    float getShadowScale            () const {return m_shadow_scale;          }
    
    /** Returns the scale factor by which the shadow plane
     *  had to be set. */
    float getShadowXOffset          () const {return m_shadow_x_offset;       }
    
    /** Returns the scale factor by which the shadow plane
     *  had to be set. */
    float getShadowYOffset          () const {return m_shadow_y_offset;       }
    
    /** Returns the maximum factor by which the steering angle
     *  can be increased. */
    float getMaxSkid                () const {return m_skid_max;              }

    /** Returns the factor by which m_skidding is multiplied when the kart is
     *  skidding to increase it to the maximum. */
    float getSkidIncrease           () const {return m_skid_increase;         }

    /** Returns the factor by which m_skidding is multiplied when the kart is
     *  not skidding to decrease the current skidding amount back to 1.0f . */
    float getSkidDecrease           () const {return m_skid_decrease;         }

    /** Returns the time (in seconds) of drifting till the maximum skidding
     *  is reached. */
    float getTimeTillMaxSkid        () const {return m_time_till_max_skid;    }

    /** Returns additional rotation of 3d model when skidding. */
    float getSkidVisual             () const {return m_skid_visual;           }

    /** Returns the time for the visual skid to reach maximum. */
    float getSkidVisualTime         () const {return m_skid_visual_time;      }

    /** Returns the angular velocity to be applied when skidding. */
    float getSkidAngularVelocity    () const { return m_skid_angular_velocity;}

    /** Returns a factor to be used to determine how much the chassis of a 
     *  kart should rotate to match the graphical view. A factor of 1 is 
     *  identical, a smaller factor will rotate the kart less (which might
     *  feel better). */
    float getPostSkidRotateFactor  () const {return m_post_skid_rotate_factor;}

    /** Returns if the kart leaves skidmarks or not. */
    bool hasSkidmarks               () const {return m_has_skidmarks;         }

    /** Returns ratio of current speed to max speed at which the gear will
     *  change (for our simualated gears = simple change of engine power). */
    const std::vector<float>&
          getGearSwitchRatio        () const {return m_gear_switch_ratio;     }

    /** Returns the power increase depending on gear. */
    const std::vector<float>&
          getGearPowerIncrease      () const {return m_gear_power_increase;   }

    /** Returns distance between kart and camera. */
    float getCameraDistance         () const {return m_camera_distance;       }

    /** Returns the angle the camera has relative to the pitch of the kart. */
    float getCameraForwardUpAngle   () const 
                                           {return m_camera_forward_up_angle; }

    /** Returns the angle the camera has relative to the pitch of the kart. */
    float getCameraBackwardUpAngle  () const 
                                          {return m_camera_backward_up_angle; }

    /** Returns AI steering variation value. */
    float getAISteeringVariation    () const {return m_ai_steering_variation; }

    /** Returns the full path where the files for this kart are stored. */
    const std::string& getKartDir   () const {return m_root;                  }

    /** Returns the square of the maximum distance at which a swatter 
     *  can hit karts. */
    float getSwatterDistance2() const { return m_swatter_distance2; }

    /** Returns how long a swatter will stay attached/ready to be used. */
    float getSwatterDuration() const { return m_swatter_duration; }

    /** Returns how long a kart remains squashed. */
    float getSquashDuration() const {return m_squash_duration; }

    /** Returns the slowdown of a kart that is squashed. */
    float getSquashSlowdown() const {return m_squash_slowdown; }

	bool hasRandomWheels() const { return m_has_rand_wheels; }

    /** Returns the bevel factor (!=0 indicates to use a bevelled box). */
    const Vec3 &getBevelFactor() const { return m_bevel_factor; }
};   // KartProperties

#endif

/* EOF */
