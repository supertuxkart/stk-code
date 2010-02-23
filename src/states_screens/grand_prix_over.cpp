
#include "states_screens/grand_prix_over.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
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
    
    m_podium_x[0] = 1.4f;
    m_podium_z[0] = 0.0f;
    
    m_podium_x[1] = 2.2f;
    m_podium_z[1] = 0.5f;
    
    m_podium_x[2] = 3.0f;
    m_podium_z[2] = 0.0f;
}

// -------------------------------------------------------------------------------------

void traverse(scene::ISceneNode* curr, int level=0)
{
    for (int n=0; n<level; n++) std::cout << "|    ";
    
    unsigned int type = curr->getType();
    const char* ptr = (const char*)&type;
    
    std::cout << "+ " << curr->getName() << " ("
              << char(ptr[0]) << char(ptr[1])
              << char(ptr[2]) << char(ptr[3]) << std::endl;
    
    const core::list<  scene::ISceneNode  * >& children = curr->getChildren();
    for (core::list<scene::ISceneNode*>::ConstIterator it=children.begin(); it != children.end(); it++)
    {
        traverse(*it, level+1);
    }
    
}

void GrandPrixOver::init()
{
    sound_manager->startMusic(sound_manager->getMusicInformation(file_manager->getMusicFile("win_theme.music")));

    m_phase = 1;
    m_sky_angle = 0.0f;
    m_global_time = 0.0f;
    
    m_sky = irr_driver->addSkyDome(file_manager->getTextureFile("clouds.png"),
                                   16 /* hori_res */, 16 /* vert_res */,
                           1.0f /* texture_percent */,  2.0f /* sphere_percent */);
    
    m_camera = irr_driver->addCameraSceneNode();
    m_camera_x = 3.0f;
    m_camera_y = 0.0f;
    m_camera_z = -5.0f;
    m_camera->setPosition( core::vector3df(m_camera_x, m_camera_y, m_camera_z) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    
    m_camera_target_x = 1.5f;
    m_camera_target_z = 0.0f;
    m_camera->setTarget( core::vector3df(m_camera_target_x, -2.0f, m_camera_target_z) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    
    scene::IMesh* model_village = irr_driver->getMesh( file_manager->getModelFile("village.b3d") );
    assert(model_village != NULL);
    m_village = irr_driver->addMesh(model_village);
    m_village->setPosition( core::vector3df(2, INITIAL_Y, 0) );
    
    
    scene::IMesh* podium_model = irr_driver->getMesh( file_manager->getModelFile("wood_podium.b3d") );
    assert(podium_model != NULL);
    
    
    m_podium_step[0] = irr_driver->addMesh(podium_model);
    m_podium_step[0]->setPosition( core::vector3df(m_podium_x[0], INITIAL_PODIUM_Y, m_podium_z[0]) );
    
    m_podium_step[1] = irr_driver->addMesh(podium_model);
    m_podium_step[1]->setPosition( core::vector3df(m_podium_x[1], INITIAL_PODIUM_Y, m_podium_z[1]) );
    
    m_podium_step[2] = irr_driver->addMesh(podium_model);
    m_podium_step[2]->setPosition( core::vector3df(m_podium_x[2], INITIAL_PODIUM_Y, m_podium_z[2]) );
    
    scene::ISceneManager* sceneManager = irr_driver->getSceneManager();
    sceneManager->setAmbientLight(video::SColor(255, 120, 120, 120));
    
    const core::vector3df &sun_pos = core::vector3df( 0, 200, 100.0f );
    m_light = irr_driver->getSceneManager()->addLightSceneNode(NULL, sun_pos,
                                                               video::SColorf(1.0f,1.0f,1.0f),
                                                               300.0f /* radius */);
    m_light->getLightData().DiffuseColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 0.0f, 0.0f, 0.0f);
    
    sfx_manager->quickSound("winner");
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

    // restore menu music when leaving (FIXME: this assume we always go to menu after)
    sound_manager->stopMusic();
    sound_manager->startMusic(stk_config->m_title_music);

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
                    karts_not_yet_done++;
                }

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
    
    if (m_phase > 1)
    {
        //m_camera_x = 3.0f;
        if (m_camera_z < -2.0f)                m_camera_z        += dt*0.2f;
        if (m_camera_x < m_podium_x[1])        m_camera_x        += dt*0.1f;
        else if (m_camera_x > m_podium_x[1])   m_camera_x        -= dt*0.1f;

        if (m_camera_target_x < m_podium_x[1]) m_camera_target_x += dt*0.1f;
        
        if (m_camera_y > -1.8f)                m_camera_y        -= dt*0.1f;
        //else  if (m_camera_y < -3.0f)          m_camera_y        += dt*0.1f;

        
        m_camera->setTarget( core::vector3df(m_camera_target_x, -2.0f, m_camera_target_z) );
        
        m_camera->setPosition( core::vector3df(m_camera_x, m_camera_y, m_camera_z) );
        m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
        m_camera->updateAbsolutePosition();
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
            
            m_kart_x[n] = m_podium_x[n];
            m_kart_y[n] = INITIAL_Y;
            m_kart_z[n] = -4;
            m_kart_rotation[n] = 0.0f;
            
            kart_main_node = irr_driver->addMesh(kartModel->getModel());
            kart_main_node->setPosition( core::vector3df(m_kart_x[n], m_kart_y[n], m_kart_z[n]) );
            kart_main_node->setScale( core::vector3df(0.4f, 0.4f, 0.4f)  );
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
