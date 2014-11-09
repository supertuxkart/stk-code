//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#ifndef HEADER_PLAYER_DIFFICULTY_HPP
#define HEADER_PLAYER_DIFFICULTY_HPP

#include "network/remote_kart_info.hpp"
#include <string>
#include <vector>


class XMLNode;

/**
 * \brief This class stores values that modify the properties of a kart.
 * This includes physical properties like speed and the effect of items.
 * The values stored in this class get multiplied with the current
 * properties of the kart. If all values here are set to 1, nothing changes.
 *
 * \ingroup karts
 */
class PlayerDifficulty
{
private:
    /** Actual difficulty */
    PerPlayerDifficulty m_difficulty;

    // -----------------
    /** Weight of kart.  */
    float m_mass;

    /** Maximum force from engine for each difficulty. */
    float m_engine_power;

    /** Braking factor * engine_power braking force. */
    float m_brake_factor;

    /** Brake_time * m_brake_time_increase will increase the break time
     * over time. */
    float m_brake_time_increase;

    /** Time a kart is moved upwards after when it is rescued. */
    float m_rescue_time;

    /** Time an animated explosion is shown. Longer = more delay for kart. */
    float m_explosion_time;

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
    float       m_plunger_in_face_duration;
    /** Nitro consumption. */
    float       m_nitro_consumption;
    /* How much the speed of a kart might exceed its maximum speed (in m/s). */
    float       m_nitro_max_speed_increase;
    /** Additional engine force to affect the kart. */
    float       m_nitro_engine_force;
    /**  How long the increased nitro max speed will be valid after
     *  the kart stops using nitro (and the fade-out-time starts). */
    float       m_nitro_duration;
    /** Duration during which the increased maximum speed
     *  due to nitro fades out. */
    float       m_nitro_fade_out_time;
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
    /** How long the swatter lasts. */
    float       m_swatter_duration;
    /** How long a kart will remain squashed. */
    float       m_squash_duration;
    /** The slowdown to apply while a kart is squashed. The new maxspeed
     *  is max_speed*m_squash_slowdown. */
    float       m_squash_slowdown;

    /** The maximum speed at each difficulty. */
    float       m_max_speed;

    float m_max_speed_reverse_ratio;

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

    /** If the kart starts within the specified time at index I after 'go',
     *  it receives the speed boost from m_startup_boost[I]. */
    std::vector<float> m_startup_times;

    /** The startup boost is the kart starts fast enough. */
    std::vector<float> m_startup_boost;


    void  load              (const std::string &filename,
                             const std::string &node);


public:
    PlayerDifficulty  (const std::string &filename="");
    ~PlayerDifficulty  ();
    void  getAllData        (const XMLNode * root);
    std::string getIdent() const;
    float getStartupBoost   () const;

    // ------------------------------------------------------------------------
    /** Returns the maximum engine power depending on difficulty. */
    float getMaxPower               () const {return m_engine_power;          }

    // ------------------------------------------------------------------------
    /** Get braking information. */
    float getBrakeFactor            () const {return m_brake_factor;          }

    // ------------------------------------------------------------------------
    /** Returns the additional brake factor which depends on time. */
    float getBrakeTimeIncrease() const { return m_brake_time_increase; }

    // ------------------------------------------------------------------------
    /** Get maximum reverse speed ratio. */
    float getMaxSpeedReverseRatio   () const
                                          {return m_max_speed_reverse_ratio;  }

    // ------------------------------------------------------------------------
    /** Returns the maximum speed dependent on the difficult level. */
    float getMaxSpeed               () const { return m_max_speed;            }

    // ------------------------------------------------------------------------
    /** Returns the nitro consumption. */
    float getNitroConsumption       () const {return m_nitro_consumption;     }

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
    /** Returns the time a kart is rised during a rescue. */
    float getRescueTime             () const {return m_rescue_time;           }

    // ------------------------------------------------------------------------
    /** Returns the time an explosion animation is shown. */
    float getExplosionTime          () const {return m_explosion_time;        }

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
    float getPlungerInFaceTime      () const {return m_plunger_in_face_duration;}

    // ------------------------------------------------------------------------
    /** Returns the time a zipper is active. */
    float getZipperTime             () const {return m_zipper_time;           }

    // ------------------------------------------------------------------------
    /** Returns the time a zipper is active. */
    float getZipperFadeOutTime      () const {return m_zipper_fade_out_time;  }

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
    /** Returns how long a swatter will stay attached/ready to be used. */
    float getSwatterDuration() const { return m_swatter_duration; }

    // ------------------------------------------------------------------------
    /** Returns how long a kart remains squashed. */
    float getSquashDuration() const {return m_squash_duration; }

    // ------------------------------------------------------------------------
    /** Returns the slowdown of a kart that is squashed. */
    float getSquashSlowdown() const {return m_squash_slowdown; }
};   // KartProperties

#endif

