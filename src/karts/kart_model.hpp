//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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
#include <vector>

#include <IAnimatedMeshSceneNode.h>
namespace irr
{
    namespace scene { class IAnimatedMesh; class IMesh;
                      class ISceneNode; class IMeshSceneNode; }
}
using namespace irr;

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class AbstractKart;
class KartProperties;
class XMLNode;

/** A speed-weighted object is an object whose characteristics are influenced by the kart's speed */
struct SpeedWeightedObject
{
    /** Parameters for a speed-weighted object */
    struct Properties
    {
        Properties();

        /** Strength factor: how much the kart speed affects the animation's distance from a static pose (-1 to disable) */
        float               m_strength_factor;

        /** Speed factor: how much the kart speed affects the animation's speed (-1 to disable) */
        float               m_speed_factor;

        /** Texture speed, in UV coordinates */
        core::vector2df     m_texture_speed;

        void    loadFromXMLNode(const XMLNode* xml_node);

        void    checkAllSet();
    };

    SpeedWeightedObject() : m_model(NULL), m_node(NULL), m_position(), m_name() {}
    /** Model */
    scene::IAnimatedMesh *              m_model;

    /** The scene node the speed weighted model is attached to */
    scene::IAnimatedMeshSceneNode *     m_node;

    /** The position of the "speed weighted" objects relative to the kart */
    Vec3                                m_position;

    /** Filename of the "speed weighted" object */
    std::string                         m_name;

    /** Current uv translation in the texture matrix for speed-weighted texture animations */
    core::vector2df                     m_texture_cur_offset;

    /** Specific properties for this given speed-weighted object,
      * otherwise just a copy of the values from the kart's properties */
    Properties                          m_properties;
};
typedef std::vector<SpeedWeightedObject>    SpeedWeightedObjectList;

/**
 * \brief This class stores a 3D kart model.
 * It takes especially care of attaching
 *  the wheels, which are loaded as separate objects. The wheels can turn
 *  and (for the front wheels) rotate. The implementation is dependent on the
 *  OpenGL library used.
 *  Note that this object is copied using the default copy function. See
 *  kart.cpp.
 * \ingroup karts
 */
class KartModel : public scene::IAnimationEndCallBack, public NoCopy
{
public:
    enum   AnimationFrameType
           {AF_BEGIN,              // First animation frame
            AF_DEFAULT = AF_BEGIN, // Default, i.e. steering animation
            AF_LEFT,               // Steering to the left
            AF_STRAIGHT,           // Going straight
            AF_RIGHT,              // Steering to the right
            AF_LOSE_START,         // Begin losing animation
            AF_LOSE_LOOP_START,    // Begin of the losing loop
            AF_LOSE_END,           // End losing animation
            AF_BEGIN_EXPLOSION,    // Begin explosion animation
            AF_END_EXPLOSION,      // End explosion animation
            AF_JUMP_START,         // Begin of jump
            AF_JUMP_LOOP,          // Begin of jump loop
            AF_JUMP_END,           // End of jump
            AF_WIN_START,          // Begin of win animation
            AF_WIN_LOOP_START,     // Begin of win loop animation
            AF_WIN_END,            // End of win animation
            AF_SPEED_WEIGHTED_START,        // Start of speed-weighted animation
            AF_SPEED_WEIGHTED_END,          // End of speed-weighted animation
            AF_END=AF_SPEED_WEIGHTED_END,   // Last animation frame
            AF_COUNT};             // Number of entries here
private:
    /** Which frame number starts/end which animation. */
    int m_animation_frame[AF_COUNT];

    /** Animation speed. */
    float m_animation_speed;

    /** The mesh of the model. */
    scene::IAnimatedMesh *m_mesh;

    /** This is a pointer to the scene node of the kart this model belongs
     *  to. It is necessary to adjust animations, and it is not used
     *  (i.e. neither read nor written) if animations are disabled. */
    scene::IAnimatedMeshSceneNode *m_animated_node;

