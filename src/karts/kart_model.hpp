//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <IAnimatedMeshSceneNode.h>
namespace irr
{
    namespace scene { class IAnimatedMesh; class IMesh;
                      class ISceneNode; class IMeshSceneNode; }
}
using namespace irr;
namespace GE { class GERenderInfo; }

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class AbstractKart;
class KartProperties;
class MovingTexture;
class XMLNode;

/** A speed-weighted object is an object whose characteristics are influenced by the kart's speed */
struct SpeedWeightedObject
{
    /** Parameters for a speed-weighted object */
    struct Properties
    {
        Properties();

        ~Properties();

        Properties& operator=(const Properties& other);

        /** Strength factor: how much the kart speed affects the animation's distance from a static pose (-1 to disable) */
        float               m_strength_factor;

        /** Speed factor: how much the kart speed affects the animation's speed (-1 to disable) */
        float               m_speed_factor;

        MovingTexture*      m_moving_texture;

        void    loadFromXMLNode(const XMLNode* xml_node);

    };

    SpeedWeightedObject() : m_model(NULL), m_node(NULL), m_name() {}
    /** Model */
    scene::IAnimatedMesh *              m_model;

    /** The scene node the speed weighted model is attached to */
    scene::IAnimatedMeshSceneNode *     m_node;

    /** The relative matrix to the parent kart scene node
     *  where the speed weighted object is attached to. */
    core::matrix4                       m_location;

    /** Filename of the "speed weighted" object */
    std::string                         m_name;

    /** Attach to which bone in kart model if not empty. */
    std::string                         m_bone_name;

    /** Specific properties for this given speed-weighted object,
      * otherwise just a copy of the values from the kart's properties */
    Properties                          m_properties;
};
typedef std::vector<SpeedWeightedObject>    SpeedWeightedObjectList;

// ============================================================================
/** A class to store the headlights of a kart.
 */
class HeadlightObject
{
private:
    /** The filename of the headlight model. */
    std::string m_filename;

    /** The relative matrix to the parent kart scene node
     *  where the headlight mesh is attached to. */
    core::matrix4 m_location;

    /** The mesh for the headlight. */
    scene::IMesh* m_model;

    /** The scene node of the headlight (real light). */
    scene::ISceneNode* m_node;

    /** The color of the real light. */
    video::SColor m_headlight_color;

    /** Attach to which bone in kart model if not empty. */
    std::string m_bone_name;

public:

    HeadlightObject()
    {
        m_model    = NULL;
        m_node     = NULL;
    }   // HeadlightObject
    // ------------------------------------------------------------------------
    HeadlightObject(const std::string& filename, const core::matrix4& location,
                    const std::string& bone_name, const video::SColor& color)
    {
        m_filename = filename;
        m_location = location;
        m_model    = NULL;
        m_node     = NULL;
        m_bone_name = bone_name;
        m_headlight_color = color;
    }   // HeadlightObjects
    // ------------------------------------------------------------------------
    const std::string& getFilename() const { return m_filename; }
    // ------------------------------------------------------------------------
    /** Sets the mesh for this headlight object. */
    void setModel(scene::IMesh *mesh) { m_model = mesh; }
    // ------------------------------------------------------------------------
    void setLight(scene::ISceneNode* parent, float energy, float radius);
    // ------------------------------------------------------------------------
    const scene::ISceneNode *getLightNode() const { return m_node;  }
    // ------------------------------------------------------------------------
    scene::ISceneNode *getLightNode() { return m_node; }
    // ------------------------------------------------------------------------
    const scene::IMesh *getModel() const { return m_model;  }
    // ------------------------------------------------------------------------
    scene::IMesh *getModel() { return m_model; }
    // ------------------------------------------------------------------------
    const core::matrix4& getLocation() const { return m_location; }
    // ------------------------------------------------------------------------
    const std::string& getBoneName() const { return m_bone_name; }
    // ------------------------------------------------------------------------
};   // class HeadlightObject

