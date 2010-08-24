
#include "states_screens/feature_unlocked.hpp"

#include <SColor.h>

#include "challenges/challenge.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/world.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;


DEFINE_SCREEN_SINGLETON( FeatureUnlockedCutScene );

// ============================================================================

#if 0
#pragma mark FeatureUnlockedCutScene::UnlockedThing
#endif

FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(KartProperties* kart, 
                                                      irr::core::stringw msg)
{
    m_unlocked_kart      = kart;
    m_unlock_message     = msg;
    m_curr_image         = -1;
}   // UnlockedThing::UnlockedThing

// -------------------------------------------------------------------------------------

FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(irr::video::ITexture* pict,
                                                      float w, float h, 
                                                      irr::core::stringw msg)
{
    m_unlocked_kart = NULL;
    m_pictures.push_back(pict);
    m_w = w;
    m_h = h;
    m_unlock_message = msg;
    m_curr_image = -1;
}   // UnlockedThing::UnlockedThing

// ----------------------------------------------------------------------------

FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(std::vector<irr::video::ITexture*> picts,
                                                      float w, float h, 
                                                      irr::core::stringw msg)
{
    m_unlocked_kart = NULL;
    m_pictures = picts;
    m_w = w;
    m_h = h;
    m_unlock_message = msg;
    m_curr_image = 0;
}   // UnlockedThing::UnlockedThing

// ----------------------------------------------------------------------------

FeatureUnlockedCutScene::UnlockedThing::~UnlockedThing()
{
    if (m_root_gift_node != NULL) irr_driver->removeNode(m_root_gift_node);
    m_root_gift_node = NULL;
}   // UnlockedThing::~UnlockedThing

// ============================================================================

#if 0
#pragma mark -
#pragma mark FeatureUnlockedCutScene
#endif