    /** The scene node for a hat the driver is wearing. */
    scene::IMeshSceneNode *m_hat_node;

    /** Offset of the hat relative to the bone called 'head'. */
    core::vector3df m_hat_offset;

    /** Name of the hat to use for this kart. "" if no hat. */
    std::string m_hat_name;

    /** Value used to indicate undefined entries. */
    static float UNDEFINED;

    /** Name of the 3d model file. */
    std::string   m_model_filename;

    /** The four wheel models. */
    scene::IMesh *m_wheel_model[4];

    /** The four scene nodes the wheels are attached to */
    scene::ISceneNode *m_wheel_node[4];

    /** Filename of the wheel models. */
    std::string   m_wheel_filename[4];

    /** The position of all four wheels in the 3d model. */
    Vec3          m_wheel_graphics_position[4];

    /** The position of the wheels for the physics, which can be different
     *  from the graphical position. */
    Vec3          m_wheel_physics_position[4];

    /** Radius of the graphical wheels.  */
    float         m_wheel_graphics_radius[4];
    
    /** The position of the nitro emitters */
    Vec3          m_nitro_emitter_position[2];

    /** True if kart has nitro emitters */
    bool          m_has_nitro_emitter;

    /** The speed weighted objects. */
    SpeedWeightedObjectList     m_speed_weighted_objects;
    
    /** Minimum suspension length. If the displayed suspension is
     *  shorter than this, the wheel would look wrong. */
    float         m_min_suspension[4];

    /** Maximum suspension length. If the displayed suspension is
     *  any longer, the wheel would look too far away from the chassis. */
    float         m_max_suspension[4];

    /** value used to divide the visual movement of wheels (because the actual movement
        of wheels in bullet is too large and looks strange). 1=no change, 2=half the amplitude */
    float         m_dampen_suspension_amplitude[4];

    /** Which animation is currently being played. This is used to overwrite
     *  the default steering animations while being in race. If this is set
     *  to AF_DEFAULT the default steering animation is shown. */
    AnimationFrameType m_current_animation;

    /** Width of kart.  */
    float m_kart_width;

    /** Length of kart. */
    float m_kart_length;

    /** Height of kart. */
    float m_kart_height;

    /** Largest coordinate on up axis. */
    float m_kart_highest_point;

    /** Smallest coordinate on up axis. */
    float m_kart_lowest_point;

    /** True if this is the master copy, managed by KartProperties. This
     *  is mainly used for debugging, e.g. the master copies might not have
     * anything attached to it etc. */
    bool  m_is_master;

    void  loadWheelInfo(const XMLNode &node,
                        const std::string &wheel_name, int index);
    
    void  loadNitroEmitterInfo(const XMLNode &node,
                        const std::string &emitter_name, int index);

    void  loadSpeedWeightedInfo(const XMLNode* speed_weighted_node,
                                const SpeedWeightedObject::Properties& fallback_properties);

    void OnAnimationEnd(scene::IAnimatedMeshSceneNode *node);

    /** Pointer to the kart object belonging to this kart model. */
    AbstractKart* m_kart;

public:
                  KartModel(bool is_master);
                 ~KartModel();
    KartModel*    makeCopy();
    void          reset();
    void          loadInfo(const XMLNode &node);
    bool          loadModels(const KartProperties &kart_properties);
    void          update(float dt, float rotation_dt, float steer,
                         const float height_abve_terrain[4], float speed);
    void          setDefaultPhysicsPosition(const Vec3 &center_shift,
                                            float wheel_radius);
    void          finishedRace();
    scene::ISceneNode*
                  attachModel(bool animatedModels, bool always_animated);
    // ------------------------------------------------------------------------
    /** Returns the animated mesh of this kart model. */
    scene::IAnimatedMesh*
                  getModel() const { return m_mesh; }

