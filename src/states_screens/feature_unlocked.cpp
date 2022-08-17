//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_model.hpp"
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
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IAnimatedMeshSceneNode.h>
#include <ICameraSceneNode.h>
#include <ILightSceneNode.h>
#include <IMeshSceneNode.h>
#include <ISceneManager.h>

#include <iostream>

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#endif

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

const float ANIM_TO = 7.0f;
const int GIFT_EXIT_FROM = (int)ANIM_TO;
const int GIFT_EXIT_TO = GIFT_EXIT_FROM + 7;

// ============================================================================

#if 0
#pragma mark FeatureUnlockedCutScene::UnlockedThing
#endif

FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(const std::string &model,
                                                      const irr::core::stringw &msg)
{
    m_unlocked_kart      = NULL;
    m_unlock_message     = msg;
    m_unlock_model       = model;
    m_curr_image         = -1;
    m_scale              = 1.0f;
}

// ----------------------------------------------------------------------------


FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(const KartProperties* kart,
                                                      const irr::core::stringw &msg)
{
    m_unlocked_kart      = kart;
    m_unlock_message     = msg;
    m_curr_image         = -1;
    m_scale              = 1.0f;
}   // UnlockedThing::UnlockedThing

// ----------------------------------------------------------------------------

FeatureUnlockedCutScene::UnlockedThing::UnlockedThing(irr::video::ITexture* pict,
                                                      float w, float h,
                                                      const irr::core::stringw &msg)
{
    m_unlocked_kart = NULL;
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        m_sp_pictures.push_back(SP::SPTextureManager::get()
            ->getTexture(pict->getName().getPtr(), NULL, true,
            ""/*container_id*/));
    }
    else
    {
        m_pictures.push_back(pict);
    }
#endif
    m_w = w;
    m_h = h;
    m_unlock_message = msg;
    m_curr_image = -1;
    m_scale = 1.0f;
}   // UnlockedThing::UnlockedThing

// ----------------------------------------------------------------------------

FeatureUnlockedCutScene::UnlockedThing
                        ::UnlockedThing(std::vector<irr::video::ITexture*> picts,
                                        float w, float h,
                                        const irr::core::stringw &msg)
{
    m_unlocked_kart = NULL;
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        for (auto* pict : picts)
        {
            m_sp_pictures.push_back(SP::SPTextureManager::get()
                ->getTexture(pict->getName().getPtr(), NULL, true,
                ""/*container_id*/));
        }
    }
    else
#endif
    {
        m_pictures = picts;
    }
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
            : CutsceneScreen("cutscene.stkgui")
{
    m_key_angle = 0;
}  // FeatureUnlockedCutScene

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::loadedFromFile()
{
}   // loadedFromFile

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::onCutsceneEnd()
{
    m_unlocked_stuff.clearAndDeleteAll();
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        SP::SPTextureManager::get()->removeUnusedTextures();
    }
