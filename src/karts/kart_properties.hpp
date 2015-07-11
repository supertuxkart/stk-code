//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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
#include "utils/interpolation_array.hpp"
#include "utils/vec3.hpp"

class AbstractCharacteristic;
class AIProperties;
class Material;
class SkiddingProperties;
class XmlCharacteristic;
class XMLNode;

/**
 *  \brief This class stores the properties of a kart.
 *  This includes size, name, identifier, physical properties etc.
 *  It is atm also the base class for STKConfig, which stores the default values
 *  for all physics constants.
 *  Note that KartProperties is copied (when setting the default values from
 *  stk_config.
 *
 * \ingroup karts
 */
class KartProperties
{
private:
    /** Base directory for this kart. */
    std::string              m_root;

    /** The skididing properties for this kart, as a separate object in order
     *  to reduce dependencies (and therefore compile time) when changing
     *  any skidding property. */
    SkiddingProperties *m_skidding_properties;

    /** AI Properties for this kart, as a separate object in order to
     *  reduce dependencies (and therefore compile time) when changing
     *  any AI property. There is one separate object for each
     *  difficulty. */
    AIProperties *m_ai_properties[RaceManager::DIFFICULTY_COUNT];

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

    /** Dummy value to detect unset properties. */
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
    float m_shadow_z_offset;          /**< Z offset of the shadow plane
                                       *   for this kart.*/
    video::ITexture *m_shadow_texture;/**< The texture with the shadow. */
    video::SColor m_color;            /**< Color the represents the kart in the
                                       *   status bar and on the track-view. */
    int  m_shape;                     /**< Number of vertices in polygon when
                                       *   drawing the dot on the mini map. */

    /** The physical, item, etc. characteristics of this kart that are loaded
     *  from the xml file.
     */
    XmlCharacteristic *m_characteristic;

    // Physic properties
    // -----------------
    /** Weight of kart.  */
    float m_mass;

    /** Maximum force from engine for each difficulty. */
    std::vector<float> m_engine_power;

    /** Braking factor * engine_power braking force. */
    float m_brake_factor;

    /** Brake_time * m_brake_time_increase will increase the break time
     * over time. */
    float m_brake_time_increase;

    /** Time for player karts to reach full steer angle. */
    InterpolationArray m_time_full_steer;

    /** Time for steering to go back to zero from full steer. */
    float m_time_reset_steer;

    /** A torque impulse applied to keep the kart parallel to the ground. */
    float m_smooth_flying_impulse;;

    /** The turn angle depending on speed. */
    InterpolationArray m_turn_angle_at_speed;

    /** If != 0 a bevelled box shape is used by using a point cloud as a
     *  collision shape. */
    Vec3  m_bevel_factor;

    /** The position of the physical wheel is a weighted average of the
     *  two ends of the beveled shape. This determines the weight: 0 =
     *  a the widest end, 1 = at the narrowest front end. If the value is
     *  < 0, the old physics settings are used which places the raycast
     *  wheels outside of the chassis - but result in a more stable
     *  physics behaviour (which is therefore atm still the default).
     */
    float m_physical_wheel_position;

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

    /** Minimum time during which nitro is consumed when pressing
     *  the nitro key (to prevent using in very small bursts)
     */
    float m_nitro_min_consumption;

    /** Type of the kart (for the properties) */
    std::string m_kart_type;