FeatureUnlockedCutScene::FeatureUnlockedCutScene() 
                       : Screen("feature_unlocked.stkgui")
{
    m_key_angle = 0;
    setNeeds3D(true);
    
    m_throttle_FPS = false;
#ifdef USE_IRRLICHT_BUG_WORKAROUND
    m_avoid_irrlicht_bug = NULL;
#endif
}  // FeatureUnlockedCutScene

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::loadedFromFile()
{
}   // loadedFromFile

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedKart(KartProperties* unlocked_kart, 
                                              irr::core::stringw msg)
{
    assert(unlocked_kart != NULL);
    m_unlocked_stuff.push_back( new UnlockedThing(unlocked_kart, msg) );
}  // addUnlockedKart

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedPicture(irr::video::ITexture* picture,
                                                 float w, float h, 
                                                 irr::core::stringw msg)
{
    assert(picture != NULL);

    m_unlocked_stuff.push_back( new UnlockedThing(picture, w, h, msg) );
}   // addUnlockedPicture

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedPictures(std::vector<irr::video::ITexture*> pictures,
                                                  float w, float h, 
                                                  irr::core::stringw msg)
{
    assert(!pictures.empty());
    
    m_unlocked_stuff.push_back( new UnlockedThing(pictures, w, h, msg) );
}   // addUnlockedPictures

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::init()
{
    m_sky_angle = 0.0f;
    m_global_time = 0.0f;
    
    std::vector<std::string> texture_names(6);
    texture_names[0] = file_manager->getTextureFile("purplenebula.png");
    texture_names[1] = file_manager->getTextureFile("purplenebula2.png");
    texture_names[2] = file_manager->getTextureFile("purplenebula.png");
    texture_names[3] = file_manager->getTextureFile("purplenebula2.png");
    texture_names[4] = file_manager->getTextureFile("purplenebula.png");
    texture_names[5] = file_manager->getTextureFile("purplenebula2.png");
    m_sky = irr_driver->addSkyBox(texture_names);
#ifdef DEBUG
    m_sky->setName("skybox");
#endif
    
    m_camera = irr_driver->addCameraSceneNode();
#ifdef DEBUG
    m_camera->setName("camera");
#endif
    m_camera->setPosition( core::vector3df(0.0, 30.0f, 70.0f) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    m_camera->setTarget( core::vector3df(0, 10, 0.0f) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    irr_driver->getSceneManager()->setActiveCamera(m_camera);
    
    scene::IAnimatedMesh* model_chest = 
        irr_driver->getAnimatedMesh( file_manager->getModelFile("chest.b3d") );
    assert(model_chest != NULL);
    m_chest = irr_driver->addAnimatedMesh(model_chest);
#ifdef DEBUG
    m_chest->setName("chest");
#endif
    m_chest->setPosition( core::vector3df(2, -3, 0) );
    m_chest->setScale( core::vector3df(10.0f, 10.0f, 10.0f) );

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 120, 
                                                                 120, 120));
    
    const core::vector3df &sun_pos = core::vector3df( 0, 200, 100.0f );
    m_light = 
        irr_driver->getSceneManager()->addLightSceneNode(NULL, sun_pos, 
                                                         video::SColorf(1.0f,1.0f,1.0f),
                                                         10000.0f /* radius */);
#ifdef DEBUG
    m_light->setName("light");
#endif
    m_light->getLightData().DiffuseColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    
    const int unlockedStuffCount = m_unlocked_stuff.size();
    
    if (unlockedStuffCount == 0)  std::cerr << "There is nothing in the unlock chest!!!\n";
    
    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_unlocked_kart != NULL)
        {
            KartModel* kart_model = 
                m_unlocked_stuff[n].m_unlocked_kart->getKartModel();
            kart_model->attachModel(&(m_unlocked_stuff[n].m_root_gift_node));
#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked kart");
#endif
#ifdef USE_IRRLICHT_BUG_WORKAROUND
            // If a mesh with this material is added, irrlicht will
            // display the 'continue' text (otherwise the text is
            // not visible). This is a terrible work around, but
            // stk to be released without waiting for the next
            // irrlicht version.
            video::SMaterial m;
            m.MaterialType    = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            scene::IMesh* mesh = 
                irr_driver->createTexturedQuadMesh(&m, 0, 0);
            m_avoid_irrlicht_bug = irr_driver->addMesh(mesh);
#endif
        }
        else if (!m_unlocked_stuff[n].m_pictures.empty())
        {
            video::SMaterial m;
            m.MaterialType    = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            m.BackfaceCulling = false;
            m.setTexture(0, m_unlocked_stuff[n].m_pictures[0]);
            m.AmbientColor  = video::SColor(255, 255, 255, 255);
            m.DiffuseColor  = video::SColor(255, 255, 255, 255);
            m.EmissiveColor = video::SColor(255, 255, 255, 255);
            m.SpecularColor = video::SColor(255, 255, 255, 255);
            m.GouraudShading = false;
            m.Shininess      = 0;
            //m.setFlag(video::EMF_TEXTURE_WRAP, false);
            
            m.TextureLayer[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
            m.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
            scene::IMesh* mesh = 
                irr_driver->createTexturedQuadMesh(&m,
                                                   m_unlocked_stuff[n].m_w,
                                                   m_unlocked_stuff[n].m_h);
            m_unlocked_stuff[n].m_root_gift_node   = irr_driver->addMesh(mesh);
#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked track picture");
#endif
        }
        else
        {
            std::cerr << "Malformed unlocked goody!!!\n";
        }
    }
}   // init

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::tearDown()
{
    Screen::tearDown();
    irr_driver->removeNode(m_sky);
    m_sky = NULL;
    
    irr_driver->removeCameraSceneNode(m_camera);
    m_camera = NULL;
    
    irr_driver->removeNode(m_chest);
    m_chest = NULL;
    
    irr_driver->removeNode(m_light);
    m_light = NULL;

#ifdef USE_IRRLICHT_BUG_WORKAROUND
    if(m_avoid_irrlicht_bug)
        irr_driver->removeNode(m_avoid_irrlicht_bug);
    m_avoid_irrlicht_bug = NULL;
#endif

    m_unlocked_stuff.clearAndDeleteAll();
}   // tearDown

