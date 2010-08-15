
#include "states_screens/grand_prix_win.hpp"

#include <SColor.h>
#include <iostream>

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

const float INITIAL_Y = -3.0f;
const float INITIAL_PODIUM_Y = -3.6f;
const float PODIUM_HEIGHT[3] = { 0.325f, 0.5f, 0.15f };

DEFINE_SCREEN_SINGLETON( GrandPrixWin );

// -------------------------------------------------------------------------------------

GrandPrixWin::GrandPrixWin() : Screen("grand_prix_win.stkgui")
{
    setNeeds3D(true);
    
    m_throttle_FPS = false;
    
    m_music = music_manager->getMusicInformation(file_manager->getMusicFile("win_theme.music"));
}   // GrandPrixWin

// -------------------------------------------------------------------------------------

void GrandPrixWin::loadedFromFile()
{
    m_kart_node[0] = NULL;
    m_kart_node[1] = NULL;
    m_kart_node[2] = NULL;
    
    m_podium_x[0] = 1.4f;
    m_podium_z[0] = 0.0f;
    
    m_podium_x[1] = 2.2f;
    m_podium_z[1] = 0.5f;
    
    m_podium_x[2] = 3.0f;
    m_podium_z[2] = 0.0f;
}   // loadedFromFile

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
}   // traverse

// -------------------------------------------------------------------------------------