    /** Filename of the wheel models. */
    std::string m_wheel_filename[4];
    /**  Radius of the graphical wheels.  */
    float       m_wheel_graphics_radius[4];
    /** An additional Y offset added to the y position of the graphical
     *  chassis. Useful for karts that don't have enough space for suspension
     *  compression. */
    float       m_graphical_y_offset;
    /** A hard flag that moves the graphical chassis higher if it's insde
      * the track. Might cause stuttering. */
    bool        m_prevent_chassis_in_terrain;
    /** If the kart is supposed to have random wheel rotation at start. */
    bool        m_has_rand_wheels;
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
    std::vector<float>  m_plunger_in_face_duration;
    /** Wheel base of the kart. */
    float       m_wheel_base;
    /** Nitro consumption. */
    float       m_nitro_consumption;
    /** Nitro amount for small bottle. */
    float       m_nitro_small_container;
    /** Nitro amount for big bittle. */
    float       m_nitro_big_container;
    /** How much the speed of a kart might exceed its maximum speed (in m/s). */
    float       m_nitro_max_speed_increase;
    /** Additional engine force to affect the kart. */
    float       m_nitro_engine_force;
    /**  How long the increased nitro max speed will be valid after
     *  the kart stops using nitro (and the fade-out-time starts). */
    float       m_nitro_duration;
    /** Duration during which the increased maximum speed
     *  due to nitro fades out. */
    float       m_nitro_fade_out_time;
    /** Maximum nitro a kart can collect. */
    float       m_nitro_max;
    /** Bubble gum diration. */
    float       m_bubblegum_time;
    /** Torque to add when a bubble gum was hit in order to make the kart go
     *  sideways a bit. */
    float       m_bubblegum_torque;
    /** Fraction of top speed that can be reached maximum after hitting a
     *  bubble gum. */
    float       m_bubblegum_speed_fraction;
    /** How long to fade in the slowdown for a bubble gum. */
    float       m_bubblegum_fade_in_time;
    /** Square of the maximum distance a swatter can operate. */
    float       m_swatter_distance2;
    /** How long the swatter lasts. */
    float       m_swatter_duration;
    /** How long a kart will remain squashed. */
    float       m_squash_duration;
    /** The slowdown to apply while a kart is squashed. The new maxspeed
     *  is max_speed*m_squash_slowdown. */
    float       m_squash_slowdown;

    /** The maximum roll a kart graphics should show when driving in a fast
     *  curve. This is read in as degrees, but stored in radians. */
     float      m_max_lean;

     /** The speed with which the roll (when leaning in a curve) changes
      *  (in radians/second). */
     float      m_lean_speed;

     /** How long a jump must be in order to trigger the jump animation. */
     float      m_jump_animation_time;

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

    /** Parameters for the speed-weighted objects */
    SpeedWeightedObject::Properties   m_speed_weighted_object_properties;

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

    /** The maximum speed at each difficulty. */
    std::vector<float> m_max_speed;

    float m_max_speed_reverse_ratio;

    /** Shift of center of gravity. */
    Vec3  m_gravity_center_shift;

    /** The suspension reaction is dampened to reach an exponential behaviour.
     *  See http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=9&t=7369\
     *  &p=25236&hilit=vehicle#p25236  for details. */
    bool  m_exp_spring_response;

    float m_suspension_rest;
    float m_suspension_travel_cm;

public:
    /** STK can add an impulse to push karts away from the track in case
     *  of a kart-track collision. This can be done in two ways: either
     *  apply the impulse in the direction of the normal, or towards the
     *  driveline. The later works nice as long as the kart is driving
     *  on the main track, but can work very bad if the kart is drivling
     *  off-track (and a wrong driveline is selected). */
    enum TerrainImpulseType {IMPULSE_NONE, IMPULSE_NORMAL,
                             IMPULSE_TO_DRIVELINE};
private:
    TerrainImpulseType m_terrain_impulse_type;

    /** An additional impulse to push a kart away if it hits terrain */
    float m_collision_terrain_impulse;

    /** An additiojnal artificial impulse that pushes two karts in a
     *  side-side collision away from each other. */
    float m_collision_impulse;

    /** How long the collision impulse should be applied. */
    float m_collision_impulse_time;

    /** The restitution factor to be used in collsions for this kart. */
    float m_restitution;

    /** How far behind a kart slipstreaming is effective. */
    float m_slipstream_length;
    /** How wide the slipstream area is at the end. */
    float m_slipstream_width;
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

    /** Distance of normal camera from kart. */
    float m_camera_distance;

    /** Up angle of the camera in relation to the pitch of the kart when
     *  driving forwards. */
    float m_camera_forward_up_angle;

    /** Up angle of the camera in relation to the pitch of the kart when
     *  driving backwards. */
    float m_camera_backward_up_angle;

    /** The following two vectors define at what ratio of the maximum speed what
     * gear is selected. E.g. 0.25 means: if speed <=0.25*maxSpeed --> gear 1,
     *                        0.5  means: if speed <=0.5 *maxSpeed --> gear 2 */
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


public:
    /** Returns the string representation of a per-player difficulty. */
    static std::string      getPerPlayerDifficultyAsString(PerPlayerDifficulty d);

          KartProperties    (const std::string &filename="");
         ~KartProperties    ();
    void  copyFrom          (const KartProperties *source);
    void  getAllData        (const XMLNode * root);
    void  checkAllSet       (const std::string &filename);
    float getStartupBoost   () const;
    bool  isInGroup         (const std::string &group) const;
    bool operator<(const KartProperties &other) const;

