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

#include "karts/kart_model.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "utils/constants.hpp"

float KartModel::UNDEFINED = -99.9f;

/** The constructor reads the model file name and wheel specification from the
 *  kart config file.
 */
KartModel::KartModel()
{
    for(unsigned int i=0; i<4; i++)
    {
        m_wheel_graphics_position[i] = Vec3(UNDEFINED);
        m_wheel_physics_position[i]  = Vec3(UNDEFINED);
        m_wheel_graphics_radius[i]   = 0.0f;   // for kart without separate wheels
        m_wheel_model[i]             = NULL;
        // default value for kart suspensions. move to config file later if we find each kart needs custom values
        m_min_suspension[i] = -1.3f;
        m_max_suspension[i] = 1.3f;
        m_dampen_suspension_amplitude[i] = 2.5f;
    }
    m_wheel_filename[0] = "wheel-front-right.3ds";
    m_wheel_filename[1] = "wheel-front-left.3ds";
    m_wheel_filename[2] = "wheel-rear-right.3ds";
    m_wheel_filename[3] = "wheel-rear-left.3ds";
    m_mesh              = NULL;
    m_af_left           = -1;
    m_af_straight       = -1;
    m_af_right          = -1;
    m_animation_speed   = 15;
}   // KartModel

// ----------------------------------------------------------------------------
/** This function loads the information about the kart from a lisp file. It 
 *  does not actually load the models (see load()).
 *  \param lisp  Lisp object of configuration file.
 */
void KartModel::loadInfo(const lisp::Lisp* lisp)
{
    lisp->get("model-file",         m_model_filename );
    lisp->get("animation-left",     m_af_left        );
    lisp->get("animation-straight", m_af_straight    );
    lisp->get("animation-right",    m_af_right       );
    lisp->get("animation-speed",    m_animation_speed);

    loadWheelInfo(lisp, "wheel-front-right", 0);
    loadWheelInfo(lisp, "wheel-front-left",  1);
    loadWheelInfo(lisp, "wheel-rear-right",  2);
    loadWheelInfo(lisp, "wheel-rear-left",   3);
}   // init
// ----------------------------------------------------------------------------
/** Destructor.
 */
KartModel::~KartModel()
{
}  // ~KartModel

// ----------------------------------------------------------------------------
/** Attach the kart model and wheels to the scene node.
 *  \param node Node to attach the models to.
 */
void KartModel::attachModel(scene::IAnimatedMeshSceneNode **node)
{
    m_node = *node = irr_driver->addAnimatedMesh(m_mesh);
    m_node->setAnimationSpeed(1500);
    m_node->setLoopMode(false);
    for(unsigned int i=0; i<4; i++)
    {
        m_wheel_node[i] = irr_driver->addMesh(m_wheel_model[i]);
        m_wheel_node[i]->setPosition(m_wheel_graphics_position[i].toIrrVector());
        (*node)->addChild(m_wheel_node[i]);
    }
}   // attachModel

// ----------------------------------------------------------------------------
/** Loads the 3d model and all wheels.
 */
void KartModel::loadModels(const KartProperties &kart_properties)
{
    std::string  full_path = kart_properties.getKartDir()+"/"+m_model_filename;
    m_mesh                 = irr_driver->getAnimatedMesh(full_path);
    Vec3 min, max;
    MeshTools::minMax3D(m_mesh, &min, &max);
    Vec3 size = max-min;
    m_z_offset    = min.getZ();
    m_kart_width  = size.getX();
    m_kart_height = size.getZ();
    m_kart_length = size.getY();
    // FIXME: How do we handle this? it's a mesh only, so we can't
    // simply move it in a transform (unless we turn it into a scene 
    // node). m_z_offset should probably be made available to kart.
    // Vec3 move_kart_to_0_z(0, 0, m_z_offset);
    // m_root->setTransform(move_kart_to_0_z);

    // Now set default some default parameters (if not defined) that 
    // depend on the size of the kart model (wheel position, center
    // of gravity shift)
    for(unsigned int i=0; i<4; i++)
    {
        if(m_wheel_graphics_position[i].getX()==UNDEFINED)
        {
            m_wheel_graphics_position[i].setX( ( i==1||i==3) 
                                               ? -0.5f*m_kart_width
                                               :  0.5f*m_kart_width  );
            m_wheel_graphics_position[i].setY( (i<2) ?  0.5f*m_kart_length
                                                     : -0.5f*m_kart_length);
            m_wheel_graphics_position[i].setZ(0);
        }
    }

    // Load the wheel models. This can't be done early, since the default
    // values for the graphical position must be defined, which in turn
    // depend on the size of the model.
    for(unsigned int i=0; i<4; i++)
    {
        std::string full_wheel = 
            kart_properties.getKartDir()+"/"+m_wheel_filename[i];
        m_wheel_model[i] = irr_driver->getMesh(full_wheel);
        // FIXME: wheel handling still missing.
    }   // for i<4
    if(!m_wheel_model[0])
    {
        m_z_offset = m_kart_height*0.5f;
    }
}   // load

// ----------------------------------------------------------------------------
/** Loads a single wheel node. Currently this is the name of the wheel model
 *  and the position of the wheel relative to the kart.
 *  \param wheel_name Name of the wheel, e.g. wheel-rear-left.
 *  \param index Index of this wheel in the global m_wheel* fields.
 */
