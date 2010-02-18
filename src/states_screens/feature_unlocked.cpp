
#include "states_screens/feature_unlocked.hpp"


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

FeatureUnlockedCutScene::FeatureUnlockedCutScene() : Screen("feature_unlocked.stkgui")
{
    setNeeds3D(true);
    
    throttleFPS = false;
    m_unlocked_kart = NULL;
    m_unlocked_thing_picture = NULL;
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::setUnlockedKart(KartProperties* unlocked_kart)
{
    assert(unlocked_kart != NULL);
    m_unlocked_kart = unlocked_kart;
    m_unlocked_thing_picture = NULL;
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::setUnlockedPicture(irr::video::ITexture* picture)
{
    assert(picture != NULL);

    m_unlocked_kart = NULL;
    m_unlocked_thing_picture = picture;
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::init()
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
    
    scene::IAnimatedMesh* model_chest = irr_driver->getAnimatedMesh( file_manager->getModelFile("chest.b3d") );
    assert(model_chest != NULL);
    m_chest = irr_driver->addAnimatedMesh(model_chest);
    m_chest->setPosition( core::vector3df(2, -3, 0) );
    m_chest->setScale( core::vector3df(10.0f, 10.0f, 10.0f) );

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 120, 120, 120));
    
    const core::vector3df &sun_pos = core::vector3df( 0, 200, 100.0f );
    m_light = irr_driver->getSceneManager()->addLightSceneNode(NULL, sun_pos, video::SColorf(1.0f,1.0f,1.0f), 10000.0f /* radius */);
    m_light->getLightData().DiffuseColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    
    if (m_unlocked_kart != NULL)
    {
        KartModel* kartModel = m_unlocked_kart->getKartModel();
        
        scene::ISceneNode* kart_node = irr_driver->getSceneManager()->addMeshSceneNode(kartModel->getModel());

        for (int n=0; n<4; n++)
        {
            scene::ISceneNode* wheel = irr_driver->getSceneManager()->addMeshSceneNode(kartModel->getWheelModel(n), kart_node);
            wheel->setPosition( kartModel->getWheelGraphicsPosition(n).toIrrVector() );
            wheel->updateAbsolutePosition();
        }
        
        m_root_gift_node = kart_node;
    }
    else if (m_unlocked_thing_picture != NULL)
    {
        video::SMaterial m;
        m.BackfaceCulling = false;
        m.setTexture(0, m_unlocked_thing_picture);
        m.AmbientColor = SColor(255,255,255,255);
        m.DiffuseColor = SColor(255,255,255,255);
        m.SpecularColor = SColor(0,0,0,0);
        m.GouraudShading = false;
        m.Shininess = 0;
        //m.setFlag(video::EMF_TEXTURE_WRAP, false);
        
#if (IRRLICHT_VERSION_MAJOR == 1) && (IRRLICHT_VERSION_MINOR >= 7)
        m.TextureLayer[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
        m.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
#else
        m.TextureLayer[0].TextureWrap = video::ETC_CLAMP_TO_EDGE;
#endif

        scene::IMesh* mesh = irr_driver->createTexturedQuadMesh(&m, 1.0, 0.75);
        m_root_gift_node   = irr_driver->addMesh(mesh);

    }
    else
    {
        std::cerr << "There is nothing in the chest!!!\n";
    }
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::tearDown()
{
    irr_driver->removeNode(m_sky);
    m_sky = NULL;
    
    irr_driver->removeCameraSceneNode(m_camera);
    m_camera = NULL;
    
    irr_driver->removeNode(m_chest);
    //irr_driver->removeNode(m_chest_top);
    //irr_driver->removeNode(m_key);
    m_chest = NULL;
    //m_chest_top = NULL;
    //m_key = NULL;
    
    irr_driver->removeNode(m_light);
    m_light = NULL;
    
    if (m_root_gift_node != NULL)
    {
        irr_driver->removeNode(m_root_gift_node);
        m_root_gift_node = NULL;
    }
}

// -------------------------------------------------------------------------------------

//FIXME: doesn't go here...
template<typename T>
T keepInRange(T from, T to, T value)
{
    if (value < from) return from;
    if (value > to  ) return to;
    return value;
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    m_global_time += dt;
    
    m_sky_angle += dt*2;
    if (m_sky_angle > 360) m_sky_angle -= 360;
    m_sky->setRotation( core::vector3df(0, m_sky_angle, 0) );

    const float ANIM_FROM = 2.0f;
    const float ANIM_TO = 5.5f;
    const int last_image = m_chest->getEndFrame() - 1;
    
    if (m_global_time < ANIM_FROM)
    {
        // progression of the chest rotation between 0 and 1
        const float rotationProgression = keepInRange( 0.0f, 1.0f, (float)sin(M_PI/2.0f*m_global_time/double(ANIM_FROM)) );
        const float chest_rotation = keepInRange(80.0f, 160.0f, (float)(80 + rotationProgression * 80) );
        m_chest->setRotation( core::vector3df(0.0f, chest_rotation, 0.0f) );
    }
    
    const float current_frame = (float)keepInRange(0.0, (double)last_image,
                                                  (m_global_time - ANIM_FROM)/(double)(ANIM_TO - ANIM_FROM) * last_image);
    //std::cout << "current_frame: " << current_frame << std::endl;
    m_chest->setCurrentFrame( current_frame );
       
    const int GIFT_EXIT_FROM = (int)ANIM_TO;
    const int GIFT_EXIT_TO = GIFT_EXIT_FROM + 12;

    if (m_global_time > GIFT_EXIT_FROM && m_global_time < GIFT_EXIT_TO && m_root_gift_node != NULL)
    {
        core::vector3df pos = m_root_gift_node->getPosition();
        pos.Y = sin( (float)((m_global_time - GIFT_EXIT_FROM)*M_PI*1.2/GIFT_EXIT_TO)  )*30.0f;
        pos.X += 2*dt;
        pos.Z += 5*dt;

        m_root_gift_node->setPosition(pos);
        
        core::vector3df scale = m_root_gift_node->getScale();
        scale.X += 2*dt;
        scale.Y += 2*dt;
        scale.Z += 2*dt;
        m_root_gift_node->setScale(scale);
        
        
        core::vector3df campos = m_camera->getPosition();
        campos.X += 5*dt;
        campos.Z += 5*dt;
        
        m_camera->setPosition(campos);
    }
    else if (m_global_time < GIFT_EXIT_FROM)
    {
        m_camera->setPosition( core::vector3df(cos((1.0f-m_key_angle)*M_PI/4 + M_PI/4)*70.0f,
                                               30.0f,
                                               sin((1.0f-m_key_angle)*M_PI/8 + M_PI/4)*70.0f) );
    }

    if (m_root_gift_node != NULL)
    {
        m_camera->setTarget( m_root_gift_node->getPosition() + core::vector3df(0.0f, 10.0f, 0.0f) );
        m_camera->updateAbsolutePosition();
    }
    else
    {
        m_camera->setTarget( core::vector3df(0, 10, 0.0f) );
        m_camera->updateAbsolutePosition();
    }
    
    static const int w = irr_driver->getFrameSize().Width;
    static const int h = irr_driver->getFrameSize().Height;
    const irr::video::SColor color(255, 255, 255, 255);
    
    static int test_y = 0;
    
    GUIEngine::getTitleFont()->draw(_("Feature Unlocked"),
                                    core::rect< s32 >( 0, test_y, w, h/10 ),
                                    color,
                                    true/* center h */, true /* center v */ );
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}

// -------------------------------------------------------------------------------------