    // ------------------------------------------------------------------------
    /** Returns the characteristics for this kart. */
    const AbstractCharacteristic* getCharacteristic() const;

    // ------------------------------------------------------------------------
    /** Returns the (maximum) speed for a given turn radius.
     *  \param radius The radius for which the speed needs to be computed. */
    float getSpeedForTurnRadius(float radius) const {
        float angle = sin(m_wheel_base / radius);
        return m_turn_angle_at_speed.getReverse(angle);
    }   // getSpeedForTurnRadius
    // ------------------------------------------------------------------------
    /** Returns the maximum steering angle (depending on speed). */
    float getMaxSteerAngle(float speed) const {
        return m_turn_angle_at_speed.get(speed);
    }   // getMaxSteerAngle

    // ------------------------------------------------------------------------
    /** Returns the material for the kart icons. */
    Material*     getIconMaterial    () const {return m_icon_material;        }

    // ------------------------------------------------------------------------
    /** Returns the texture to use in the minimap, or NULL if not defined. */
    video::ITexture *getMinimapIcon  () const {return m_minimap_icon;         }

    // ------------------------------------------------------------------------
    /** Returns a pointer to the KartModel object. */
    KartModel*    getKartModelCopy   () const
                                            {return m_kart_model->makeCopy(); }

    // ------------------------------------------------------------------------
    /** Returns a pointer to the main KartModel object. This copy
     *  should not be modified, not attachModel be called on it. */
    const KartModel& getMasterKartModel() const {return *m_kart_model;        }

    // ------------------------------------------------------------------------
    /** Sets the name of a mesh to be used for this kart.
     *  \param hat_name Name of the mesh.
     */
    void setHatMeshName(const std::string &hat_name)
    {
        m_kart_model->setHatMeshName(hat_name);
    }   // setHatMeshName
    // ------------------------------------------------------------------------
    /** Returns the name of this kart.
        \note Pass it through fridibi as needed, this is the LTR name
      */
    core::stringw getName() const
    {
        return core::stringw(translations->w_gettext(m_name.c_str()));
    }

    // ------------------------------------------------------------------------
    const std::string getNonTranslatedName() const {return m_name;}

    // ------------------------------------------------------------------------
    /** Returns the internal identifier of this kart. */
    const std::string& getIdent      () const {return m_ident;                }

    // ------------------------------------------------------------------------
    /** Returns the type of this kart. */
    const std::string& getKartType   () const { return m_kart_type;           }

    // ------------------------------------------------------------------------
    /** Returns the shadow texture to use. */
    video::ITexture *getShadowTexture() const {return m_shadow_texture;       }

    // ------------------------------------------------------------------------
    /** Returns the absolute path of the icon file of this kart. */
    const std::string& getAbsoluteIconFile() const      { return m_icon_file; }

    // ------------------------------------------------------------------------
    /** Returns custom sound effects for this kart. */
    const int          getCustomSfxId (SFXManager::CustomSFX type)
                                       const  {return m_custom_sfx_id[type];  }

    // ------------------------------------------------------------------------
    /** Returns the version of the .kart file. */
    int   getVersion                () const {return m_version;               }

    // ------------------------------------------------------------------------
    /** Returns the dot color to use for this kart in the race gui. */
    const video::SColor &getColor   () const {return m_color;                 }

    // ------------------------------------------------------------------------
    /** Returns the number of edges for the polygon used to draw the dot of
     *  this kart on the mini map of the race gui. */
    int   getShape                  () const {return m_shape;                 }

    // ------------------------------------------------------------------------
    /** Returns the list of groups this kart belongs to. */
    const std::vector<std::string>&
                  getGroups         () const {return m_groups;                }
    // ------------------------------------------------------------------------
    /** Returns the mass of this kart. */
    float getMass                   () const {return m_mass;                  }
    // ------------------------------------------------------------------------
    /** Returns the maximum engine power depending on difficulty. */
    float getMaxPower               () const
                        {return m_engine_power[race_manager->getDifficulty()];}

    // ------------------------------------------------------------------------
    /** Returns the time the kart needs to fully steer in one direction from
     *  steering straight depending on the current steering value.
     *  \param steer Current steering value, must be >=0. */
    float getTimeFullSteer(float steer) const
    {
        assert(steer>=0);
        return m_time_full_steer.get(steer);
    }   // getTimeFullSteer