void GrandPrixWin::init()
{
    Screen::init();
    if (unlock_manager->getRecentlyUnlockedFeatures().size() > 0)
    {
        const core::dimension2d<u32>& frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();

        
        core::stringw message = _("You unlocked a new feature!");
        const int message_width = GUIEngine::getFont()->getDimension(message.c_str()).Width + 30;
        
        const int label_height = GUIEngine::getFontHeight() + 15;
        
        const int y_from       = frame_size.Height - label_height*2;
        const int y_to         = frame_size.Height - label_height;
        
        const int label_x_from = frame_size.Width/2 - message_width/2;
        const int label_x_to   = frame_size.Width/2 + message_width/2;

        // button_h is used in the x coordinates not by mistake, but because the icon is square and
        // scaled according to the available height.
        core::rect< s32 > iconarea(label_x_from - label_height, y_from,
                                   label_x_from,                y_to);
        IGUIImage* img = GUIEngine::getGUIEnv()->addImage( iconarea );
        img->setImage( irr_driver->getTexture( file_manager->getTextureFile("cup_gold.png") ) );
        img->setScaleImage(true);
        img->setTabStop(false);
        img->setUseAlphaChannel(true);
        
        core::rect< s32 > icon2area(label_x_to,                y_from,
                                    label_x_to + label_height, y_to);
        img = GUIEngine::getGUIEnv()->addImage( icon2area );
        img->setImage( irr_driver->getTexture( file_manager->getTextureFile("cup_gold.png") ) );
        img->setScaleImage(true);
        img->setTabStop(false);
        img->setUseAlphaChannel(true);
        
        GUIEngine::LabelWidget* unlocked_label = new GUIEngine::LabelWidget();
        unlocked_label->m_properties[GUIEngine::PROP_ID] = "label";
        unlocked_label->m_properties[GUIEngine::PROP_TEXT_ALIGN] = "center";
        unlocked_label->m_x = label_x_from;
        unlocked_label->m_y = y_from;
        unlocked_label->m_w = message_width;
        unlocked_label->m_h = label_height;
        unlocked_label->m_text = message;
        //const irr::video::SColor orange(255, 255, 126, 21);
        //unlocked_label->setColor(orange);
        
        unlocked_label->add();
    }
    
    //music_manager->startMusic(music_manager->getMusicInformation(file_manager->getMusicFile("win_theme.music")));

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
    irr_driver->getSceneManager()->setActiveCamera(m_camera);

    m_camera_target_x = 1.5f;
    m_camera_target_z = 0.0f;
    m_camera->setTarget( core::vector3df(m_camera_target_x, -2.0f, m_camera_target_z) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    
    scene::IMesh* model_village = irr_driver->getMesh( file_manager->getModelFile("village.b3d") );
    assert(model_village != NULL);
    m_village = irr_driver->addMesh(model_village);
#ifdef DEBUG
    m_village->setName("village");
#endif
    m_village->setPosition( core::vector3df(2, INITIAL_Y, 0) );
    
    
    scene::IMesh* podium_model = irr_driver->getMesh( file_manager->getModelFile("wood_podium.b3d") );
    assert(podium_model != NULL);
    
    
    m_podium_step[0] = irr_driver->addMesh(podium_model);
#ifdef DEBUG
    m_podium_step[0]->setName("Podium 0");
#endif
    m_podium_step[0]->setPosition( core::vector3df(m_podium_x[0], INITIAL_PODIUM_Y, m_podium_z[0]) );
    
    m_podium_step[1] = irr_driver->addMesh(podium_model);
#ifdef DEBUG
    m_podium_step[1]->setName("Podium 1");
#endif
    m_podium_step[1]->setPosition( core::vector3df(m_podium_x[1], INITIAL_PODIUM_Y, m_podium_z[1]) );
    
    m_podium_step[2] = irr_driver->addMesh(podium_model);
#ifdef DEBUG
    m_podium_step[2]->setName("Podium 2");
#endif
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
}   // init

// -------------------------------------------------------------------------------------

void GrandPrixWin::tearDown()
{
    Screen::tearDown();
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
}   // tearDown

// -------------------------------------------------------------------------------------

void GrandPrixWin::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    m_global_time += dt;
        
    m_sky_angle += dt*2;
    if (m_sky_angle > 360) m_sky_angle -= 360;
    m_sky->setRotation( core::vector3df(0, m_sky_angle, 0) );

        
    // ---- karts move
    if (m_phase == 1)
    {
        assert(m_kart_node[0] != NULL || m_kart_node[1] != NULL || m_kart_node[2] != NULL);
        
        int karts_not_yet_done = 0;
        for (int k=0; k<3; k++)
        {
            if (m_kart_node[k] != NULL)
            {

                if (fabsf(m_kart_z[k] - m_podium_z[k]) > dt)
                {
                    if (m_kart_z[k] < m_podium_z[k] - dt)
                    {
                        m_kart_z[k] += dt;
                    }
                    else if (m_kart_z[k] > m_podium_z[k] + dt)
                    {
                        m_kart_z[k] -= dt;
                    }
                    else
                    {
                        m_kart_z[k] = m_podium_z[k];
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
        if (m_camera_z < -2.0f)                          m_camera_z        += dt*0.2f;
        
        if      (m_camera_x < m_podium_x[1] - dt*0.1f)   m_camera_x        += dt*0.1f;
        else if (m_camera_x > m_podium_x[1] + dt*0.1f)   m_camera_x        -= dt*0.1f;
        else                                             m_camera_x         = m_podium_x[1];
            
        if (m_camera_target_x < m_podium_x[1])           m_camera_target_x += dt*0.1f;
        
        if (m_camera_y > -1.8f)                          m_camera_y        -= dt*0.1f;

        
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
    
    GUIEngine::getTitleFont()->draw(_("You won the Grand Prix!"),
                                    core::rect< s32 >( 0, test_y, w, h/10 ),
                                    color,
                                    true/* center h */, true /* center v */ );
}   // onUpdate

// -------------------------------------------------------------------------------------

void GrandPrixWin::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    if (name == "continue")
    {
        // un-set the GP mode so that after unlocking, it doesn't try to continue the GP
        race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
        
        if (unlock_manager->getRecentlyUnlockedFeatures().size() > 0)
        {
            std::vector<const Challenge*> unlocked = unlock_manager->getRecentlyUnlockedFeatures();
            unlock_manager->clearUnlocked();
            
            FeatureUnlockedCutScene* scene = FeatureUnlockedCutScene::getInstance();
            
            assert(unlocked.size() > 0);
            scene->addUnlockedThings(unlocked);
            
            StateManager::get()->replaceTopMostScreen(scene);
        }
        else
        {
            // we assume the main menu was pushed before showing this menu
            StateManager::get()->popMenu();
        }
    }
}   // eventCallback

// -------------------------------------------------------------------------------------

void GrandPrixWin::setKarts(const std::string idents_arg[3])
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
            KartModel* kart_model = kart->getKartModel();
            assert(kart_model != NULL);
            
            m_kart_x[n] = m_podium_x[n];
            m_kart_y[n] = INITIAL_Y;
            m_kart_z[n] = -4;
            m_kart_rotation[n] = 0.0f;
            
            kart_model->attachModel(&kart_main_node);
            assert(kart_main_node != NULL);
            kart_main_node->setPosition( core::vector3df(m_kart_x[n], m_kart_y[n], m_kart_z[n]) );
            kart_main_node->setScale( core::vector3df(0.4f, 0.4f, 0.4f)  );
        }
        else
        {
            std::cerr << "GrandPrixWin : warning : kart '" << idents[n] << "' not found!\n";
        }
        
        m_kart_node[n] = kart_main_node;
    } // end for
    
    assert(m_kart_node[0] != NULL || m_kart_node[1] != NULL || m_kart_node[2] != NULL);
}   // setKarts

// -------------------------------------------------------------------------------------
