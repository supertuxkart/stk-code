
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

FeatureUnlockedCutScene::FeatureUnlockedCutScene() : Screen("feature_unlocked.stkgui")
{
    setNeeds3D(true);
    
    throttleFPS = false;
    m_unlocked_kart = NULL;
    m_unlocked_thing_picture = NULL;
}

void FeatureUnlockedCutScene::setUnlockedKart(KartProperties* unlocked_kart)
{
    assert(unlocked_kart != NULL);
    m_unlocked_kart = unlocked_kart;
    m_unlocked_thing_picture = NULL;
}

void FeatureUnlockedCutScene::setUnlockedPicture(irr::video::ITexture* picture)
{
    assert(picture != NULL);

    m_unlocked_kart = NULL;
    m_unlocked_thing_picture = picture;
}

void FeatureUnlockedCutScene::init()
{
    m_sky_angle = 0.0f;
    m_global_time = 0.0f;
    
    m_sky = irr_driver->addSkyDome(file_manager->getTextureFile("lscales.png"), 16 /* hori_res */, 16 /* vert_res */,
                           1.0f /* texture_percent */,  2.0f /* sphere_percent */);
    
    m_camera = irr_driver->addCameraSceneNode();
    m_camera->setPosition( core::vector3df(0.0, 30.0f, 70.0f) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    m_camera->setTarget( core::vector3df(0, 10, 0.0f) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    scene::IMesh* model_chest = item_manager->getOtherModel("chest_bottom");
    scene::IMesh* model_chest_top = item_manager->getOtherModel("chest_top");
    scene::IMesh* model_key = item_manager->getOtherModel("key");
    
    m_chest = irr_driver->addMesh(model_chest);
    m_chest_top = irr_driver->addMesh(model_chest_top);
    m_key = irr_driver->addMesh(model_key);

    m_chest->setPosition( core::vector3df(-3, -3, 0) );
    m_chest_top->setPosition( core::vector3df(-3, -3, 0) );
    m_key_pos = 45.0f;
    m_key_angle = 0.0f;
    m_key->setPosition( core::vector3df(0, 0, m_key_pos) );
    
    const int materials = m_key->getMaterialCount();
    for (int n=0; n<materials; n++)
    {
        m_key->getMaterial(n).setFlag(EMF_LIGHTING, true);
        
        m_key->getMaterial(n).Shininess = 100.0f; // set size of specular highlights
        m_key->getMaterial(n).SpecularColor.set(255,50,50,50); 
        m_key->getMaterial(n).DiffuseColor.set(255,150,150,150);
        
        m_key->getMaterial(n).setFlag(EMF_GOURAUD_SHADING , true);
        
        m_key->getMaterial(n).MaterialType = video::EMT_SPHERE_MAP;
    }
    
    m_key->setScale( core::vector3df(0.8f, 0.8f, 0.8f) );
    
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
        // TODO
        video::SMaterial m;
        m.BackfaceCulling = false;
        m.setTexture(0, m_unlocked_thing_picture);
        
        scene::IMesh* mesh = irr_driver->createTexturedQuadMesh(&m, 1.0, 0.75);
        m_root_gift_node   = irr_driver->addMesh(mesh);

    }
    else
    {
        std::cerr << "There is nothing in the chest!!!\n";
    }
    
    /*
    for (unsigned int i=0; i<sky->getMaterialCount(); i++)
    {
        video::SMaterial &irrMaterial = sky->getMaterial(i);
        for (unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
        {
            video::ITexture* t = irrMaterial.getTexture(j);
            if(!t) continue;
            core::matrix4 *m = &irrMaterial.getTextureMatrix(j);
            m_animated_textures.push_back(new MovingTexture(m, 0.05f, 0.0f));
        }   // for j<MATERIAL_MAX_TEXTURES
    }   // for i<getMaterialCount
     */
}
void FeatureUnlockedCutScene::tearDown()
{
    printf("+++++++ FeatureUnlockedCutScene:tearDown +++++++++\n");
    
    irr_driver->removeNode(m_sky);
    m_sky = NULL;
    
    irr_driver->removeCameraSceneNode(m_camera);
    m_camera = NULL;
    
    irr_driver->removeNode(m_chest);
    irr_driver->removeNode(m_chest_top);
    irr_driver->removeNode(m_key);
    m_chest = NULL;
    m_chest_top = NULL;
    m_key = NULL;
    
    irr_driver->removeNode(m_light);
    m_light = NULL;
    
    if (m_root_gift_node != NULL)
    {
        irr_driver->removeNode(m_root_gift_node);
        m_root_gift_node = NULL;
    }
}

void FeatureUnlockedCutScene::onUpdate(float dt, irr::video::IVideoDriver* driver)
{
    m_global_time += dt;
    
    m_sky_angle += dt*2;
    if (m_sky_angle > 360) m_sky_angle -= 360;
    m_sky->setRotation( core::vector3df(0, m_sky_angle, 0) );

    const float KEY_Y = 6.8f;
    const float KEY_FINAL_DIST = 15;
    
    if (m_key_pos > KEY_FINAL_DIST) m_key_pos -= dt*5;
    m_key->setPosition( core::vector3df(0, KEY_Y, m_key_pos) );
    
    // distance at which the key starts rotating
    const float KEY_ROTATION_FROM = 30.0f;
    // distance at which the key finishes rotating
    const float KEY_ROTATION_TO = KEY_FINAL_DIST + 10.0f;
    
    //const float CHEST_OPEN_FROM = KEY_ROTATION_TO + 1.0f;
    //const float CHEST_OPEN_TO = KEY_ROTATION_TO + 8.0f;

    m_key_angle = 1.0f - (m_key_pos - KEY_ROTATION_TO) / (KEY_ROTATION_FROM - KEY_ROTATION_TO);
    if (m_key_angle < 0.0f) m_key_angle = 0.0f;
    else if (m_key_angle > 1.0f) m_key_angle = 1.0f;

    
    //printf("m_key_angle = %f\n", m_key_angle);
    m_key->setRotation( core::vector3df(0, m_key_angle*90.0f, -m_key_angle*90.0f) );

    const int GIFT_EXIT_FROM = 7;
    const int GIFT_EXIT_TO = 20;
        
    if (m_global_time > GIFT_EXIT_FROM && m_global_time < GIFT_EXIT_TO && m_root_gift_node != NULL)
    {
        const double chest_top_angle = ((double)(m_global_time - GIFT_EXIT_FROM)*3/(double)GIFT_EXIT_TO)*110.0;
        m_chest_top->setRotation( core::vector3df( 360.0f-(float)std::min(110.0, chest_top_angle), 0, 0 ));
        if (chest_top_angle < 110.0) 
        {
            core::vector3df chestpos = m_chest_top->getPosition();
            chestpos.Y += dt*6;
            m_chest_top->setPosition(chestpos);
        }
        
        core::vector3df pos = m_root_gift_node->getPosition();
        pos.Y = sin( (float)((m_global_time - GIFT_EXIT_FROM)*M_PI*1.5/GIFT_EXIT_TO)  )*50.0f;
        pos.X += 5*dt;
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

void FeatureUnlockedCutScene::eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}