    // ------------------------------------------------------------------------
    /** Returns the time the kart needs to go back to steering straight from
     *  full steer. */
    float getTimeResetSteer         () const { return m_time_reset_steer;     }
    // ------------------------------------------------------------------------
    /** Get braking information. */
    float getBrakeFactor            () const {return m_brake_factor;          }

    // ------------------------------------------------------------------------
    /** Returns the additional brake factor which depends on time. */
    float getBrakeTimeIncrease() const { return m_brake_time_increase; }

    // ------------------------------------------------------------------------
    /** Returns the torque scaling factor used to keep the karts parallel to
     *  the ground when flying. */
    float getSmoothFlyingImpulse() const
    {
        return m_smooth_flying_impulse;
    }   // getSmoothFlyingImpulse

    // ------------------------------------------------------------------------
    /** Get maximum reverse speed ratio. */
    float getMaxSpeedReverseRatio() const {return m_max_speed_reverse_ratio;  }

    // ------------------------------------------------------------------------
    /** Returns the engine type (used to change sfx depending on kart size). */
    const std::string& getEngineSfxType    () const {return m_engine_sfx_type;}

    // Bullet physics get functions
    //-----------------------------
    /** Returns the suspension stiffness. */
    float getSuspensionStiffness    () const {return m_suspension_stiffness;  }

    // ------------------------------------------------------------------------
    /** Returns damping relaxation. */
    float getWheelDampingRelaxation () const
                                          {return m_wheel_damping_relaxation; }

    // ------------------------------------------------------------------------
    /** Returns the wheel damping compression. */
    float getWheelDampingCompression() const
                                          {return m_wheel_damping_compression;}

    // ------------------------------------------------------------------------
    /** Returns maximum suspension force. */
    float getMaxSuspensionForce() const {return m_max_suspension_force; }

    // ------------------------------------------------------------------------
    /** Returns friction slip. */
    float getFrictionSlip           () const {return m_friction_slip;         }

    // ------------------------------------------------------------------------
    /** Returns roll influence. */
    float getRollInfluence          () const {return m_roll_influence;        }

    // ------------------------------------------------------------------------
    /** Returns wheel radius. */
    float getWheelRadius            () const {return m_wheel_radius;          }

    // ------------------------------------------------------------------------
    /** Return the additional Y offset added to the y position of the graphical
     *  chassis. Useful for karts that don't have enough space for suspension
     *  compression. */
    float getGraphicalYOffset() const {return m_graphical_y_offset; }
    // ------------------------------------------------------------------------
    /** A hard flag that moves the graphical chassis higher if it's insde
      * the track. Might cause stuttering. */
    bool getPreventChassisInTerrain() const
    {
        return m_prevent_chassis_in_terrain;
    }   // getPreventChassisInTerrain
    // ------------------------------------------------------------------------
    /** Returns parameters for the speed-weighted objects */
    const SpeedWeightedObject::Properties& getSpeedWeightedObjectProperties() const
    {
        return m_speed_weighted_object_properties;
    }

    // ------------------------------------------------------------------------
    /** Returns the wheel base (distance front to rear axis). */
    float getWheelBase              () const {return m_wheel_base;            }

    // ------------------------------------------------------------------------
    /** Returns linear damping of chassis. */
    float getChassisLinearDamping   () const {return m_chassis_linear_damping;}

    // ------------------------------------------------------------------------
    /** Returns angular damping of chassis. */
    float getChassisAngularDamping  () const
                                           {return m_chassis_angular_damping; }

    // ------------------------------------------------------------------------
    /** Artifical downward impulse every frame. */
    float getDownwardImpulseFactor() const { return m_downward_impulse_factor;}

    // ------------------------------------------------------------------------
    /** Returns artificial acceleration to keep wheels on track. */
    float getTrackConnectionAccel   () const {return m_track_connection_accel;}

    // ------------------------------------------------------------------------
    /** Returns the maximum speed dependent on the difficult level. */
    float getMaxSpeed               () const
    {
        return m_max_speed[race_manager->getDifficulty()];
    }

    // ------------------------------------------------------------------------
    /** Return the absolute maximum speed, independent on the difficulty. */
    float getAbsMaxSpeed            () const
    {
        return m_max_speed[m_max_speed.size()-1];
    }

