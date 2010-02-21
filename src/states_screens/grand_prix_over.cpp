
#include "states_screens/grand_prix_over.hpp"


#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <SColor.h>

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

// -------------------------------------------------------------------------------------

GrandPrixOver::GrandPrixOver() : Screen("grand_prix_over.stkgui")
{
    setNeeds3D(true);
    
    throttleFPS = false;
}

// -------------------------------------------------------------------------------------

void GrandPrixOver::init()
{
    m_sky_angle = 0.0f;
    m_global_time = 0.0f;
    
    //m_sky = irr_driver->addSkyDome(file_manager->getTextureFile("lscales.png"), 16 /* hori_res */, 16 /* vert_res */,
    //                       1.0f /* texture_percent */,  2.0f /* sphere_percent */);
    
    std::vector<std::string> texture_names(6);
    texture_names[0] = file_manager->getTextureFile("purplenebula.png");
    texture_names[1] = file_manager->getTextureFile("purplenebula2.png");
    texture_names[2] = file_manager->getTextureFile("purplenebula.png");
    texture_names[3] = file_manager->getTextureFile("purplenebula2.png");
    texture_names[4] = file_manager->getTextureFile("purplenebula.png");
    texture_names[5] = file_manager->getTextureFile("purplenebula2.png");
    m_sky = irr_driver->addSkyBox(texture_names);
    
    m_camera = irr_driver->addCameraSceneNode();
    m_camera->setPosition( core::vector3df(0.0, 30.0f, 70.0f) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    m_camera->setTarget( core::vector3df(0, 10, 0.0f) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 120, 120, 120));
    
    const core::vector3df &sun_pos = core::vector3df( 0, 200, 100.0f );
    m_light = irr_driver->getSceneManager()->addLightSceneNode(NULL, sun_pos, video::SColorf(1.0f,1.0f,1.0f), 10000.0f /* radius */);
    m_light->getLightData().DiffuseColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);

}

// -------------------------------------------------------------------------------------

void GrandPrixOver::tearDown()
{
    irr_driver->removeNode(m_sky);
    m_sky = NULL;
    
    irr_driver->removeCameraSceneNode(m_camera);
    m_camera = NULL;
    
    irr_driver->removeNode(m_light);
    m_light = NULL;
}

// -------------------------------------------------------------------------------------

void GrandPrixOver::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    m_global_time += dt;
    
    m_sky_angle += dt*2;
    if (m_sky_angle > 360) m_sky_angle -= 360;
    m_sky->setRotation( core::vector3df(0, m_sky_angle, 0) );

        
    static const int w = irr_driver->getFrameSize().Width;
    static const int h = irr_driver->getFrameSize().Height;
    const irr::video::SColor color(255, 255, 255, 255);
    
    static int test_y = 0;
    
    GUIEngine::getTitleFont()->draw(_("Grand Prix Results"),
                                    core::rect< s32 >( 0, test_y, w, h/10 ),
                                    color,
                                    true/* center h */, true /* center v */ );
}

// -------------------------------------------------------------------------------------

void GrandPrixOver::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    if (name == "continue")
    {
        // we assume the main menu was pushed before showing this menu
        StateManager::get()->popMenu();
    }
}

// -------------------------------------------------------------------------------------