// ============================================================================

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
            AF_LOSE_END_STRAIGHT,  // End losing animation to straight frame
            AF_BEGIN_EXPLOSION,    // Begin explosion animation
            AF_END_EXPLOSION,      // End explosion animation
            AF_JUMP_START,         // Begin of jump
            AF_JUMP_LOOP,          // Begin of jump loop
            AF_JUMP_END,           // End of jump
            AF_WIN_START,          // Begin of win animation
            AF_WIN_LOOP_START,     // Begin of win loop animation
            AF_WIN_END,            // End of win animation
            AF_WIN_END_STRAIGHT,   // End of win animation to straight frame
            AF_SELECTION_START,    // Start frame in kart selection screen
            AF_SELECTION_END,      // End frame in kart selection screen
            AF_BACK_LEFT,          // Going back left
            AF_BACK_STRAIGHT,      // Going back straight
            AF_BACK_RIGHT,         // Going back right
            AF_END=AF_BACK_RIGHT,  // Last animation frame
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

    /** Location of hat in object space. */
    core::matrix4* m_hat_location;

    /** Name of the bone for hat attachment. */
    std::string m_hat_bone;

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

    /** Radius of the graphical wheels.  */
    float         m_wheel_graphics_radius[4];
    
    /** The position of the nitro emitters */
    Vec3          m_nitro_emitter_position[2];

    /** True if kart has nitro emitters */
    bool          m_has_nitro_emitter;

    /** The speed weighted objects. */
    SpeedWeightedObjectList     m_speed_weighted_objects;
    
    std::vector<HeadlightObject> m_headlight_objects;

    /** Length of the physics suspension when the kart is at rest. */
    float m_default_physics_suspension[4];

    /** Minimum suspension length (i.e. most compressed). If the displayed
     *  suspension is shorter than this, the wheel would look wrong. */
    float         m_min_suspension[4];

    /** Maximum suspension length (i.e. most extended). If the displayed
     *  suspension is any longer, the wheel would look too far away from the
     *  chassis. */
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
     *  anything attached to it etc. */
    bool  m_is_master;

    void  loadWheelInfo(const XMLNode &node,
                        const std::string &wheel_name, int index);
    
    void  loadNitroEmitterInfo(const XMLNode &node,
                        const std::string &emitter_name, int index);

    void  loadSpeedWeightedInfo(const XMLNode* speed_weighted_node, int index);

    void  loadHeadlights(const XMLNode &node);

    void OnAnimationEnd(scene::IAnimatedMeshSceneNode *node);

    /** Pointer to the kart object belonging to this kart model. */
    AbstractKart* m_kart;

    /** For our engine to get the desired hue for colorization. */
    std::shared_ptr<GE::GERenderInfo> m_render_info;

    /** True if this kart model can be colorization in red / blue (now only
     *  used in soccer mode). */
    bool m_support_colorization;

    /** Used to cache inverse bone matrices for each bone in straight frame
     *  for attachment. */
    std::unordered_map<std::string, core::matrix4> m_inverse_bone_matrices;

    /** Version of kart model (in kart.xml).  */
    unsigned m_version;

    /** Exhaust particle file (xml) for the kart, empty if disabled.  */
    std::string m_exhaust_xml;

    const KartProperties* m_kart_properties;
    // ------------------------------------------------------------------------
    void initInverseBoneMatrices();
    // ------------------------------------------------------------------------
    void configNode(scene::ISceneNode* node, const core::matrix4& global_mat,
                    const core::matrix4& inv_mat)
    {
        const core::matrix4 mat = inv_mat * global_mat;
        const core::vector3df position = mat.getTranslation();
        const core::vector3df rotation = mat.getRotationDegrees();
        const core::vector3df scale = mat.getScale();
        node->setPosition(position);
        node->setRotation(rotation);
        node->setScale(scale);
    }

public:
                  KartModel(bool is_master);
                 ~KartModel();
    KartModel*    makeCopy(std::shared_ptr<GE::GERenderInfo> ri);
    void          reset();
    void          loadInfo(const XMLNode &node);
    bool          loadModels(const KartProperties &kart_properties);
    void          setDefaultSuspension();
    void          update(float dt, float distance, float steer, float speed,
                         float current_lean_angle,
                         int gt_replay_index = -1);
    void          finishedRace();
    void          resetVisualWheelPosition();
    scene::ISceneNode*
                  attachModel(bool animatedModels, bool human_player);
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
    int  getFrame(AnimationFrameType f) const  { return m_animation_frame[f]; }
    // ------------------------------------------------------------------------
    float  getAnimationSpeed() const              { return m_animation_speed; }
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
    /** Returns information about currently played animation */
    AnimationFrameType getAnimation() { return m_current_animation; }
    // ------------------------------------------------------------------------
    /** Enables- or disables the end animation. */
    void  setAnimation(AnimationFrameType type, bool play_non_loop = false);
    // ------------------------------------------------------------------------
    /** Sets the kart this model is currently used for */
    void  setKart(AbstractKart* k) { m_kart = k; }
    // ------------------------------------------------------------------------
    /**  Name of the hat mesh to use. */
    void setHatMeshName(const std::string &name) {m_hat_name = name; }
    // ------------------------------------------------------------------------
    /** Returns the array of wheel nodes. */
    scene::ISceneNode** getWheelNodes() { return m_wheel_node; }
    // ------------------------------------------------------------------------
    scene::IAnimatedMeshSceneNode* getAnimatedNode(){ return m_animated_node; }
    // ------------------------------------------------------------------------
    std::shared_ptr<GE::GERenderInfo> getRenderInfo();
    // ------------------------------------------------------------------------
    bool supportColorization() const         { return m_support_colorization; }
    // ------------------------------------------------------------------------
    void toggleHeadlights(bool on);
    // ------------------------------------------------------------------------
    const core::matrix4&
                      getInverseBoneMatrix(const std::string& bone_name) const;
    // ------------------------------------------------------------------------
    const std::string& getExhaustXML() const          { return m_exhaust_xml; }
    // ------------------------------------------------------------------------
    bool hasWheel() const              { return !m_wheel_filename[0].empty(); }
    // ------------------------------------------------------------------------
    const KartProperties* getKartProperties() const
                                                  { return m_kart_properties; }
};   // KartModel
#endif