    // ------------------------------------------------------------------------
    /** Returns the nitro consumption. */
    float getNitroConsumption       () const {return m_nitro_consumption;     }

    // ------------------------------------------------------------------------
    /** Returns the amount of nitro for a small container. */
    float getNitroSmallContainer    () const {return m_nitro_small_container; }

    // ------------------------------------------------------------------------
    /** Returns the amount of nitro for a big container. */
    float getNitroBigContainer      () const {return m_nitro_big_container;   }

    // ------------------------------------------------------------------------
    /** Returns the increase of maximum speed due to nitro. */
    float getNitroMaxSpeedIncrease  () const
                                          {return m_nitro_max_speed_increase; }

    // ------------------------------------------------------------------------
    float getNitroEngineForce       () const {return m_nitro_engine_force;    }
    // ------------------------------------------------------------------------
    /** Returns how long the increased nitro max speed will be valid after
     *  the kart stops using nitro (and the fade-out-time starts). */
    float getNitroDuration          () const {return m_nitro_duration;        }

    // ------------------------------------------------------------------------
    /** Returns the duration during which the increased maximum speed
     *  due to nitro fades out. */
    float getNitroFadeOutTime       () const {return m_nitro_fade_out_time;   }

    // ------------------------------------------------------------------------
    /** Returns the maximum amount of nitro a kart can store. */
    float getNitroMax               () const {return m_nitro_max;             }
    // ------------------------------------------------------------------------
    /** Returns how long a bubble gum is active. */
    float getBubblegumTime() const { return m_bubblegum_time; }
    // ------------------------------------------------------------------------
    /** Returns the torque to add when a bubble gum was hit . */
    float getBubblegumTorque() const { return m_bubblegum_torque; }
    // ------------------------------------------------------------------------
    /** Returns the fraction of top speed that can be reached maximum after
     *  hitting a bubble gum. */
    float getBubblegumSpeedFraction() const {return m_bubblegum_speed_fraction;}
    // ------------------------------------------------------------------------
    /** Returns how long to fade in the slowdown for a bubble gum. */
    float getBubblegumFadeInTime() const { return m_bubblegum_fade_in_time; }
    // ------------------------------------------------------------------------
    /** Returns a shift of the center of mass (lowering the center of mass
     *  makes the karts more stable. */
    const Vec3&getGravityCenterShift() const {return m_gravity_center_shift;  }

    // ------------------------------------------------------------------------
    /** Retusn suspension rest length. */
    float getSuspensionRest         () const {return m_suspension_rest;       }

    // ------------------------------------------------------------------------
    /** Returns the amount the suspension can extend. */
    float getSuspensionTravelCM     () const {return m_suspension_travel_cm;  }

    // ------------------------------------------------------------------------
    /** Returns if the spring should be exponentially dampened. */
    bool getExpSpringResponse() const {return m_exp_spring_response; }

    // ------------------------------------------------------------------------
    /** Returns an artificial impulse to push karts away from the terrain
     *  it hits. */
    float getCollisionTerrainImpulse() const
                                          {return m_collision_terrain_impulse;}

    // ------------------------------------------------------------------------
    /** Returns what kind of impulse STK should use in case of a kart-track
     *  collision. */
    TerrainImpulseType getTerrainImpulseType() const
                                             { return m_terrain_impulse_type; }
    // ------------------------------------------------------------------------
    /** Returns the (artificial) collision impulse this kart will apply
     *  to another kart in case of a non-frontal collision. */
    float getCollisionImpulse       () const {return m_collision_impulse;}

    // ------------------------------------------------------------------------
    /** Returns how long the collision impulse should be applied. */
    float getCollisionImpulseTime() const { return m_collision_impulse_time;}

    // ------------------------------------------------------------------------
    /** Returns the restitution factor for this kart. */
    float getRestitution            () const { return m_restitution; }

    // ------------------------------------------------------------------------
    /** Returns the vertical offset when rescuing karts to avoid karts being
     *  rescued in (or under) the track. */
    float getVertRescueOffset       () const {return m_rescue_vert_offset;    }

    // ------------------------------------------------------------------------
    /** Returns the time a kart is rised during a rescue. */
    float getRescueTime             () const {return m_rescue_time;           }

    // ------------------------------------------------------------------------
    /** Returns the height a kart is moved to during a rescue. */
    float getRescueHeight           () const {return m_rescue_height;         }

