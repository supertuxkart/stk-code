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

#ifndef HEADER_FEATURE_UNLOCKED_HPP
#define HEADER_FEATURE_UNLOCKED_HPP

#include "graphics/irr_driver.hpp"
#include "guiengine/screen.hpp"
#include "race/race_manager.hpp"
#include "utils/ptr_vector.hpp"

#include <memory>

namespace irr {
    namespace scene { class ISceneNode; class ICameraSceneNode;
                      class ILightSceneNode;                       }
}

namespace SP
{
    class SPTexture;
}

class KartModel;
class KartProperties;
class ChallengeData;

/**
  * \brief Screen shown when a feature has been unlocked
  * \ingroup states_screens
 */
class FeatureUnlockedCutScene : public GUIEngine::CutsceneScreen,
                                public GUIEngine::ScreenSingleton<FeatureUnlockedCutScene>
{
    friend class GUIEngine::ScreenSingleton<FeatureUnlockedCutScene>;

    FeatureUnlockedCutScene();

    /** Whichever of these is non-null decides what comes out of the chest */
    struct UnlockedThing
    {
        /** Will be non-null if this unlocked thing is a kart */
        const KartProperties* m_unlocked_kart;

        std::string m_unlock_model;

        /** Will be non-empty if this unlocked thing is one or many pictures */
        std::vector<irr::video::ITexture*> m_pictures;

        std::vector<std::shared_ptr<SP::SPTexture> > m_sp_pictures;

        /** Will be set if this unlocked thing is a picture */
        float m_w, m_h;
        /** used for slideshows */
        int m_curr_image;

        /** Contains whatever is in the chest */
        scene::ISceneNode* m_root_gift_node;

        scene::ISceneNode* m_side_1;
        scene::ISceneNode* m_side_2;

        float m_scale;

        irr::core::stringw m_unlock_message;

        UnlockedThing(const std::string &model, const irr::core::stringw &msg);

        UnlockedThing(const KartProperties* kart, const irr::core::stringw &msg);

        /**
          * Creates a 'picture' reward.
          * \param pict  the picture to display as reward.
          * \param w     width of the picture to display
          * \param y     height of the picture to display
          */
        UnlockedThing(irr::video::ITexture* pict, float w, float h,
                      const irr::core::stringw &msg);

        /**
         * Creates a 'picture slideshow' reward.
         * \param picts the pictures to display as reward.
         * \param w     width of the pictures to display
         * \param y     height of the pictures to display
         */
        UnlockedThing(std::vector<irr::video::ITexture*> picts, float w, float h,
                      const irr::core::stringw &msg);

        ~UnlockedThing();
    };

    /** The list of all unlocked things. */
    PtrVector<UnlockedThing, HOLD> m_unlocked_stuff;

    /** To store the copy of the KartModel for each unlocked kart. */
    PtrVector<KartModel> m_all_kart_models;

    /** Global evolution of time */
    float m_global_time;

    /** Key position from origin (where the chest is) */
    float m_key_pos;

    /** Angle of the key (from 0 to 1, simply traces progression) */
    float m_key_angle;

    void continueButtonPressed();

public:

    virtual void onCutsceneEnd() OVERRIDE;

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    void onUpdate(float dt) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    void tearDown() OVERRIDE;

    void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                       const int playerID) OVERRIDE;

    void findWhatWasUnlocked(RaceManager::Difficulty difficulty, 
                             std::vector<const ChallengeData*>& unlocked);

    /** Call before showing up the screen to make a kart come out of the chest.
     *  'addUnlockedThings' will invoke this, so you generally don't need to 
     *  call this directly. */
    void addUnlockedKart(const KartProperties* unlocked_kart);

    /** Call before showing up the screen to make a picture come out of the 
     *  chest 'addUnlockedThings' will invoke this, so you generally don't 
     *  need to call this directly. */
    void addUnlockedPicture(irr::video::ITexture* picture, float w, float h,
                            const irr::core::stringw &msg);

    /** Call before showing up the screen to make a picture slideshow come out
     *  of the chest 'addUnlockedThings' will invoke this, so you generally
     *  don't need to call this directly. */
    void addUnlockedPictures(std::vector<irr::video::ITexture*> pictures,
                             float w, float h, irr::core::stringw msg);

    void addUnlockedTrack(const Track* track);
    void addUnlockedGP(const GrandPrixData* gp);

    /** Call before showing up the screen to make whatever the passed challenges unlocked
      * come out of the chest */
    // unused for now... maybe this could could useful later?
    /*
    void addUnlockedThings(const std::vector<const ChallengeData*> unlocked);
    */

    void addTrophy(RaceManager::Difficulty difficulty, bool is_grandprix);

    /** override from base class to handle escape press */
    virtual bool onEscapePressed() OVERRIDE;

    virtual MusicInformation* getInGameMenuMusic() const OVERRIDE;
};

#endif
