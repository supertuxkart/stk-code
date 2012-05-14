//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 SuperTuxKart-Team
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

namespace irr { 
    namespace scene { class ISceneNode; class ICameraSceneNode; 
                      class ILightSceneNode;                       } 
}
class KartModel;
class KartProperties;
class ChallengeData;

/**
  * \brief Screen shown when a feature has been unlocked
  * \ingroup states_screens
 */
class FeatureUnlockedCutScene : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<FeatureUnlockedCutScene>
{
    friend class GUIEngine::ScreenSingleton<FeatureUnlockedCutScene>;
    
    FeatureUnlockedCutScene();
    
    /** Whichever of these is non-null decides what comes out of the chest */
    struct UnlockedThing
    {
        /** Will be non-null if this unlocked thing is a kart */
        KartProperties* m_unlocked_kart;
        
        std::string m_unlock_model;
        
        /** Will be non-empty if this unlocked thing is one or many pictures */
        std::vector<irr::video::ITexture*> m_pictures;
        /** Will be set if this unlocked thing is a picture */
        float m_w, m_h;
        /** used for slideshows */
        int m_curr_image;
        
        /** Contains whatever is in the chest */
        scene::ISceneNode* m_root_gift_node;
        
        irr::core::stringw m_unlock_message;
        
        UnlockedThing(std::string model, irr::core::stringw msg);

        UnlockedThing(KartProperties* kart, irr::core::stringw msg);
        
        /**
          * Creates a 'picture' reward.
          * \param pict  the picture to display as reward.
          * \param w     width of the picture to display
          * \param y     height of the picture to display
          */
        UnlockedThing(irr::video::ITexture* pict, float w, float h, irr::core::stringw msg);
        
        /**
         * Creates a 'picture slideshow' reward.
         * \param picts the pictures to display as reward.
         * \param w     width of the pictures to display
         * \param y     height of the pictures to display
         */
        UnlockedThing(std::vector<irr::video::ITexture*> picts, float w, float h, irr::core::stringw msg);
        
        ~UnlockedThing();
    };

    /** The list of all unlocked things. */
    PtrVector<UnlockedThing, HOLD> m_unlocked_stuff;
    
    /** To store the copy of the KartModel for each unlocked kart. */
    PtrVector<KartModel> m_all_kart_models;
    
    /** sky angle, 0-360 */
    float m_sky_angle;
    
    /** Global evolution of time */
    double m_global_time;
    
    /** Key position from origin (where the chest is) */
    float m_key_pos;
    
    /** Angle of the key (from 0 to 1, simply traces progression) */
    float m_key_angle;
    
    /** The scene node for the sky box. */
    irr::scene::ISceneNode             *m_sky;

    /** The scene node for the camera. */
    irr::scene::ICameraSceneNode       *m_camera;

    /** The scene node for the animated mesh. */
    irr::scene::IAnimatedMeshSceneNode *m_chest;

    /** The scene node for the light. */
    irr::scene::ILightSceneNode* m_light;
    
    //#define USE_IRRLICHT_BUG_WORKAROUND
    
#ifdef USE_IRRLICHT_BUG_WORKAROUND
    scene::IMeshSceneNode *m_avoid_irrlicht_bug;
#endif
    
    void continueButtonPressed();
    
public:

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    void onUpdate(float dt, irr::video::IVideoDriver*) OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void init() OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void tearDown() OVERRIDE;
    
    void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                       const int playerID) OVERRIDE;
    
    /** Call before showing up the screen to make a kart come out of the chest.
        'addUnlockedThings' will invoke this, so you generally don't need to call this directly. */
    // unused for now, maybe will be useful later?
    //void addUnlockedKart(KartProperties* unlocked_kart, irr::core::stringw msg);
    
    /** Call before showing up the screen to make a picture come out of the chest
        'addUnlockedThings' will invoke this, so you generally don't need to call this directly. */
    // unused for now, maybe will be useful later?
    //void addUnlockedPicture(irr::video::ITexture* picture, float w, float h, irr::core::stringw msg);
    
    /** Call before showing up the screen to make a picture slideshow come out of the chest
        'addUnlockedThings' will invoke this, so you generally don't need to call this directly. */
    // unused for now, maybe will be useful later?
    //void addUnlockedPictures(std::vector<irr::video::ITexture*> pictures,
    //                         float w, float h, irr::core::stringw msg);
    
    /** Call before showing up the screen to make whatever the passed challenges unlocked
      * come out of the chest */
    // unused for now... maybe this could could useful later?
    /*
    void addUnlockedThings(const std::vector<const ChallengeData*> unlocked);
    */
    
    void addTrophy(RaceManager::Difficulty difficulty);
    
    /** override from base class to handle escape press */
    virtual bool onEscapePressed() OVERRIDE;
};

#endif