// ----------------------------------------------------------------------------

//FIXME: doesn't go here...
template<typename T>
T keepInRange(T from, T to, T value)
{
    if (value < from) return from;
    if (value > to  ) return to;
    return value;
}   // keepInRange

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::onUpdate(float dt, 
                                       irr::video::IVideoDriver* driver)
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
        const float rotationProgression = 
            keepInRange( 0.0f, 1.0f, (float)sin(M_PI/2.0f*m_global_time/ANIM_FROM) );
        const float chest_rotation = 
            keepInRange(80.0f, 160.0f, 80 + rotationProgression * 80);
        m_chest->setRotation( core::vector3df(0.0f, chest_rotation, 0.0f) );
    }
    
    const float current_frame = keepInRange(0.0f, (float)last_image,
                                            (float)(m_global_time - ANIM_FROM)
                                           /(ANIM_TO - ANIM_FROM) * last_image);
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
            pos.Y = sin( (float)((m_global_time-GIFT_EXIT_FROM)*M_PI*1.2/GIFT_EXIT_TO) )*30.0f;
            
            // when there are more than 1 unlocked items, make sure they each 
            // have their own path when they move
            if (unlockedStuffCount > 1)
            {
                if (n % 2 == 0) pos.X -= 2.2f*dt*float( int((n + 1)/2) );
                else            pos.X += 2.2f*dt*float( int((n + 1)/2) );
                //std::cout << "Object " << n << " moving by " << 
                //  (n % 2 == 0 ? -4.0f : 4.0f)*float( n/2 + 1 ) << std::endl;
            }
            else
            {
                pos.X += 2.0f*dt;
            }
            
            if (m_global_time > GIFT_EXIT_FROM + 2.0f) pos.Z += 5.0f*dt;

            m_unlocked_stuff[n].m_root_gift_node->setPosition(pos);
            
            core::vector3df scale = 
                m_unlocked_stuff[n].m_root_gift_node->getScale();
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
    
    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_root_gift_node == NULL) continue;
        
        if (!m_unlocked_stuff[n].m_pictures.empty())
        {
            const int pictureCount = m_unlocked_stuff[n].m_pictures.size();
            
            if (pictureCount > 1)
            {
                const int previousTextureID = m_unlocked_stuff[n].m_curr_image;
                const int textureID = int(m_global_time/1.2f) % pictureCount;
                
                if (textureID != previousTextureID)
                {
                    scene::IMeshSceneNode* node = (scene::IMeshSceneNode*)m_unlocked_stuff[n].m_root_gift_node;
                    scene::IMesh* mesh = node->getMesh();
                    
                    assert(mesh->getMeshBufferCount() == 1);
                    
                    scene::IMeshBuffer* mb = mesh->getMeshBuffer(0);
                    
                    SMaterial& m = mb->getMaterial();
                    m.setTexture(0, m_unlocked_stuff[n].m_pictures[textureID]);
                    
                    // FIXME: this mesh is already associated with this node. I'm calling this
                    // to force irrLicht to refresh the display, now that Material has changed.
                    node->setMesh(mesh);
                    
                    m_unlocked_stuff[n].m_curr_image = textureID;
                }   // textureID != previousTextureID
            }   // if pictureCount>1
        }   // if !m_unlocked_stuff[n].m_pictures.empty()
    }   // for n<unlockedStuffCount
    
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
    printf("camera: %f %f %f\n",
        m_camera->getPosition().X,
        m_camera->getPosition().Y,
        m_camera->getPosition().Z);
}   // onUpdate

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedThings(const std::vector<const Challenge*> unlocked)
{
    for (unsigned int n=0; n<unlocked.size(); n++)
    {
        const std::vector<UnlockableFeature>& unlockedFeatures = unlocked[n]->getFeatures();
        assert(unlockedFeatures.size() > 0);
        
        for (unsigned int i=0; i<unlockedFeatures.size(); i++)
        {
            
            switch (unlockedFeatures[i].type)
            {
                case UNLOCK_TRACK:
                {
                    Track* track = track_manager->getTrack(unlockedFeatures[i].name);
                    assert(track != NULL);
                    const std::string sshot = track->getScreenshotFile();
                    addUnlockedPicture( irr_driver->getTexture(sshot.c_str()), 1.0f, 0.75f,
                                         unlockedFeatures[i].getUnlockedMessage() );
                    break;
                }
                case UNLOCK_GP:
                {
                    std::vector<ITexture*> images;
                    const GrandPrixData* gp = grand_prix_manager->getGrandPrix(unlockedFeatures[i].name);
                    if (gp == NULL)
                    {
                        std::cerr << "ERROR: Unlocked GP does not exist???\n";
                        video::ITexture* WTF_image = irr_driver->getTexture( file_manager->getGUIDir() + "/main_help.png");
                        images.push_back(WTF_image);
                    }
                    else
                    {
                        const std::vector<std::string>& gptracks = gp->getTracks();
                        const int trackAmount = gptracks.size();
                        
                        if (trackAmount == 0)
                        {
                            std::cerr << "ERROR: Unlocked GP is empty???\n";
                            video::ITexture* WTF_image = irr_driver->getTexture( file_manager->getGUIDir() + "/main_help.png");
                            images.push_back(WTF_image);
                        }
                        
                        for (int t=0; t<trackAmount; t++)
                        {
                            Track* track = track_manager->getTrack(gptracks[t]);
                            
                            ITexture* tex = irr_driver->getTexture(track  != NULL ?
                                                                   track->getScreenshotFile().c_str() :
                                                                   file_manager->getDataDir() + "gui/main_help.png");
                            images.push_back(tex);
                        }
                    }

                    addUnlockedPictures(images, 1.0f, 0.75f,
                                        unlockedFeatures[i].getUnlockedMessage() );
                    break;
                }
                case UNLOCK_MODE:
                {
                    const RaceManager::MinorRaceModeType mode =
                    RaceManager::getModeIDFromInternalName(unlockedFeatures[i].name.c_str());
                    const std::string icon = file_manager->getDataDir() + "/" + RaceManager::getIconOf(mode);
                    addUnlockedPicture( irr_driver->getTexture(icon.c_str()), 0.8f, 0.8f,
                                        unlockedFeatures[i].getUnlockedMessage() );
                    break;
                }
                case UNLOCK_KART:
                {
                    const KartProperties* kart = kart_properties_manager->getKart(unlockedFeatures[i].name);
                    assert(kart != NULL);
                    
                    // the passed kart will not be modified, that's why I allow myself to use const_cast
                    addUnlockedKart( const_cast<KartProperties*>(kart),
                                     unlockedFeatures[i].getUnlockedMessage() );
                    break;
                }
                case UNLOCK_DIFFICULTY:
                {
                    //TODO : implement difficulty reward
                    std::cerr << "OK, I see you unlocked a difficulty, but this is not supported yet\n";

                    video::ITexture* tex = irr_driver->getTexture( file_manager->getGUIDir() + "/main_help.png");
                    addUnlockedPicture( tex, 1.0f, 0.75f,
                                        unlockedFeatures[i].getUnlockedMessage() );
                    break;
                }
                default:
                {
                    assert(false);
                }
            }
            
        } // next feature
    } // next challenge
}   // addUnlockedThings

// ----------------------------------------------------------------------------

bool FeatureUnlockedCutScene::onEscapePressed()
{
    continueButtonPressed();
    return false; // continueButtonPressed already pop'ed the menu
}   // onEscapePressed

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::continueButtonPressed()
{
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        // in GP mode, continue GP after viewing this screen
        StateManager::get()->popMenu();
        race_manager->next();
    }
    else
    {
        // back to menu
        race_manager->exitRace();
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
    }
    
}   // continueButtonPressed

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    if (name == "continue")
    {
        continueButtonPressed();
    }   
}   // eventCallback

// -------------------------------------------------------------------------------------
