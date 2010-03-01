#ifndef HEADER_FEATURE_UNLOCKED_HPP
#define HEADER_FEATURE_UNLOCKED_HPP

#include "guiengine/screen.hpp"

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; class ILightSceneNode; } }
class KartProperties;


class FeatureUnlockedCutScene : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<FeatureUnlockedCutScene>
{
    friend class GUIEngine::ScreenSingleton<FeatureUnlockedCutScene>;
    
    FeatureUnlockedCutScene();
    
    /** Whichever of these is non-null decides whhat comes out of the chest */
    KartProperties* m_unlocked_kart;
    irr::video::ITexture* m_unlocked_thing_picture;
    
    /** Contains whatever is in the chest */
    scene::ISceneNode* m_root_gift_node;
    
    /** sky angle, 0-360 */
    float m_sky_angle;
    
    /** Global evolution of time */
    double m_global_time;
    
    /** Key position from origin (where the chest is) */
    float m_key_pos;
    
    /** Angle of the key (from 0 to 1, simply traces progression) */
    float m_key_angle;
    
    irr::scene::ISceneNode* m_sky;
    irr::scene::ICameraSceneNode* m_camera;
    irr::scene::IAnimatedMeshSceneNode* m_chest;
    //irr::scene::ISceneNode* m_chest_top;
    //irr::scene::ISceneNode* m_key;
    irr::scene::ILightSceneNode* m_light;
    
    void continueButtonPressed();
    
public:

    void onUpdate(float dt, irr::video::IVideoDriver*);
    
    void init();
    void tearDown();
    
    void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);
    
    /** Call before showing up the screen to make a kart come out of the chest */
    void setUnlockedKart(KartProperties* unlocked_kart);
    
    /** Call before showing up the screen to make a picture come out of the chest */
    void setUnlockedPicture(irr::video::ITexture* picture);
    
    /** override from base class to handle escape press */
    virtual bool onEscapePressed();
};

#endif