    // ------------------------------------------------------------------------
    /** Returns the time an explosion animation is shown. */
    float getExplosionTime          () const {return m_explosion_time;        }

    // ------------------------------------------------------------------------
    /** Returns the height of the explosion animation. */
    float getExplosionRadius        () const {return m_explosion_radius;      }

    // ------------------------------------------------------------------------
    /** Returns how long a kart is invulnerable after being hit by an
        explosion. */
    float getExplosionInvulnerabilityTime() const
                                   { return m_explosion_invulnerability_time; }

    // ------------------------------------------------------------------------
    /** Returns the maximum length of a rubber band before it breaks. */
    float getRubberBandMaxLength    () const {return m_rubber_band_max_length;}

    // ------------------------------------------------------------------------
    /** Returns force a rubber band has when attached to a kart. */
    float getRubberBandForce        () const {return m_rubber_band_force;     }

    // ------------------------------------------------------------------------
    /** Returns the duration a rubber band is active for. */
    float getRubberBandDuration     () const {return m_rubber_band_duration;  }

    // ------------------------------------------------------------------------
    /** Returns the increase of maximum speed while a rubber band is
     *  pulling. */
    float getRubberBandSpeedIncrease() const
    {
        return m_rubber_band_speed_increase;
    }

    // ------------------------------------------------------------------------
    /** Return the fade out time once a rubber band is removed. */
    float getRubberBandFadeOutTime() const
    {
        return m_rubber_band_fade_out_time;
    }

    // ------------------------------------------------------------------------
    /** Returns duration of a plunger in your face. */
    float getPlungerInFaceTime      () const
            {return m_plunger_in_face_duration[race_manager->getDifficulty()];}

    // ------------------------------------------------------------------------
    /** Returns the time a zipper is active. */
    float getZipperTime             () const {return m_zipper_time;           }

    // ------------------------------------------------------------------------
    /** Returns the time a zipper is active. */
    float getZipperFadeOutTime     () const {return m_zipper_fade_out_time;   }

    // ------------------------------------------------------------------------
    /** Returns the additional force added applied to the kart. */
    float getZipperForce            () const { return m_zipper_force;         }

    // ------------------------------------------------------------------------
    /** Returns the initial zipper speed gain. */
    float getZipperSpeedGain        () const { return m_zipper_speed_gain;    }

    // ------------------------------------------------------------------------
    /** Returns the increase of the maximum speed of the kart
     *  if a zipper is active. */
    float getZipperMaxSpeedIncrease () const
                                         { return m_zipper_max_speed_increase;}

    // ------------------------------------------------------------------------
    /** Returns how far behind a kart slipstreaming works. */
    float getSlipstreamLength       () const {return m_slipstream_length;     }

    // ------------------------------------------------------------------------
    /** Returns how wide the slipstream area is at the end. */
    float getSlipstreamWidth        () const {return m_slipstream_width;      }

    // ------------------------------------------------------------------------
    /** Returns time after which slipstream has maximum effect. */
    float getSlipstreamCollectTime  () const
                                          {return m_slipstream_collect_time;  }

    // ------------------------------------------------------------------------
    /** Returns time after which slipstream has maximum effect. */
    float getSlipstreamUseTime      () const {return m_slipstream_use_time;   }

    // ------------------------------------------------------------------------
    /** Returns additional power due to slipstreaming. */
    float getSlipstreamAddPower     () const {return m_slipstream_add_power;  }

    // ------------------------------------------------------------------------
    /** Returns the minimum slipstream speed. */
    float getSlipstreamMinSpeed     () const {return m_slipstream_min_speed;  }

    // ------------------------------------------------------------------------
    /** Returns the increase of the maximum speed of a kart
     *  due to slipstream. */
    float getSlipstreamMaxSpeedIncrease() const
                                    { return m_slipstream_max_speed_increase; }
    // ------------------------------------------------------------------------
    /** Returns how long the higher speed lasts after slipstream
     *  stopped working. */
    float getSlipstreamDuration     () const { return m_slipstream_duration;  }

    // ------------------------------------------------------------------------
    /** Returns how long the slip stream speed increase will gradually
     *  be reduced. */
    float getSlipstreamFadeOutTime  () const
                                         { return m_slipstream_fade_out_time; }

    // ------------------------------------------------------------------------
    /** Returns the scale factor by which the shadow plane
     *  had to be set. */
    float getShadowScale            () const {return m_shadow_scale;          }