#endif
    m_all_kart_models.clearAndDeleteAll();

    // update point count and the list of locked/unlocked stuff
    PlayerManager::getCurrentPlayer()->computeActive();
}

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::
               findWhatWasUnlocked(RaceManager::Difficulty difficulty,
                                   std::vector<const ChallengeData*>& unlocked)
{
    if (UserConfigParams::m_unlock_everything > 0)
        return;

    PlayerProfile *player = PlayerManager::getCurrentPlayer();

    // The number of points is updated before this function is called
    int points_before = player->getPointsBefore();
    int points_now = player->getPoints();

    std::vector<std::string> tracks;
    std::vector<std::string> gps;
    std::vector<std::string> karts;

    player->computeActive();
    unlock_manager->findWhatWasUnlocked(points_before, points_now, tracks, gps,
                                        karts, unlocked);

    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        addUnlockedTrack(track_manager->getTrack(tracks[i]));
    }
    for (unsigned int i = 0; i < gps.size(); i++)
    {
        addUnlockedGP(grand_prix_manager->getGrandPrix(gps[i]));
    }
    for (unsigned int i = 0; i < karts.size(); i++)
    {
        addUnlockedKart(kart_properties_manager->getKart(karts[i]));
    }
}

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addTrophy(RaceManager::Difficulty difficulty,
                                        bool is_grandprix)
{
    core::stringw msg;

    int gp_factor = is_grandprix ? GP_FACTOR : 1;
    RaceManager::Difficulty max_unlocked_difficulty = RaceManager::DIFFICULTY_BEST;

    if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
        max_unlocked_difficulty = RaceManager::DIFFICULTY_HARD;

    switch (difficulty)
    {
        case RaceManager::DIFFICULTY_EASY:
            msg = _("You completed the easy challenge! "
                    "Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_EASY]*gp_factor, 
                    CHALLENGE_POINTS[max_unlocked_difficulty]*gp_factor);
            break;
        case RaceManager::DIFFICULTY_MEDIUM:
            msg = _("You completed the intermediate challenge! "
                    "Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_MEDIUM]*gp_factor,
                    CHALLENGE_POINTS[max_unlocked_difficulty]*gp_factor);
            break;
        case RaceManager::DIFFICULTY_HARD:
            msg = _("You completed the difficult challenge! "
                    "Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD]*gp_factor,
                    CHALLENGE_POINTS[max_unlocked_difficulty]*gp_factor);
            break;
        case RaceManager::DIFFICULTY_BEST:
            msg = _("You completed the SuperTux challenge! "
                    "Points earned on this level: %i/%i",
                    CHALLENGE_POINTS[RaceManager::DIFFICULTY_BEST]*gp_factor,
                CHALLENGE_POINTS[max_unlocked_difficulty]*gp_factor);
            break;
        default:
            assert(false);
    }

    std::string model;
    switch (difficulty)
    {
        case RaceManager::DIFFICULTY_EASY:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_bronze.spm");
            break;
        case RaceManager::DIFFICULTY_MEDIUM:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_silver.spm");
            break;
        case RaceManager::DIFFICULTY_HARD:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_gold.spm");
            break;
        case RaceManager::DIFFICULTY_BEST:
            model = file_manager->getAsset(FileManager::MODEL,"trophy_platinum.spm");
            break;
        default:
            assert(false);
            return;
    }


    m_unlocked_stuff.push_back( new UnlockedThing(model, msg) );
}   // addTrophy

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedKart(const KartProperties* unlocked_kart)
{
    if (unlocked_kart == NULL)
    {
        Log::error("FeatureUnlockedCutScene::addUnlockedKart",
                   "Unlocked kart does not exist");
        return;
    }
    irr::core::stringw msg = _("You unlocked %s!", unlocked_kart->getName());
    m_unlocked_stuff.push_back( new UnlockedThing(unlocked_kart, msg) );
}  // addUnlockedKart

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedPicture(irr::video::ITexture* picture,
                                                 float w, float h,
                                                 const irr::core::stringw &msg)
{
    if (picture == NULL)
    {
        Log::warn("FeatureUnlockedCutScene::addUnlockedPicture",
                  "Unlockable has no picture: %s",
                  core::stringc(msg.c_str()).c_str());
        picture = irr_driver->getTexture(
                     file_manager->getAsset(FileManager::GUI_ICON,"main_help.png"));

    }

    m_unlocked_stuff.push_back( new UnlockedThing(picture, w, h, msg) );
}   // addUnlockedPicture

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::addUnlockedPictures
                              (std::vector<irr::video::ITexture*> pictures,
                               float w, float h, irr::core::stringw msg)
{
    assert(!pictures.empty());

    m_unlocked_stuff.push_back( new UnlockedThing(pictures, w, h, msg) );
}   // addUnlockedPictures

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::init()
{
    m_global_time = 0.0f;
    CutsceneWorld::setUseDuration(false);

    const int unlockedStuffCount = m_unlocked_stuff.size();

    if (unlockedStuffCount == 0)
        Log::error("FeatureUnlockedCutScene::init",
                   "There is nothing in the unlock chest");

    m_all_kart_models.clearAndDeleteAll();
    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_unlock_model.size() > 0)
        {
            irr::scene::IMesh *mesh = 
                irr_driver->getMesh(m_unlocked_stuff[n].m_unlock_model);
            m_unlocked_stuff[n].m_root_gift_node = 
                irr_driver->addMesh(mesh, "unlocked_model");
            m_unlocked_stuff[n].m_scale = 0.7f;
        }
        else if (m_unlocked_stuff[n].m_unlocked_kart != NULL)
        {
            KartModel *kart_model =
                m_unlocked_stuff[n].m_unlocked_kart->getKartModelCopy();
            m_all_kart_models.push_back(kart_model);
            m_unlocked_stuff[n].m_root_gift_node =
                kart_model->attachModel(true, false);
            m_unlocked_stuff[n].m_scale = 5.0f;
            kart_model->setAnimation(KartModel::AF_DEFAULT);
            // Set model current frame to "center"
            kart_model->update(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked kart");
#endif
        }
#ifndef SERVER_ONLY
        else if (!m_unlocked_stuff[n].m_sp_pictures.empty())
        {
            scene::IMesh* mesh = irr_driver->createTexturedQuadMesh(NULL,
                m_unlocked_stuff[n].m_w, m_unlocked_stuff[n].m_h);
            m_unlocked_stuff[n].m_root_gift_node =
                irr_driver->getSceneManager()->addEmptySceneNode();
            SP::SPMesh* spm = SP::convertEVTStandard(mesh);
            m_unlocked_stuff[n].m_side_1 = irr_driver->addMesh(spm,
                "unlocked_picture", m_unlocked_stuff[n].m_root_gift_node);
            spm->getSPMeshBuffer(0)->uploadGLMesh();
            spm->getSPMeshBuffer(0)->getSPTextures()[0] =
                m_unlocked_stuff[n].m_sp_pictures[0];
            spm->getSPMeshBuffer(0)->reloadTextureCompare();
            spm->drop();

            mesh = irr_driver->createTexturedQuadMesh(NULL,
                m_unlocked_stuff[n].m_w, m_unlocked_stuff[n].m_h);
            spm = SP::convertEVTStandard(mesh);
            m_unlocked_stuff[n].m_side_2 = irr_driver->addMesh(spm,
                "unlocked_picture",  m_unlocked_stuff[n].m_root_gift_node);
            m_unlocked_stuff[n].m_side_2->setRotation
                (core::vector3df(0.0f, 180.0f, 0.0f));
            spm->getSPMeshBuffer(0)->uploadGLMesh();
            spm->getSPMeshBuffer(0)->getSPTextures()[0] =
                m_unlocked_stuff[n].m_sp_pictures[0];
            spm->getSPMeshBuffer(0)->reloadTextureCompare();
            spm->drop();
#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked track picture");
#endif
        }
#endif
        else if (!m_unlocked_stuff[n].m_pictures.empty())
        {
#ifndef SERVER_ONLY
            bool vk = (GE::getDriver()->getDriverType() == video::EDT_VULKAN);
            if (vk)
                GE::getGEConfig()->m_convert_irrlicht_mesh = true;
#endif

            video::SMaterial m;
            //m.MaterialType    = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
            m.BackfaceCulling = true;
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
            m_unlocked_stuff[n].m_root_gift_node =
                irr_driver->getSceneManager()->addEmptySceneNode();
            irr_driver->setAllMaterialFlags(mesh);
            m_unlocked_stuff[n].m_side_1 =
                irr_driver->addMesh(mesh, "unlocked_picture",
                                    m_unlocked_stuff[n].m_root_gift_node);
            mesh->drop();

            mesh = irr_driver->createTexturedQuadMesh(&m,
                m_unlocked_stuff[n].m_w,
                m_unlocked_stuff[n].m_h);
            irr_driver->setAllMaterialFlags(mesh);
            m_unlocked_stuff[n].m_side_2 =
                irr_driver->addMesh(mesh, "unlocked_picture", 
                                    m_unlocked_stuff[n].m_root_gift_node);
            m_unlocked_stuff[n].m_side_2
                ->setRotation(core::vector3df(0.0f, 180.0f, 0.0f));
            mesh->drop();

#ifndef SERVER_ONLY
            if (vk)
                GE::getGEConfig()->m_convert_irrlicht_mesh = false;
#endif
#ifdef DEBUG
            m_unlocked_stuff[n].m_root_gift_node->setName("unlocked track picture");
#endif
        }
        else
        {
            Log::error("FeatureUnlockedCutScene::init", "Malformed unlocked goody");
        }
    }
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

    // When the chest has opened but the items are not yet at their final position
    if (m_global_time > GIFT_EXIT_FROM && m_global_time < GIFT_EXIT_TO)
    {
        float progress_factor = (m_global_time - GIFT_EXIT_FROM) 
                              / (GIFT_EXIT_TO - GIFT_EXIT_FROM);
        float smoothed_progress_factor = 
                                sinf((progress_factor - 0.5f)*M_PI)/2.0f + 0.5f;

        for (int n=0; n<unlockedStuffCount; n++)
        {
            if (m_unlocked_stuff[n].m_root_gift_node == NULL) continue;

            core::vector3df pos = m_unlocked_stuff[n].m_root_gift_node->getPosition();
            pos.Y = sinf(smoothed_progress_factor*2.5f)*10.0f;

            // when there are more than 1 unlocked items, make sure they each
            // have their own path when they move
            // and that they won't end offscreen in usual situations

            // Put the trophy (item 0 in the unlocked stuff) in center
            // For this, we exchange the position of the trophy with
            // the item in the middle of the unlocked array.
            int pos_value = (n == 0) ? unlockedStuffCount/2 :
                            (n == unlockedStuffCount/2) ? 0 : n;
            float offset = (float) pos_value - unlockedStuffCount/2.0f + 0.5f;
            offset *= (unlockedStuffCount <= 3) ? 1.4f :
                      (unlockedStuffCount <= 5) ? 1.2f : 1.0f;
            pos.X += offset*dt;

            pos.Z = smoothed_progress_factor * -4.0f;

            m_unlocked_stuff[n].m_root_gift_node->setPosition(pos);
        }
    }

    for (int n=0; n<unlockedStuffCount; n++)
    {
        if (m_unlocked_stuff[n].m_root_gift_node == NULL) continue;

        irr::core::vector3df new_rot = m_unlocked_stuff[n].m_root_gift_node
                                                          ->getRotation() 
                                     + core::vector3df(0.0f, dt*25.0f, 0.0f);
        m_unlocked_stuff[n].m_root_gift_node->setRotation(new_rot);

        if (!m_unlocked_stuff[n].m_pictures.empty())
        {
            const int picture_count = (int)m_unlocked_stuff[n].m_pictures.size();

            if (picture_count > 1)
            {
                const int previousTextureID = m_unlocked_stuff[n].m_curr_image;
                const int textureID = int(m_global_time/1.2f) % picture_count;

                if (textureID != previousTextureID)
                {
#ifndef SERVER_ONLY
                    if (CVS->isGLSL())
                    {
                        SP::SPMesh* mesh = static_cast<SP::SPMeshNode*>
                                           (m_unlocked_stuff[n].m_side_1)->getSPM();

                        assert(mesh->getMeshBufferCount() == 1);
                        SP::SPMeshBuffer* mb = mesh->getSPMeshBuffer(0);
                        mb->getSPTextures()[0] =
                            m_unlocked_stuff[n].m_sp_pictures[textureID];
                        mb->reloadTextureCompare();

                        mesh = static_cast<SP::SPMeshNode*>
                                    (m_unlocked_stuff[n].m_side_2)->getSPM();
                        assert(mesh->getMeshBufferCount() == 1);
                        mb = mesh->getSPMeshBuffer(0);
                        mb->getSPTextures()[0] =
                            m_unlocked_stuff[n].m_sp_pictures[textureID];
                        mb->reloadTextureCompare();

                        m_unlocked_stuff[n].m_curr_image = textureID;
                    }
                    else
                    {
                        scene::IMesh* mesh = 
                            static_cast<scene::IMeshSceneNode*>
                                       (m_unlocked_stuff[n].m_side_1)->getMesh();

                        assert(mesh->getMeshBufferCount() == 1);

                        scene::IMeshBuffer* mb = mesh->getMeshBuffer(0);

                        SMaterial& m = mb->getMaterial();
                        m.setTexture(0, m_unlocked_stuff[n].m_pictures[textureID]);

                        // FIXME: this mesh is already associated with this
                        // node. I'm calling this to force irrLicht to refresh
                        // the display, now that Material has changed.
                        static_cast<scene::IMeshSceneNode*>
                                 (m_unlocked_stuff[n].m_side_1)->setMesh(mesh);

                        m_unlocked_stuff[n].m_curr_image = textureID;


                        mesh = static_cast<scene::IMeshSceneNode*>
                                    (m_unlocked_stuff[n].m_side_2)->getMesh();
                        assert(mesh->getMeshBufferCount() == 1);
                        mb = mesh->getMeshBuffer(0);

                        SMaterial& m2 = mb->getMaterial();
                        m2.setTexture(0, m_unlocked_stuff[n].m_pictures[textureID]);

                        // FIXME: this mesh is already associated with this
                        // node. I'm calling this to force irrLicht to refresh
                        // the display, now that Material has changed.
                        static_cast<scene::IMeshSceneNode*>
                            (m_unlocked_stuff[n].m_side_2)->setMesh(mesh);

                        m_unlocked_stuff[n].m_curr_image = textureID;
                    }
#endif
                }   // textureID != previousTextureID
            }   // if picture_count>1
        }   // if !m_unlocked_stuff[n].m_pictures.empty()

        float scale = m_unlocked_stuff[n].m_scale;
        if (m_global_time <= GIFT_EXIT_FROM)
            scale *= 0.1f;
        else if (m_global_time > GIFT_EXIT_FROM &&
                 m_global_time < GIFT_EXIT_TO)
        {
            scale *= (  (m_global_time - GIFT_EXIT_FROM)
                       / (GIFT_EXIT_TO - GIFT_EXIT_FROM) );
        }
        m_unlocked_stuff[n].m_root_gift_node
                            ->setScale(core::vector3df(scale, scale, scale));
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
                                       core::rect< s32 >( 0, message_y, w,
                                                             message_y + fontH ),
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
        Log::error("FeatureUnlockedCutScene::addUnlockedTrack",
                   "Unlocked track does not exist");
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
    core::stringw gpname;
    if (gp == NULL)
    {
        Log::error("FeatureUnlockedCutScene::addUnlockedGP",
                   "Unlocked GP does not exist");
        const std::string t_name =
            file_manager->getAsset(FileManager::GUI_ICON, "main_help.png");
        video::ITexture* WTF_image = irr_driver->getTexture(t_name);
        images.push_back(WTF_image);
    }
    else
    {
        const std::vector<std::string> gptracks = gp->getTrackNames();
        const int track_amount = (int)gptracks.size();

        if (track_amount == 0)
        {
            Log::error("FeatureUnlockedCutScene::addUnlockedGP",
                       "Unlocked GP is empty");
            video::ITexture* WTF_image =
                irr_driver->getTexture( file_manager
                                        ->getAsset(FileManager::GUI_ICON,"main_help.png"));
            images.push_back(WTF_image);
        }

        for (int t=0; t<track_amount; t++)
        {
            Track* track = track_manager->getTrack(gptracks[t]);

            const std::string t_name = 
                track ? track->getScreenshotFile()
                      : file_manager->getAsset(FileManager::GUI_ICON, "main_help.png");
            ITexture* tex = irr_driver->getTexture(t_name);
            images.push_back(tex);
        }
        gpname = gp->getName();
    }

    addUnlockedPictures(images, 4.0f, 3.0f, _("You unlocked grand prix %0", gpname));
}

// ----------------------------------------------------------------------------

bool FeatureUnlockedCutScene::onEscapePressed()
{
    continueButtonPressed();
    return false; // continueButtonPressed already pop'ed the menu
}   // onEscapePressed

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::continueButtonPressed()
{
    if (m_global_time < GIFT_EXIT_TO)
    {
        // If animation was not over yet, the button is used to skip the animation
        while (m_global_time < GIFT_EXIT_TO)
        {
            // simulate all the steps of the animation until we reach the end
            onUpdate(0.4f);
            World::getWorld()->updateWorld(stk_config->time2Ticks(0.4f));
        }
    }
    else
    {
        ((CutsceneWorld*)World::getWorld())->abortCutscene();
    }

}   // continueButtonPressed

// ----------------------------------------------------------------------------

void FeatureUnlockedCutScene::eventCallback(GUIEngine::Widget* widget,
                                            const std::string& name,
                                            const int playerID)
{
    if (name == "continue")
    {
        continueButtonPressed();
    }
}   // eventCallback

// ----------------------------------------------------------------------------

MusicInformation* FeatureUnlockedCutScene::getInGameMenuMusic() const
{
    MusicInformation* mi = stk_config->m_unlock_music;
    return mi;
}
