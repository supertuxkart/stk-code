#ifndef HEADER_GRAND_PRIX_OVER_HPP
#define HEADER_GRAND_PRIX_OVER_HPP

#include "guiengine/screen.hpp"

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; class ILightSceneNode; } }
class KartProperties;


class GrandPrixOver : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<GrandPrixOver>
{
    friend class GUIEngine::ScreenSingleton<GrandPrixOver>;
    
    GrandPrixOver();
    
    /** sky angle, 0-360 */
    float m_sky_angle;
    
    /** Global evolution of time */
    double m_global_time;
    
    
    irr::scene::ISceneNode* m_sky;
    irr::scene::ICameraSceneNode* m_camera;

    irr::scene::ILightSceneNode* m_light;
public:

    void onUpdate(float dt, irr::video::IVideoDriver*);
    
    void init();
    void tearDown();
    
    void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);

};

#endif

