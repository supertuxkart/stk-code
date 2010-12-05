
#include "states_screens/grand_prix_lose.hpp"

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

const float DURATION = 15.0f;

const float CAMERA_END_X = -15.0f;
const float CAMERA_END_Y = 1.5f;
const float CAMERA_END_Z = 5.0f;
const float CAMERA_START_X = -17.0f;
const float CAMERA_START_Y = 2.0f;
const float CAMERA_START_Z = 5.5f;

const float DISTANCE_BETWEEN_KARTS = 2.0f;


const float KART_START_X = -17.0f;
const float KART_END_X = -5.0f;
const float KART_Y = -3.0f;
const float KART_Z = 0.0f;


const float GARAGE_DOOR_OPEN_TIME = 6.0f;

const int MAX_KART_COUNT = 4;

DEFINE_SCREEN_SINGLETON( GrandPrixLose );

// -------------------------------------------------------------------------------------

GrandPrixLose::GrandPrixLose() : Screen("grand_prix_lose.stkgui")
{
    setNeeds3D(true);
    
    m_throttle_FPS = false;
    
    try
    {
        m_music = music_manager->getMusicInformation(file_manager->getMusicFile("lose_theme.music"));
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s", e.what());
        m_music = NULL;
    }
}   // GrandPrixLose

// -------------------------------------------------------------------------------------

void GrandPrixLose::loadedFromFile()
{
    m_kart_node[0] = NULL;
    m_kart_node[1] = NULL;
    m_kart_node[2] = NULL;
    m_kart_node[3] = NULL;
}   // loadedFromFile

// -------------------------------------------------------------------------------------