void KartModel::loadWheelInfo(const lisp::Lisp* const lisp,
                              const std::string &wheel_name, int index)
{
    const lisp::Lisp* const wheel = lisp->getLisp(wheel_name);
    if(!wheel)
    {
        // Only print the warning if a model filename is given. Otherwise the
        // stk_config file is read (which has no model information).
        if(m_model_filename!="")
        {
            fprintf(stderr, "Missing wheel information '%s' for model '%s'.\n",
                wheel_name.c_str(), m_model_filename.c_str());
            fprintf(stderr, "This can be ignored, but the wheels will not rotate.\n");
        }
        return;
    }
    wheel->get("model",            m_wheel_filename[index]         );
    wheel->get("position",         m_wheel_graphics_position[index]);
    wheel->get("physics-position", m_wheel_physics_position[index] );
    wheel->get("min-suspension",   m_min_suspension[index]         );
    wheel->get("max-suspension",   m_max_suspension[index]         );
}   // loadWheelInfo

// ----------------------------------------------------------------------------
/** Sets the default position for the physical wheels if they are not defined
 *  in the data file. The default position is to have the wheels at the corner
 *  of the chassis. But since the position is relative to the center of mass,
 *  this must be specified.
 *  \param center_shift Amount the kart chassis is moved relative to the center
 *                      of mass.
 *  \param wheel_radius Radius of the physics wheels.
 */
void  KartModel::setDefaultPhysicsPosition(const Vec3 &center_shift,
                                           float wheel_radius)
{
    for(unsigned int i=0; i<4; i++)
    {
        if(m_wheel_physics_position[i].getX()==UNDEFINED)
        {
            m_wheel_physics_position[i].setX( ( i==1||i==3) 
                                               ? -0.5f*m_kart_width
                                               :  0.5f*m_kart_width
                                               +center_shift.getX(  ));
            m_wheel_physics_position[i].setY( (0.5f*m_kart_length-wheel_radius)
                                              * ( (i<2) ? 1 : -1)
                                               +center_shift.getY());
            // Set the connection point so that a maximum compressed wheel
            // (susp. length=0) will still poke a little bit out under the 
            // kart
            m_wheel_physics_position[i].setZ(wheel_radius-0.05f);
        }   // if physics position is not defined
    }

}   // setDefaultPhysicsPosition

// ----------------------------------------------------------------------------
/** Rotates and turns the wheels appropriately, and adjust for suspension.
 *  \param rotation How far the wheels should rotate.
 *  \param visual_steer How much the front wheels are turned for steering.
 *  \param steer The actual steer settings.
 *  \param suspension Suspension height for all four wheels.
 */
void KartModel::update(float rotation, float visual_steer,
                       float steer, const float suspension[4])
{

    float clamped_suspension[4];
    // Clamp suspension to minimum and maximum suspension length, so that
    // the graphical wheel models don't look too wrong.
    for(unsigned int i=0; i<4; i++)
    {
        const float suspension_length = (m_max_suspension[i]-m_min_suspension[i])/2;
        
        // limit amplitude between set limits, first dividing it by a
        // somewhat arbitrary constant to reduce visible wheel movement
        clamped_suspension[i] = std::min(std::max(suspension[i]/m_dampen_suspension_amplitude[i],
                                                  m_min_suspension[i]),
                                                  m_max_suspension[i]);
        float ratio = clamped_suspension[i] / suspension_length;
        const int sign = ratio < 0 ? -1 : 1;
        ratio = sign * fabsf(ratio*(2-ratio)); // expanded form of 1 - (1 - x)^2, i.e. making suspension display quadratic and not linear
        clamped_suspension[i] = ratio*suspension_length;
    }   // for i<4

//    core::vector3df wheel_rear (RAD_TO_DEGREE(-rotation), 0, 0);
    core::vector3df wheel_rear (-rotation, 0, 0);
    core::vector3df wheel_steer(0, -visual_steer, 0);
    core::vector3df wheel_front = wheel_rear+wheel_steer;

    for(unsigned int i=0; i<4; i++)
    {
        core::vector3df pos =  m_wheel_graphics_position[i].toIrrVector();
        pos.Y += clamped_suspension[i];
        m_wheel_node[i]->setPosition(pos);
    }
    m_wheel_node[0]->setRotation(wheel_front);
    m_wheel_node[1]->setRotation(wheel_front);
    m_wheel_node[2]->setRotation(wheel_rear );
    m_wheel_node[3]->setRotation(wheel_rear );

    if(m_af_left<0) return;   // no animations defined

    // Update animation if necessary
    // -----------------------------
    // FIXME: this implementation is currently very simple, it will always
    // animate to the very left or right, even if actual steering is only
    // (say) 50% of left or right.
    int end;
    static int last_end=-1;
    if(steer>0.0f)       end = m_af_straight-(int)((m_af_straight-m_af_left)*steer);
    else if(steer<0.0f)  end = m_af_straight+(int)((m_af_straight-m_af_right)*steer);
    else                 end = m_af_straight;

    // No changes to current frame loop
    if(end==last_end) return;

    int begin = (int)m_node->getFrameNr();
    last_end = end;
    // Handle reverse animation, which are done by setting
    // the animation speed to a negative number.
    if(begin<end)
    {
        m_node->setFrameLoop(begin, end);
        m_node->setAnimationSpeed(m_animation_speed);
    }
    else
    {
        m_node->setFrameLoop(begin, end);
        m_node->setAnimationSpeed(-m_animation_speed);
    }
}   // update

// ----------------------------------------------------------------------------
/** Puts all wheels in the default position. Used when displaying the karts
 *  in the character selection screen.
 */
void KartModel::resetWheels()
{
    const float suspension[4]={0,0,0,0};
    update(0, 0, 0.0f, suspension);
}   // reset
