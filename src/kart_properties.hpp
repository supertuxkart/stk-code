//  $Id: KartProperties.h,v 1.2 2005/07/14 15:43:44 joh Exp $
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

#ifndef HEADER_KARTPROPERTIES_H
#define HEADER_KARTPROPERTIES_H

#include <string>
#include "lisp/lisp.hpp"
#include "no_copy.hpp"

class Material;
class ssgEntity;

class KartProperties : public NoCopy {
 private:

  Material* icon_material;
  ssgEntity* model;

 protected:  
  /* Display and gui */
  /* --------------- */
  std::string name;         // The human readable Name of the karts driver
  std::string ident;        // The computer readable-name of the karts driver
  std::string model_file;   // Filename of 3d model that is used for kart
  std::string icon_file;    // Filename of icon that represents the kart in
			                      // the statusbar and the character select screen  
  std::string shadow_file;  // Filename of the image file that contains the 
			                      // shadow for this kart
  float color[3];           // Color the represents the kart in the status
			                      // bar and on the track-view
  
  /* Physic properties */
  /* ----------------- */
  float mass;               // weight of kart
  float air_friction;       // air friction
  float roll_resistance;    // rolling resistance etc
  float wheel_base;         // distance between front and read wheels
  float heightCOG;          // height of center of gravity
  float engine_power;       // maximum force from engine
  float brake_factor;       // braking factor * engine_power = braking force
  float tire_grip;          // grip of tires in longitudinal direction
  float max_steer_angle;    // maximum steering angle

  /** old properties */
  float corn_f;
  float corn_r;
  float inertia;

 public:
                KartProperties   ();
  virtual      ~KartProperties   ();
  
  virtual void  init_defaults    ();
  virtual void  getAllData       (const lisp::Lisp* lisp);
  virtual void  load             (const char* filename, char* node="tuxkart-kart");

  Material*     getIconMaterial()   const { return icon_material;     }
  ssgEntity*    getModel()          const {return model;              }
  const char*   getName()           const {return name.c_str();       }
  const char*   getIdent()          const {return ident.c_str();      }
  const char*   getShadowFile()     const {return shadow_file.c_str();}
  const char*   getIconFile()       const {return icon_file.c_str();  }
  const sgVec3* getColor()          const {return &color;             }
  float         getMass()           const {return mass;               }
  float         getAirFriction()    const {return air_friction;       }
  float         getRollResistance() const {return roll_resistance;    }
  float         getMaxPower()       const { return engine_power;      }
  float         getBrakeFactor()    const {return brake_factor;       }
  float         getWheelBase()      const {return wheel_base;         }
  float         getHeightCOG()      const {return heightCOG;          }
  float         getTireGrip()       const {return tire_grip;          }
  float         getMaxSteerAngle()  const {return max_steer_angle;    }
  float         getCornerStiffF()   const {return corn_f;             }
  float         getCornerStiffR()   const {return corn_r;             }
  float         getInertia()        const {return inertia;            }

};

#endif

/* EOF */