    // ------------------------------------------------------------------------
    /** Returns the mesh of the wheel for this kart. */
    scene::IMesh* getWheelModel(const int i) const
                             { assert(i>=0 && i<4); return m_wheel_model[i]; }
    // ------------------------------------------------------------------------
    /** Since karts might be animated, we might need to know which base frame
     *  to use. */
    int  getBaseFrame() const   { return m_animation_frame[AF_STRAIGHT];  }
    // ------------------------------------------------------------------------
    /** Returns the position of a wheel relative to the kart.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear
     *           right, 3 = rear left.  */
    const Vec3& getWheelGraphicsPosition(unsigned int i) const
                {assert(i<4); return m_wheel_graphics_position[i];}
    // ------------------------------------------------------------------------
    /** Returns the position of wheels relative to the kart.
     */
    const Vec3* getWheelsGraphicsPosition() const
                {return m_wheel_graphics_position;}
    // ------------------------------------------------------------------------
    /** Returns the position of a wheel relative to the kart for the physics.
     *  The physics wheels can be attached at a different place to make the
     *  karts more stable.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear
     *           right, 3 = rear left.  */
    const Vec3& getWheelPhysicsPosition(unsigned int i) const
                {assert(i<4); return m_wheel_physics_position[i];}
    // ------------------------------------------------------------------------
    /** Returns the radius of the graphical wheels.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear
     *           right, 3 = rear left.  */
    float       getWheelGraphicsRadius(unsigned int i) const
                {assert(i<4); return m_wheel_graphics_radius[i]; }
    // ------------------------------------------------------------------------
    /** Returns the position of nitro emitter relative to the kart.
     *  \param i Index of the emitter: 0 = right, 1 = left
     */
    const Vec3& getNitroEmittersPositon(unsigned int i) const
                { assert(i<2);  return m_nitro_emitter_position[i]; }
    // ------------------------------------------------------------------------
    /** Returns true if kart has nitro emitters */
    const bool hasNitroEmitters() const
                {return m_has_nitro_emitter;}
    // ------------------------------------------------------------------------
    /** Returns the number of speed weighted objects for this kart */
    size_t      getSpeedWeightedObjectsCount() const
                {return m_speed_weighted_objects.size();}
    // ------------------------------------------------------------------------
    /** Returns the position of a speed weighted object relative to the kart.
     *  \param i Index of the object  */
    const SpeedWeightedObject& getSpeedWeightedObject(int i) const
                {return m_speed_weighted_objects[i];}
    // ------------------------------------------------------------------------
    /** Returns the length of the kart model. */
    float getLength                 () const {return m_kart_length;      }
    // ------------------------------------------------------------------------
    /** Returns the width of the kart model. */
    float getWidth                  () const {return m_kart_width;       }
    // ------------------------------------------------------------------------
    /** Returns the height of the kart. */
    float getHeight                 () const {return m_kart_height;      }
    // ------------------------------------------------------------------------
    /** Highest coordinate on up axis */
    float getHighestPoint           () const { return m_kart_highest_point;  }
    // ------------------------------------------------------------------------
    /** Lowest coordinate on up axis */
    float getLowestPoint           () const { return m_kart_lowest_point;  }
    // ------------------------------------------------------------------------
    /** Enables- or disables the end animation. */
    void  setAnimation(AnimationFrameType type);
    // ------------------------------------------------------------------------
    /** Sets the kart this model is currently used for */
    void  setKart(AbstractKart* k) { m_kart = k; }
    // ------------------------------------------------------------------------
    /**  Name of the hat mesh to use. */
    void setHatMeshName(const std::string &name) {m_hat_name = name; }
    // ------------------------------------------------------------------------
    void attachHat();
    // ------------------------------------------------------------------------
    /** Returns the array of wheel nodes. */
    scene::ISceneNode** getWheelNodes() { return m_wheel_node; }
    // ------------------------------------------------------------------------
    scene::IAnimatedMeshSceneNode* getAnimatedNode(){ return m_animated_node; }
    // ------------------------------------------------------------------------
    core::vector3df getHatOffset() { return m_hat_offset; }

};   // KartModel
#endif
