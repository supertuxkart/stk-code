
#include "states_screens/grand_prix_over.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <SColor.h>
#include <iostream>

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

const float INITIAL_Y = -3.0f;
const float INITIAL_PODIUM_Y = -3.6f;
const float PODIUM_HEIGHT[3] = { 0.325f, 0.5f, 0.15f };

// -------------------------------------------------------------------------------------

GrandPrixOver::GrandPrixOver() : Screen("grand_prix_over.stkgui")
{
    setNeeds3D(true);
    
    throttleFPS = false;
    
    m_kart_node[0] = NULL;
    m_kart_node[1] = NULL;
    m_kart_node[2] = NULL;
}

// -------------------------------------------------------------------------------------

void GrandPrixOver::init()
{
    m_phase = 1;
    m_sky_angle = 0.0f;
    m_global_time = 0.0f;
    
    m_sky = irr_driver->addSkyDome(file_manager->getTextureFile("clouds.png"),
                                   16 /* hori_res */, 16 /* vert_res */,
                           1.0f /* texture_percent */,  2.0f /* sphere_percent */);
    
    m_camera = irr_driver->addCameraSceneNode();
    m_camera->setPosition( core::vector3df(3.0, 0.0f, -5.0f) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    m_camera->setTarget( core::vector3df(1.5f, -2.0f, 0.0f) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    
    scene::IMesh* model_village = irr_driver->getMesh( file_manager->getModelFile("village.b3d") );
    assert(model_village != NULL);
    m_village = irr_driver->addMesh(model_village);
    m_village->setPosition( core::vector3df(2, INITIAL_Y, 0) );
    
    
    scene::IMesh* podium_model = irr_driver->getMesh( file_manager->getModelFile("wood_podium.b3d") );
    assert(podium_model != NULL);
    
    
    m_podium_x[0] = 1.4f;
    m_podium_z[0] = 0.0f;
    
    m_podium_x[1] = 2.2f;
    m_podium_z[1] = 0.5f;
    
    m_podium_x[2] = 3.0f;
    m_podium_z[2] = 0.0f;
    
    m_podium_step[0] = irr_driver->addMesh(podium_model);
    m_podium_step[0]->setPosition( core::vector3df(m_podium_x[0], INITIAL_PODIUM_Y, m_podium_z[0]) );
    
    m_podium_step[1] = irr_driver->addMesh(podium_model);
    m_podium_step[1]->setPosition( core::vector3df(m_podium_x[1], INITIAL_PODIUM_Y, m_podium_z[1]) );
    
    m_podium_step[2] = irr_driver->addMesh(podium_model);
    m_podium_step[2]->setPosition( core::vector3df(m_podium_x[2], INITIAL_PODIUM_Y, m_podium_z[2]) );
    
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
    
    irr_driver->removeNode(m_village);
    m_village = NULL;
    
    for (int n=0; n<3; n++)
    {
        irr_driver->removeNode(m_podium_step[n]);
        m_podium_step[n] = NULL;
        if (m_kart_node[n] != NULL) irr_driver->removeNode(m_kart_node[n]);
        m_kart_node[n] = NULL;
    }
}

// -------------------------------------------------------------------------------------

void GrandPrixOver::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    m_global_time += dt;
    
    m_sky_angle += dt*2;
    if (m_sky_angle > 360) m_sky_angle -= 360;
    m_sky->setRotation( core::vector3df(0, m_sky_angle, 0) );

        
    // ---- karts move
    if (m_phase == 1)
    {
        int karts_not_yet_done = 0;
        for (int k=0; k<3; k++)
        {
            if (m_kart_node[k] != NULL)
            {
                bool x_target_reached = false, z_target_reached = false;
                
                if (fabsf(m_kart_x[k] - m_podium_x[k]) > dt)
                {
                    if (m_kart_x[k] < m_podium_x[k])
                    {
                        m_kart_x[k] += dt;
                    }
                    else if (m_kart_x[k] > m_podium_x[k])
                    {
                        m_kart_x[k] -= dt;
                    }
                }
                else
                {
                    m_kart_x[k] = m_podium_x[k];
                    x_target_reached = true;
                }
                
                if (fabsf(m_kart_z[k] - m_podium_z[k]) > dt)
                {
                    if (m_kart_z[k] < m_podium_z[k])
                    {
                        m_kart_z[k] += dt;
                    }
                    else if (m_kart_z[k] > m_podium_z[k])
                    {
                        m_kart_z[k] -= dt;
                    }
                }
                else
                {
                    m_kart_z[k] = m_podium_z[k];
                    z_target_reached=true;
                }
                
                
                if (!x_target_reached || !z_target_reached) karts_not_yet_done++;
                
                //std::cout << "kart position [" << k << "] = " << m_kart_x[k] << ", " << m_kart_z[k] << std::endl;
                
                m_kart_node[k]->setPosition( core::vector3df(m_kart_x[k], m_kart_y[k], m_kart_z[k]) );
            }
        } // end for
        
        if (karts_not_yet_done == 0)
        {
            m_phase = 2;
        }
    }
    
    // ---- Karts Rotate
    else if (m_phase == 2)
    {
        int karts_not_yet_done = 0;
        for (int k=0; k<3; k++)
        {
            if (m_kart_node[k] != NULL)
            {
                if (m_kart_rotation[k] < 180.f)
                {
                    m_kart_rotation[k] += 25.0f*dt;
                    m_kart_node[k]->setRotation( core::vector3df(0, m_kart_rotation[k], 0) );
                    m_podium_step[k]->setRotation( core::vector3df(0, m_kart_rotation[k], 0) );
                    karts_not_yet_done++;
                }
            }
        } // end for
        
        if (karts_not_yet_done == 0) m_phase = 3;
    }
    
    // ---- Podium Rises
    else if (m_phase == 3)
    {
        for (int k=0; k<3; k++)
        {
            if (m_kart_node[k] != NULL)
            {
                const float y_target = INITIAL_Y + PODIUM_HEIGHT[k];
                if (m_kart_y[k] < y_target)
                {
                    m_kart_y[k] += dt*(PODIUM_HEIGHT[k]);
                    m_kart_node[k]->setPosition(   core::vector3df(m_kart_x[k], m_kart_y[k], m_kart_z[k]) );
                    m_podium_step[k]->setPosition( core::vector3df(m_podium_x[k],
                                                                   INITIAL_PODIUM_Y - (INITIAL_Y - m_kart_y[k]),
                                                                   m_podium_z[k]) );

                }
            }
        } // end for
        
    }
    
    // ---- title
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

void GrandPrixOver::setKarts(const std::string idents_arg[3])
{
    // reorder in "podium order" (i.e. second player to the left, first player in the middle, last at the right)
    std::string idents[3];
    idents[0] = idents_arg[1];
    idents[1] = idents_arg[0];
    idents[2] = idents_arg[2];

    for (int n=0; n<3; n++)
    {
        if (idents[n].size() == 0) continue;
        
        scene::ISceneNode* kart_main_node = NULL;
        
        const KartProperties* kart = kart_properties_manager->getKart(idents[n]);
        if (kart != NULL)
        {
            KartModel* kartModel = kart->getKartModel();
            
            m_kart_x[n] = n*2;
            m_kart_y[n] = INITIAL_Y;
            m_kart_z[n] = -4;
            m_kart_rotation[n] = 0.0f;
            
            kart_main_node = irr_driver->addMesh(kartModel->getModel());
            kart_main_node->setPosition( core::vector3df(m_kart_x[n], m_kart_y[n], m_kart_z[n]) );
            kart_main_node->setScale( core::vector3df(0.4f, 0.4, 0.4f)  );
            kart_main_node->updateAbsolutePosition();
            
            for (int wheel=0; wheel<4; wheel++)
            {
                scene::ISceneNode* wheel_model = irr_driver->getSceneManager()->addMeshSceneNode(
                                                 kartModel->getWheelModel(wheel),
                                                 kart_main_node);
                wheel_model->setPosition( kartModel->getWheelGraphicsPosition(wheel).toIrrVector() );
                wheel_model->updateAbsolutePosition();
            }
        }
        
        m_kart_node[n] = kart_main_node;
    }
}

// -------------------------------------------------------------------------------------
