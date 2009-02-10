//  $Id: kart_model.hpp 2400 2008-10-30 02:02:56Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_KART_MODEL_HPP
#define HEADER_KART_MODEL_HPP

#include <string>

#define _WINSOCKAPI_
#ifdef HAVE_IRRLICHT
#include "irrlicht.h"
using namespace irr;
#else
#include <plib/ssg.h>
#endif

#include "no_copy.hpp"
#include "lisp/lisp.hpp"
#include "utils/vec3.hpp"

/** This class stores a 3D kart model. It takes especially care of attaching
 *  the wheels, which are loaded as separate objects. The wheels can turn
 *  and (for the front wheels) rotate. The implementation is dependent on the
 *  OpenGL library used. 
 */
class KartModel
{
private:
#ifdef HAVE_IRRLICHT
    /** The mesh of the model. */
    scene::IMesh *m_mesh;
#else
    /** The transform node/root of the kart model. */
    ssgTransform *m_root;
#endif

    /** Value used to indicate undefined entries. */
    static float UNDEFINED;

    /** Name of the 3d model file. */
    std::string   m_model_filename;

#ifdef HAVE_IRRLICHT
    /** The four wheel models. */
    scene::IMesh *m_wheel_model[4];
#else
    /** The four wheel models. */
    ssgEntity    *m_wheel_model[4];
#endif

    /** Filename of the wheel models. */
    std::string   m_wheel_filename[4];

    /** The position of all four wheels in the 3d model. */
    Vec3          m_wheel_graphics_position[4];

    /** The position of the wheels for the physics, which can be different 
     *  from the graphical position. */
    Vec3          m_wheel_physics_position[4]; 

    /** Radius of the graphical wheels.  */
    float         m_wheel_graphics_radius[4];

    /** Minimum suspension length. If the displayed suspension is
     *  shorter than this, the wheel would look wrong. */
    float         m_min_suspension[4];

    /** Maximum suspension length. If the displayed suspension is
     *  any longer, the wheel would look too far away from the chassis. */
    float         m_max_suspension[4];

    /** value used to divide the visual movement of wheels (because the actual movement
        of wheels in bullet is too large and looks strange). 1=no change, 2=half the amplitude */
    float         m_dampen_suspension_amplitude[4];

#ifdef HAVE_IRRLICHT
    /** The transform for the wheels, used to rotate the wheels and display
     *  the suspension in the race.      */
#else
    /** The transform for the wheels, used to rotate the wheels and display
     *  the suspension in the race.      */
    ssgTransform *m_wheel_transform[4]; 
#endif

    float m_kart_width;               /**< Width of kart.  */
    float m_kart_length;              /**< Length of kart. */
    float m_kart_height;              /**< Height of kart. */
    float m_z_offset;                 /**< Models are usually not at z=0 (due
                                       *   to the wheels), so this value moves
                                       *   the karts down appropriately. */
    void  loadWheelInfo(const lisp::Lisp* const lisp,
                        const std::string &wheel_name, int index);

public:
         KartModel();
        ~KartModel();
    void loadInfo(const lisp::Lisp* lisp);
    void loadModels(const std::string &kart_ident);
#ifdef HAVE_IRRLICHT
    void attachModel(scene::ISceneNode **node);
#else
    ssgTransform *getRoot() const { return m_root; }
#endif

    /** Returns the position of a wheel relative to the kart. 
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear 
     *           right, 3 = rear left.  */
    const Vec3& getWheelGraphicsPosition(int i) const 
                {return m_wheel_graphics_position[i];}

    /** Returns the position of a wheel relative to the kart for the physics.
     *  The physics wheels can be attached at a different place to make the 
     *  karts more stable.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear 
     *           right, 3 = rear left.  */
    const Vec3& getWheelPhysicsPosition(int i) const 
                {return m_wheel_physics_position[i];}

    /** Returns the radius of the graphical wheels.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear 
     *           right, 3 = rear left.  */
    float       getWheelGraphicsRadius(int i) const 
                {return m_wheel_graphics_radius[i]; }
    float getLength                 () const {return m_kart_length;              }
    float getWidth                  () const {return m_kart_width;               }
    float getHeight                 () const {return m_kart_height;              }
    /** Returns the amount a kart has to be moved down so that the bottom of
     *  the kart is at z=0. */
    float getZOffset                () const {return m_z_offset;                 }
    void  adjustWheels(float rotation, float steer,
                       const float suspension[4]);
    void  resetWheels();
    void  setDefaultPhysicsPosition(const Vec3 &center_shift, float wheel_radius);
};   // KartModel
#endif
