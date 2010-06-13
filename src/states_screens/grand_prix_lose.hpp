#ifndef HEADER_GRAND_PRIX_LOSE_HPP
#define HEADER_GRAND_PRIX_LOSE_HPP

#include "guiengine/screen.hpp"

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; class ILightSceneNode; } }
class KartProperties;

/**
  * \brief Screen shown at the end of a Grand Prix
  * \ingroup states_screens
  */
class GrandPrixLose : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<GrandPrixLose>
{
    friend class GUIEngine::ScreenSingleton<GrandPrixLose>;
    
    GrandPrixLose();
    
    /** sky angle, 0-360 */
    float m_sky_angle;
    
    /** Global evolution of time */
    double m_global_time;
    
    irr::scene::IMeshSceneNode* m_garage;

    irr::scene::IAnimatedMeshSceneNode* m_garage_door;

    irr::scene::ISceneNode* m_kart_node;
    
    irr::scene::ISceneNode* m_sky;
    irr::scene::ICameraSceneNode* m_camera;

    irr::scene::ILightSceneNode* m_light;
    
    int m_phase;
    
    float m_kart_x, m_kart_y, m_kart_z;
    
    float m_camera_x, m_camera_y, m_camera_z;
    float m_camera_target_x, m_camera_target_z;

    MusicInformation* m_music;
    
public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();
    
    /** \brief implement optional callback from parent class GUIEngine::Screen */
    void onUpdate(float dt, irr::video::IVideoDriver*);
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void init();
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void tearDown();
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);
    
    void setKart(const std::string ident);

    virtual MusicInformation* getMusic() const { return m_music; }

};

#endif