    // ------------------------------------------------------------------------
    /** Returns the scale factor by which the shadow plane
     *  had to be set. */
    float getShadowXOffset          () const {return m_shadow_x_offset;       }

    // ------------------------------------------------------------------------
    /** Returns the scale factor by which the shadow plane
     *  had to be set. */
    float getShadowZOffset          () const {return m_shadow_z_offset;       }

    // ------------------------------------------------------------------------
    /** Returns a pointer to the skidding properties. */
    const SkiddingProperties *getSkiddingProperties() const
                                              { return m_skidding_properties; }

    // ------------------------------------------------------------------------
    /** Returns a pointer to the AI properties. */
    const AIProperties *getAIPropertiesForDifficulty() const
    {
        return m_ai_properties[race_manager->getDifficulty()];
    }   // getAIProperties

    // ------------------------------------------------------------------------
    /** Returns ratio of current speed to max speed at which the gear will
     *  change (for our simualated gears = simple change of engine power). */
    const std::vector<float>&
          getGearSwitchRatio        () const {return m_gear_switch_ratio;     }

    // ------------------------------------------------------------------------
    /** Returns the power increase depending on gear. */
    const std::vector<float>&
          getGearPowerIncrease      () const {return m_gear_power_increase;   }

    // ------------------------------------------------------------------------
    /** Returns the average power of the kart (in all gears). */
    const float getAvgPower         () const;

    // ------------------------------------------------------------------------
    /** Returns distance between kart and camera. */
    float getCameraDistance         () const {return m_camera_distance;       }

    // ------------------------------------------------------------------------
    /** Returns the angle the camera has relative to the pitch of the kart. */
    float getCameraForwardUpAngle   () const
                                           {return m_camera_forward_up_angle; }

    // ------------------------------------------------------------------------
    /** Returns the angle the camera has relative to the pitch of the kart. */
    float getCameraBackwardUpAngle  () const
                                          {return m_camera_backward_up_angle; }

    // ------------------------------------------------------------------------
    /** Returns the full path where the files for this kart are stored. */
    const std::string& getKartDir   () const {return m_root;                  }

    // ------------------------------------------------------------------------
    /** Returns the square of the maximum distance at which a swatter
     *  can hit karts. */
    float getSwatterDistance2() const { return m_swatter_distance2; }

    // ------------------------------------------------------------------------
    /** Returns how long a swatter will stay attached/ready to be used. */
    float getSwatterDuration() const { return m_swatter_duration; }

    // ------------------------------------------------------------------------
    /** Returns how long a kart remains squashed. */
    float getSquashDuration() const {return m_squash_duration; }

    // ------------------------------------------------------------------------
    /** Returns the slowdown of a kart that is squashed. */
    float getSquashSlowdown() const {return m_squash_slowdown; }

    // ------------------------------------------------------------------------
    /** The maximum leaning a kart should show (In radians). */
    float getMaxLean() const { return m_max_lean; }

    // ------------------------------------------------------------------------
    /** The speed with which a kart should lean (in radians/s). */
    float getLeanSpeed() const { return m_lean_speed; }
    // ------------------------------------------------------------------------
    /** Return show long a jump must last in order to play the jump
     *  animation. */
    float getJumpAnimationTime() const { return m_jump_animation_time; }
    // ------------------------------------------------------------------------
    /** Returns true if wheels should have random rotation at start. */
    bool hasRandomWheels() const { return m_has_rand_wheels; }
    // ------------------------------------------------------------------------
    /** Returns minimum time during which nitro is consumed when pressing nitro
     *  key, to prevent using nitro in very short bursts
     */
    float getNitroMinConsumptionTime() const { return m_nitro_min_consumption; }
    // ------------------------------------------------------------------------
    /** Returns the bevel factor (!=0 indicates to use a bevelled box). */
    const Vec3 &getBevelFactor() const { return m_bevel_factor; }
    // ------------------------------------------------------------------------
    /** Returns position of the physical wheel is a weighted average of the
     *  two ends of the beveled shape. This determines the weight: 0 =
     *  a the widest end, 1 = at the narrowest, front end. If the value is <0,
     *  the old physics position is picked, which placed the raycast wheels
     *  outside of the chassis, but gives more stable physics. */
    const float getPhysicalWheelPosition() const
    {
        return m_physical_wheel_position;
    }   // getPhysicalWheelPosition
};   // KartProperties

#endif

/* EOF */
