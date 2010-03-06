
#include "states_screens/feature_unlocked.hpp"


#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "states_screens/main_menu_screen.hpp"
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
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedKart(KartProperties* unlocked_kart, irr::core::stringw msg)
{
    assert(unlocked_kart != NULL);
    m_unlocked_stuff.push_back( new UnlockedThing(unlocked_kart, msg) );
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedPicture(irr::video::ITexture* picture,
                                                 float w, float h, irr::core::stringw msg)
{
    assert(picture != NULL);

    m_unlocked_stuff.push_back( new UnlockedThing(picture, w, h, msg) );
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
    
    const int unlockedStuffCount = m_unlocked_stuff.size();
    
    if (unlockedStuffCount == 0)  std::cerr << "There is nothing in the unlock chest!!!\n";
    
    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_unlocked_kart != NULL)
        {
            KartModel* kartModel = m_unlocked_stuff[n].m_unlocked_kart->getKartModel();
            
            scene::ISceneNode* kart_node = irr_driver->getSceneManager()->addMeshSceneNode(kartModel->getModel());

            for (int w=0; w<4; w++)
            {
                scene::ISceneNode* wheel = irr_driver->getSceneManager()->addMeshSceneNode(kartModel->getWheelModel(w), kart_node);
                wheel->setPosition( kartModel->getWheelGraphicsPosition(w).toIrrVector() );
                wheel->updateAbsolutePosition();
            }
            
            m_unlocked_stuff[n].m_root_gift_node = kart_node;
        }
        else if (m_unlocked_stuff[n].m_picture != NULL)
        {
            video::SMaterial m;
            m.MaterialType    = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            m.BackfaceCulling = false;
            m.setTexture(0, m_unlocked_stuff[n].m_picture);
            m.AmbientColor  = video::SColor(255, 255, 255, 255);
            m.DiffuseColor  = video::SColor(255, 255, 255, 255);
            m.EmissiveColor = video::SColor(255, 255, 255, 255);
            m.SpecularColor = video::SColor(255, 255, 255, 255);
            m.GouraudShading = false;
            m.Shininess      = 0;
            //m.setFlag(video::EMF_TEXTURE_WRAP, false);
            
    #if (IRRLICHT_VERSION_MAJOR == 1) && (IRRLICHT_VERSION_MINOR >= 7)
            m.TextureLayer[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
            m.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
    #else
            m.TextureLayer[0].TextureWrap = video::ETC_CLAMP_TO_EDGE;
    #endif

            scene::IMesh* mesh = irr_driver->createTexturedQuadMesh(&m,
                                                                    m_unlocked_stuff[n].m_w,
                                                                    m_unlocked_stuff[n].m_h);
            m_unlocked_stuff[n].m_root_gift_node   = irr_driver->addMesh(mesh);

        }
        else
        {
            std::cerr << "Malformed unlocked goody!!!\n";
        }
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
    
    m_unlocked_stuff.clearAndDeleteAll();
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

    const int unlockedStuffCount = m_unlocked_stuff.size();

    if (m_global_time > GIFT_EXIT_FROM && m_global_time < GIFT_EXIT_TO)
    {
                
        for (int n=0; n<unlockedStuffCount; n++)
        {
            if (m_unlocked_stuff[n].m_root_gift_node == NULL) continue;
            
            core::vector3df pos = m_unlocked_stuff[n].m_root_gift_node->getPosition();
            pos.Y = sin( (float)((m_global_time - GIFT_EXIT_FROM)*M_PI*1.2/GIFT_EXIT_TO)  )*30.0f;
            
            // when there are more than 1 unlocked items, make sure they each have their own path when they move
            if (unlockedStuffCount > 1)
            {
                if (n % 2 == 0) pos.X -= 2.2f*dt*float( int((n + 1)/2) );
                else            pos.X += 2.2f*dt*float( int((n + 1)/2) );
                //std::cout << "Object " << n << " moving by " << (n % 2 == 0 ? -4.0f : 4.0f)*float( n/2 + 1 ) << std::endl;
            }
            else
            {
                pos.X += 2.0f*dt;
            }
            
            if (m_global_time > GIFT_EXIT_FROM + 2.0f) pos.Z += 5.0f*dt;

            m_unlocked_stuff[n].m_root_gift_node->setPosition(pos);
            
            core::vector3df scale = m_unlocked_stuff[n].m_root_gift_node->getScale();
            scale.X += 2.0f*dt;
            scale.Y += 2.0f*dt;
            scale.Z += 2.0f*dt;
            m_unlocked_stuff[n].m_root_gift_node->setScale(scale);
        }
        
        core::vector3df campos = m_camera->getPosition();
        campos.X += 2.0f*dt;
        campos.Z += 5.0f*dt;
        
        m_camera->setPosition(campos);
    }
    else if (m_global_time < GIFT_EXIT_FROM)
    {
        m_camera->setPosition( core::vector3df(cos((1.0f-m_key_angle)*M_PI/4 + M_PI/4)*70.0f,
                                               30.0f,
                                               sin((1.0f-m_key_angle)*M_PI/8 + M_PI/4)*70.0f) );
    }

    assert(m_unlocked_stuff.size() > 0);
    if (m_unlocked_stuff[0].m_root_gift_node != NULL)
    {
        m_camera->setTarget( m_unlocked_stuff[0].m_root_gift_node->getPosition() +
                            core::vector3df(0.0f, 10.0f, 0.0f) );
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
    
    GUIEngine::getTitleFont()->draw(_("Feature Unlocked"),
                                    core::rect< s32 >( 0, 0, w, h/10 ),
                                    color,
                                    true/* center h */, true /* center v */ );
    
    if (m_global_time > GIFT_EXIT_TO)
    {
        const irr::video::SColor color2(255, 255, 126, 21);
        const int fontH = GUIEngine::getFontHeight();
        const int MARGIN = 10;
        
        int message_y = h - fontH*3 - MARGIN;
        
        for (int n=0; n<unlockedStuffCount; n++)
        {
            GUIEngine::getFont()->draw(m_unlocked_stuff[n].m_unlock_message,
                                       core::rect< s32 >( 0, message_y, w, message_y + fontH ),
                                       color2,
                                       true /* center h */, true /* center v */ );
            message_y -= (fontH + MARGIN);
        }
    }
}

// -------------------------------------------------------------------------------------

bool FeatureUnlockedCutScene::onEscapePressed()
{
    continueButtonPressed();
    return false; // continueButtonPressed already pop'ed the menu
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::continueButtonPressed()
{
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        // in GP mode, continue GP after viewing this screen (TODO: test)
        StateManager::get()->popMenu();
        race_manager->next();
    }
    else
    {
        // back to menu
        race_manager->exitRace();
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
    }
    
}

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    
    if (name == "continue")
    {
        continueButtonPressed();
    }
    
}

// -------------------------------------------------------------------------------------
