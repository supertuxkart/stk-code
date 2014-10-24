//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "states_screens/feature_unlocked.hpp"

#include <SColor.h>

#include "audio/music_manager.hpp"
#include "challenges/challenge_data.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/overworld.hpp"
#include "modes/world.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

#include <IAnimatedMeshSceneNode.h>
#include <ICameraSceneNode.h>
#include <ILightSceneNode.h>
#include <IMeshSceneNode.h>
#include <ISceneManager.h>

#include <iostream>

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

const float ANIM_FROM = 3.0f;
const float ANIM_TO = 7.0f;
const int GIFT_EXIT_FROM = (int)ANIM_TO;
const int GIFT_EXIT_TO = GIFT_EXIT_FROM + 7;

DEFINE_SCREEN_SINGLETON( FeatureUnlockedCutScene );

// ============================================================================

#if 0
#pragma mark FeatureUnlockedCutScene::UnlockedThing
#endif

FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(std::string model,
                                                      irr::core::stringw msg)
{
    m_unlocked_kart      = NULL;
    m_unlock_message     = msg;
    m_unlock_model       = model;
    m_curr_image         = -1;
    m_scale              = 1.0f;
}

// -------------------------------------------------------------------------------------


FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(KartProperties* kart,
                                                      irr::core::stringw msg)
{
    m_unlocked_kart      = kart;
    m_unlock_message     = msg;
    m_curr_image         = -1;
    m_scale              = 1.0f;
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
    m_scale = 1.0f;
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
    m_scale = 1.0f;
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
            : CutsceneScreen("feature_unlocked.stkgui")
{
    m_key_angle = 0;

#ifdef USE_IRRLICHT_BUG_WORKAROUND
    m_avoid_irrlicht_bug = NULL;
#endif
}  // FeatureUnlockedCutScene

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::loadedFromFile()
{
}   // loadedFromFile

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::onCutsceneEnd()
{
#ifdef USE_IRRLICHT_BUG_WORKAROUND
    if (m_avoid_irrlicht_bug)
        irr_driver->removeNode(m_avoid_irrlicht_bug);
    m_avoid_irrlicht_bug = NULL;
#endif
    
    m_unlocked_stuff.clearAndDeleteAll();
    m_all_kart_models.clearAndDeleteAll();

    // update point count and the list of locked/unlocked stuff
    PlayerManager::getCurrentPlayer()->computeActive();
}

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::findWhatWasUnlocked(RaceManager::Difficulty difficulty)
{
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    int points_before = player->getPoints();
    int points_now = points_before + CHALLENGE_POINTS[difficulty];

    std::vector<std::string> tracks;
    std::vector<std::string> gps;

    player->computeActive();
    unlock_manager->findWhatWasUnlocked(points_before, points_now, tracks, gps);

    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        addUnlockedTrack(track_manager->getTrack(tracks[i]));
    }
    for (unsigned int i = 0; i < gps.size(); i++)
    {
        addUnlockedGP(grand_prix_manager->getGrandPrix(gps[i]));
    }
}

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addTrophy(RaceManager::Difficulty difficulty)
{
    core::stringw msg;
    switch (difficulty)
    {
        case RaceManager::DIFFICULTY_EASY:
            msg = _("You completed the easy challenge! Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_EASY], CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD]);
            break;
        case RaceManager::DIFFICULTY_MEDIUM:
            msg = _("You completed the intermediate challenge! Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_MEDIUM], CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD]);
            break;
        case RaceManager::DIFFICULTY_HARD:
            msg = _("You completed the difficult challenge! Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD], CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD]);
            break;
        default:
            assert(false);
    }

    std::string model;
    switch (difficulty)
    {
        case RaceManager::DIFFICULTY_EASY:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_bronze.b3d");
            break;
        case RaceManager::DIFFICULTY_MEDIUM:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_silver.b3d");
            break;
        case RaceManager::DIFFICULTY_HARD:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_gold.b3d");
            break;
        default:
            assert(false);
            return;
    }


    m_unlocked_stuff.push_back( new UnlockedThing(model, msg) );
}

// ----------------------------------------------------------------------------
// unused for now, maybe will be useful later?

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
    if (picture == NULL)
    {
        Log::warn("FeatureUnlockedCutScene::addUnlockedPicture", "Unlockable has no picture: %s",
            core::stringc(msg.c_str()).c_str());
        picture = irr_driver->getTexture(file_manager->getAsset(FileManager::GUI,"main_help.png"));

    }

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
    m_global_time = 0.0f;

    const int unlockedStuffCount = m_unlocked_stuff.size();

    if (unlockedStuffCount == 0)
        Log::error("FeatureUnlockedCutScene::init", "There is nothing in the unlock chest");

    m_all_kart_models.clearAndDeleteAll();
    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_unlock_model.size() > 0)
        {
            m_unlocked_stuff[n].m_root_gift_node = irr_driver->addMesh(
                irr_driver->getMesh(m_unlocked_stuff[n].m_unlock_model), "unlocked_model");
            m_unlocked_stuff[n].m_scale = 0.7f;
            //m_unlocked_stuff[n].m_root_gift_node->setScale(core::vector3df(0.2f, 0.2f, 0.2f));
        }
        else if (m_unlocked_stuff[n].m_unlocked_kart != NULL)
        {
            KartModel *kart_model =
                m_unlocked_stuff[n].m_unlocked_kart->getKartModelCopy();
            m_all_kart_models.push_back(kart_model);
            m_unlocked_stuff[n].m_root_gift_node = kart_model->attachModel(true, false);
            m_unlocked_stuff[n].m_scale = 5.0f;
            kart_model->setAnimation(KartModel::AF_DEFAULT);
            kart_model->update(0.0f, 0.0f, 0.0f, 0.0f);

#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked kart");
#endif
#ifdef USE_IRRLICHT_BUG_WORKAROUND
            // If a mesh with this material is added, irrlicht will
            // display the 'continue' text (otherwise the text is
            // not visible). This is a terrible work around, but allows
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
            //m.MaterialType    = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            m.BackfaceCulling = false;
            m.setTexture(0, m_unlocked_stuff[n].m_pictures[0]);
            m.AmbientColor  = video::SColor(255, 255, 255, 255);
            m.DiffuseColor  = video::SColor(255, 255, 255, 255);
            m.EmissiveColor = video::SColor(255, 255, 255, 255);
            m.SpecularColor = video::SColor(255, 255, 255, 255);
            m.GouraudShading = false;
            m.Shininess      = 0;

            m.TextureLayer[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
            m.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
            scene::IMesh* mesh =
                irr_driver->createTexturedQuadMesh(&m,
                                                   m_unlocked_stuff[n].m_w,
                                                   m_unlocked_stuff[n].m_h);
            m_unlocked_stuff[n].m_root_gift_node = irr_driver->getSceneManager()->addEmptySceneNode();
            m_unlocked_stuff[n].m_side_1 = irr_driver->addMesh(mesh, "unlocked_picture", m_unlocked_stuff[n].m_root_gift_node);
            //mesh->drop();

            mesh = irr_driver->createTexturedQuadMesh(&m,
                m_unlocked_stuff[n].m_w,
                m_unlocked_stuff[n].m_h);
            m_unlocked_stuff[n].m_side_2 = irr_driver->addMesh(mesh, "unlocked_picture",  m_unlocked_stuff[n].m_root_gift_node);
            m_unlocked_stuff[n].m_side_2->setRotation(core::vector3df(0.0f, 180.0f, 0.0f));
            //mesh->drop();
#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked track picture");
#endif
        }
        else
        {
            Log::error("FeatureUnlockedCutScene::init", "Malformed unlocked goody");
        }
    }

    PlayerManager::getCurrentPlayer()->clearUnlocked();
}   // init

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::tearDown()
{
    Screen::tearDown();
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

void FeatureUnlockedCutScene::onUpdate(float dt)
{
    m_global_time += dt;
    const int unlockedStuffCount = m_unlocked_stuff.size();

    if (m_global_time > GIFT_EXIT_FROM && m_global_time < GIFT_EXIT_TO)
    {
        float progress_factor = (m_global_time - GIFT_EXIT_FROM) / (GIFT_EXIT_TO - GIFT_EXIT_FROM);
        float smoothed_progress_factor = sin((progress_factor - 0.5f)*M_PI)/2.0f + 0.5f;

        for (int n=0; n<unlockedStuffCount; n++)
        {
            if (m_unlocked_stuff[n].m_root_gift_node == NULL) continue;

            core::vector3df pos = m_unlocked_stuff[n].m_root_gift_node->getPosition();
            pos.Y = sin(smoothed_progress_factor*2.5f)*10.0f;

            // when there are more than 1 unlocked items, make sure they each
            // have their own path when they move
            if (unlockedStuffCount > 1)
            {
                if (n == 1) pos.X -= 1.0f*dt*float( int((n + 1)/2) );
                else if (n > 1) pos.X += 1.0f*dt*(n - 0.3f);

                //else            pos.X += 6.2f*dt*float( int((n + 1)/2) );
                //Log::info("FeatureUnlockedCutScene", "Object %d moving by %f", n,
                //    (n % 2 == 0 ? -4.0f : 4.0f)*float( n/2 + 1 ));
            }
            else
            {
                //pos.X -= 2.0f*dt;
            }

            //if (m_global_time > GIFT_EXIT_FROM + 2.0f) pos.Z -= 2.0f*dt;

            pos.Z = smoothed_progress_factor * -4.0f;

            m_unlocked_stuff[n].m_root_gift_node->setPosition(pos);
        }
    }
    else if (m_global_time < GIFT_EXIT_FROM)
    {
    }

    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_root_gift_node == NULL) continue;

        m_unlocked_stuff[n].m_root_gift_node->setRotation(m_unlocked_stuff[n].m_root_gift_node->getRotation() + core::vector3df(0.0f, dt*25.0f, 0.0f));

        if (!m_unlocked_stuff[n].m_pictures.empty())
        {
            const int picture_count = (int)m_unlocked_stuff[n].m_pictures.size();

            if (picture_count > 1)
            {
                const int previousTextureID = m_unlocked_stuff[n].m_curr_image;
                const int textureID = int(m_global_time/1.2f) % picture_count;

                if (textureID != previousTextureID)
                {
                    scene::IMesh* mesh = m_unlocked_stuff[n].m_side_1->getMesh();

                    assert(mesh->getMeshBufferCount() == 1);

                    scene::IMeshBuffer* mb = mesh->getMeshBuffer(0);

                    SMaterial& m = mb->getMaterial();
                    m.setTexture(0, m_unlocked_stuff[n].m_pictures[textureID]);

                    // FIXME: this mesh is already associated with this node. I'm calling this
                    // to force irrLicht to refresh the display, now that Material has changed.
                    m_unlocked_stuff[n].m_side_1->setMesh(mesh);

                    m_unlocked_stuff[n].m_curr_image = textureID;


                    mesh = m_unlocked_stuff[n].m_side_2->getMesh();
                    assert(mesh->getMeshBufferCount() == 1);
                    mb = mesh->getMeshBuffer(0);

                    SMaterial& m2 = mb->getMaterial();
                    m2.setTexture(0, m_unlocked_stuff[n].m_pictures[textureID]);

                    // FIXME: this mesh is already associated with this node. I'm calling this
                    // to force irrLicht to refresh the display, now that Material has changed.
                    m_unlocked_stuff[n].m_side_2->setMesh(mesh);

                    m_unlocked_stuff[n].m_curr_image = textureID;
                }   // textureID != previousTextureID
            }   // if picture_count>1
        }   // if !m_unlocked_stuff[n].m_pictures.empty()

        float scale = m_unlocked_stuff[n].m_scale;
        if (m_global_time <= GIFT_EXIT_FROM)
            scale *= 0.1f;
        else if (m_global_time > GIFT_EXIT_FROM && m_global_time < GIFT_EXIT_TO)
            scale *= ((m_global_time - GIFT_EXIT_FROM) / (GIFT_EXIT_TO - GIFT_EXIT_FROM));
        m_unlocked_stuff[n].m_root_gift_node->setScale(core::vector3df(scale, scale, scale));
    }   // for n<unlockedStuffCount

    assert(m_unlocked_stuff.size() > 0);

    static const int w = irr_driver->getFrameSize().Width;
    static const int h = irr_driver->getFrameSize().Height;
    const irr::video::SColor color(255, 255, 255, 255);

    GUIEngine::getTitleFont()->draw(_("Challenge Completed"),
                                    core::rect< s32 >( 0, 0, w, h/10 ),
                                    color,
                                    true/* center h */, true /* center v */ );

    if (m_global_time > GIFT_EXIT_TO)
    {
        const irr::video::SColor color2(255, 0, 0, 0);
        const int fontH = GUIEngine::getFontHeight();
        const int MARGIN = 10;

        int message_y = h - fontH*3 - MARGIN;

        for (int n=unlockedStuffCount - 1; n>=0; n--)
        {
            GUIEngine::getFont()->draw(m_unlocked_stuff[n].m_unlock_message,
                                       core::rect< s32 >( 0, message_y, w, message_y + fontH ),
                                       color2,
                                       true /* center h */, true /* center v */ );
            message_y -= (fontH + MARGIN);
        }
    }
}   // onUpdate

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedTrack(const Track* track)
{
    if (track == NULL)
    {
        Log::error("FeatureUnlockedCutScene::addUnlockedTrack", "Unlocked track does not exist");
        return;
    }

    const std::string sshot = track->getScreenshotFile();
    core::stringw trackname = track->getName();
    addUnlockedPicture( irr_driver->getTexture(sshot.c_str()), 4.0f, 3.0f,
        _("You unlocked track %0", trackname));
}

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedGP(const GrandPrixData* gp)
{
    std::vector<ITexture*> images;
    if (gp == NULL)
    {
        Log::error("FeatureUnlockedCutScene::addUnlockedGP", "Unlocked GP does not exist");
        video::ITexture* WTF_image = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI,"main_help.png"));
        images.push_back(WTF_image);
    }
    else
    {
        const std::vector<std::string> gptracks = gp->getTrackNames();
        const int track_amount = (int)gptracks.size();

        if (track_amount == 0)
        {
            Log::error("FeatureUnlockedCutScene::addUnlockedGP", "Unlocked GP is empty");
            video::ITexture* WTF_image = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI,"main_help.png"));
            images.push_back(WTF_image);
        }

        for (int t=0; t<track_amount; t++)
        {
            Track* track = track_manager->getTrack(gptracks[t]);

            ITexture* tex = irr_driver->getTexture(track  ?  track->getScreenshotFile().c_str()
                                                          : file_manager->getAsset(FileManager::GUI,"main_help.png"));
            images.push_back(tex);
        }
    }

    core::stringw gpname = gp->getName();
    addUnlockedPictures(images, 4.0f, 3.0f, _("You unlocked grand prix %0", gpname));
}

// ----------------------------------------------------------------------------

bool FeatureUnlockedCutScene::onEscapePressed()
{
    continueButtonPressed();
    return false; // continueButtonPressed already pop'ed the menu
}   // onEscapePressed

// -------------------------------------------------------------------------------------

void FeatureUnlockedCutScene::continueButtonPressed()
{
    //if (m_global_time < GIFT_EXIT_TO)
    //{
    //    // If animation was not over yet, the button is used to skip the animation
    //    while (m_global_time < GIFT_EXIT_TO)
    //    {
    //        // simulate all the steps of the animation until we reach the end
    //        onUpdate(0.4f);
    //        World::getWorld()->updateWorld(0.4f);
    //    }
    //}
    //else
    //{
        ((CutsceneWorld*)World::getWorld())->abortCutscene();
    //}

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

MusicInformation* FeatureUnlockedCutScene::getInGameMenuMusic() const
{
    MusicInformation* mi = music_manager->getMusicInformation("win_theme.music");
    return mi;
}