void GrandPrixLose::init()
{
    Screen::init();

    m_phase = 1;
    m_sky_angle = 0.0f;
    m_global_time = 0.0f;
    
    m_sky = irr_driver->addSkyDome(file_manager->getTextureFile("clouds.png"),
                                   16 /* hori_res */, 16 /* vert_res */,
                                   1.0f /* texture_percent */,  2.0f /* sphere_percent */);
    
    m_camera = irr_driver->addCameraSceneNode();
    m_camera_x = CAMERA_START_X;
    m_camera_y = CAMERA_START_Y;
    m_camera_z = CAMERA_START_Z;
    m_camera->setPosition( core::vector3df(m_camera_x, m_camera_y, m_camera_z) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    irr_driver->getSceneManager()->setActiveCamera(m_camera);

    m_camera_target_x = 0.0f;
    m_camera_target_z = -2.0f;
    m_camera->setTarget( core::vector3df(m_camera_target_x, -2.0f, m_camera_target_z) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    
    scene::IAnimatedMesh* model_garage_door = irr_driver->getAnimatedMesh( file_manager->getModelFile("gplose_door.b3d") );
    assert(model_garage_door!= NULL);
    m_garage_door = irr_driver->addAnimatedMesh(model_garage_door);
#ifdef DEBUG
    m_garage_door->setName("garage-door");
#endif
    m_garage_door->setPosition( core::vector3df(2, INITIAL_Y, 0) );
    m_garage_door->setAnimationSpeed(0);
    
    scene::IMesh* model_garage = irr_driver->getMesh( file_manager->getModelFile("gplose.b3d") );
    assert(model_garage!= NULL);
    m_garage = irr_driver->addMesh(model_garage);
#ifdef DEBUG
    m_garage->setName("garage");
#endif

    m_garage->setPosition( core::vector3df(2, INITIAL_Y, 0) );    
    
    scene::ISceneManager* sceneManager = irr_driver->getSceneManager();
    sceneManager->setAmbientLight(video::SColor(255, 120, 120, 120));
    
    const core::vector3df &sun_pos = core::vector3df( 0, 200, 100.0f );
    m_light = irr_driver->getSceneManager()->addLightSceneNode(NULL, sun_pos,
                                                               video::SColorf(1.0f,1.0f,1.0f),
                                                               300.0f /* radius */);
    m_light->getLightData().DiffuseColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 0.0f, 0.0f, 0.0f);
}   // init

// -------------------------------------------------------------------------------------

void GrandPrixLose::tearDown()
{
    Screen::tearDown();
    irr_driver->removeNode(m_sky);
    m_sky = NULL;
    
    irr_driver->removeCameraSceneNode(m_camera);
    m_camera = NULL;
    
    irr_driver->removeNode(m_light);
    m_light = NULL;
    
    irr_driver->removeNode(m_garage);
    irr_driver->removeNode(m_garage_door);
    m_garage = NULL;
    
    for(unsigned int i=0; i<m_all_kart_models.size(); i++)
        delete m_all_kart_models[i];

    m_all_kart_models.clear();

    for (int n=0; n<MAX_KART_COUNT; n++)
    {
        if (m_kart_node[n] != NULL)
        {
            irr_driver->removeNode(m_kart_node[n]);
        }
    }
    
}   // tearDown

// -------------------------------------------------------------------------------------

void GrandPrixLose::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    m_global_time += dt;
    
    m_sky_angle += dt*2;
    if (m_sky_angle > 360) m_sky_angle -= 360;
    m_sky->setRotation( core::vector3df(0, m_sky_angle, 0) );

    const int lastFrame = m_garage_door->getEndFrame();
    if (m_global_time < GARAGE_DOOR_OPEN_TIME)
    {
        m_garage_door->setCurrentFrame( (m_global_time/GARAGE_DOOR_OPEN_TIME)*lastFrame );
    }
    else if (m_global_time > DURATION - GARAGE_DOOR_OPEN_TIME)
    {
        m_garage_door->setCurrentFrame( (1.0f - ((m_global_time -
                                                  (DURATION - GARAGE_DOOR_OPEN_TIME))/GARAGE_DOOR_OPEN_TIME))
                                       *lastFrame );
    }
    //else if (m_global_time < DURATION)
    //{
    //    m_garage_door->setCurrentFrame( lastFrame );
    //}
    
    const float kartProgression = m_global_time/(DURATION - 6.0f);
    if (kartProgression <= 1.0f)
    {
        m_kart_x = KART_START_X + (KART_END_X - KART_START_X)*kartProgression;
        
        for (int n=0; n<MAX_KART_COUNT; n++)
        {
            if (m_kart_node[n] != NULL)
            {
                m_kart_node[n]->setPosition( core::vector3df(m_kart_x + n*DISTANCE_BETWEEN_KARTS,
                                                             m_kart_y,
                                                             m_kart_z) );
            }
        }
    }
    
    const float progression = m_global_time / DURATION;
    if (progression <= 1.5f)
    {
        m_camera_x = CAMERA_START_X + (CAMERA_END_X - CAMERA_START_X)*progression;
        m_camera_y = CAMERA_START_Y + (CAMERA_END_Y - CAMERA_START_Y)*progression;
        m_camera_z = CAMERA_START_Z + (CAMERA_END_Z - CAMERA_START_Z)*progression;
    }
    
    m_camera->setPosition( core::vector3df(m_camera_x, m_camera_y, m_camera_z) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    m_camera->updateAbsolutePosition();
    
    // ---- title
    const int w = irr_driver->getFrameSize().Width;
    const int h = irr_driver->getFrameSize().Height;
    const irr::video::SColor color(255, 255, 255, 255);
    
    static int test_y = 0;
    
    //I18N: when failing a GP
    GUIEngine::getTitleFont()->draw(_("Better luck next time!"),
                                    core::rect< s32 >( 0, test_y, w, h ),
                                    color,
                                    true/* center h */, false /* center v */ );
}   // onUpdate

// -------------------------------------------------------------------------------------

void GrandPrixLose::eventCallback(GUIEngine::Widget* widget,
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

void GrandPrixLose::setKarts(std::vector<std::string> ident_arg)
{    
    scene::ISceneNode* kart_main_node = NULL;
    
    assert(ident_arg.size() > 0);
    if ((int)ident_arg.size() > MAX_KART_COUNT)
    {
        ident_arg.resize(MAX_KART_COUNT);
    }
    
    // (there is at least one kart so kart node 0 is sure to be set)
    m_kart_node[1] = NULL;
    m_kart_node[2] = NULL;
    m_kart_node[3] = NULL;
    
    const int count = ident_arg.size();
    for (int n=0; n<count; n++)
    {
        const KartProperties* kart = kart_properties_manager->getKart(ident_arg[n]);
        if (kart != NULL)
        {
            KartModel* kart_model = kart->getKartModelCopy();
            m_all_kart_models.push_back(kart_model);
            m_kart_x = KART_START_X;
            m_kart_y = KART_Y;
            m_kart_z = KART_Z;
            
            kart_model->attachModel(&kart_main_node);
            kart_main_node->setPosition( core::vector3df(m_kart_x, m_kart_y, m_kart_z) );
            //kart_main_node->setScale( core::vector3df(0.4f, 0.4f, 0.4f)  );
            kart_main_node->updateAbsolutePosition();
            kart_main_node->setRotation(vector3df(0, 90, 0));
            kart_model->setAnimation(KartModel::AF_DEFAULT);
            float susp[4]={0,0,0,0};
            kart_model->update(0.0f, 0.0f, susp);
        }
        else
        {
            fprintf(stderr, "[GrandPrixLose] WARNING: could not find a kart named '%s'\n", ident_arg[n].c_str());
        }// if kart !=NULL
        
        m_kart_node[n] = kart_main_node;
    }
    
    /*
    const int w = UserConfigParams::m_width;
    const int h = UserConfigParams::m_height;
    
    switch (count)
    {
        case 1:
            m_viewport[0] = core::recti(0, 0, w, h);
            break;
            
        case 2:
            m_viewport[0] = core::recti(0, 0, w, h/2);
            m_viewport[1] = core::recti(0, h/2, w, h);
            break;
            
        case 3:
            m_viewport[0] = core::recti(0, 0, w/2, h/2);
            m_viewport[1] = core::recti(w/2, 0, 2, h/2);
            m_viewport[2] = core::recti(0, h/2, w, h);
            break;
            
        case 4:
            m_viewport[0] = core::recti(0, 0, w/2, h/2);
            m_viewport[1] = core::recti(w/2, 0, 2, h/2);
            m_viewport[2] = core::recti(0, h/2, w/2, h);
            m_viewport[3] = core::recti(w/2, h/2, w, h);
            break;
            
        default:
            assert(false);
    }
     */
    
}   // setKarts

// -------------------------------------------------------------------------------------

