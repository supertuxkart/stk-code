//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "items/powerup.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "items/item_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include "network/race_state.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"


const wchar_t* getAnchorString()
{
    const int ANCHOR_STRINGS_COUNT = 3;

    RandomGenerator r;
    const int id = r.get(ANCHOR_STRINGS_COUNT);

    switch (id)
    {
        //I18N: shown when anchor applied. %s is the victim.
        case 0: return _("Arrr, the %s dropped anchor, Captain!");
        case 1: return _("%s pays the next round of grog!");
        case 2: return _("%s is a mighty pirate!");
        default: assert(false); return L"";   // avoid compiler warning.
    }
}   // getAnchorString

//-----------------------------------------------------------------------------
const wchar_t* getParachuteString()
{
    const int PARACHUTE_STRINGS_COUNT = 3;

    RandomGenerator r;
    const int id = r.get(PARACHUTE_STRINGS_COUNT);

    switch (id)
    {
        case 0: return _("Geronimo!!!"); // Parachutist shout
        case 1: return _("The Space Shuttle has landed!");
        case 2: return _("Do you want to fly kites?");
        default: assert(false); return  L"";  // avoid compiler warning
    }
}   // getParachuteString

//-----------------------------------------------------------------------------
const wchar_t* getSwapperString()
{
    const int SWAPPER_STRINGS_COUNT = 3;

    RandomGenerator r;
    const int id = r.get(SWAPPER_STRINGS_COUNT);

    switch (id)
    {
        case 0: return _("Magic, son. Nothing else in the world smells like that.");
        case 1: return _("A wizard did it!");
        case 2: return _("Banana? Box? Banana? Box? Banana? Box?");
        default: assert(false); return L"";  // avoid compiler warning
    }
}   // getSwapperString

//-----------------------------------------------------------------------------
/** Constructor, stores the kart to which this powerup belongs. 
 *  \param kart The kart to which this powerup belongs. 
 */
Powerup::Powerup(Kart* kart)
{
    m_owner               = kart;
    m_sound_use           = NULL;
    reset();
}   // Powerup

//-----------------------------------------------------------------------------
/** Frees the memory for the sound effects.
 */
Powerup::~Powerup()
{
    if(m_sound_use) sfx_manager->deleteSFX(m_sound_use);
}   // ~Powerup

//-----------------------------------------------------------------------------
/** Resets the powerup, called at begin of a race.
 */
void Powerup::reset()
{
    m_type = PowerupManager::POWERUP_NOTHING;
    m_number = 0;
    
    int type, number;
    World::getWorld()->getDefaultCollectibles( type, number );
    set( (PowerupManager::PowerupType)type, number );
}   // reset

//-----------------------------------------------------------------------------
/** Sets the collected items. The number of items is increased if the same
 *  item is currently collected, otherwise replaces the existing item. It also
 *  sets item specific sounds.
 *  \param type Thew new type.
 *  \param n Number of items of the given type.
 */
void Powerup::set(PowerupManager::PowerupType type, int n)
{
    if (m_type==type)
    {
        m_number+=n;
        return;
    }
    m_type=type;
    m_number=n;
    
    if(m_sound_use != NULL)
    {
        sfx_manager->deleteSFX(m_sound_use);
        m_sound_use = NULL;
    }
    
    switch (m_type)
    {
        case PowerupManager::POWERUP_ZIPPER:
            break ;
            
        case PowerupManager::POWERUP_BOWLING:
            m_sound_use = sfx_manager->createSoundSource("bowling_roll");
            break ;
            
        case PowerupManager::POWERUP_ANVIL:
            m_sound_use = sfx_manager->createSoundSource("use_anvil");
            break;
            
        case PowerupManager::POWERUP_PARACHUTE:
            m_sound_use = sfx_manager->createSoundSource("use_parachute");
            break;
            
        case PowerupManager::POWERUP_BUBBLEGUM:
            m_sound_use = sfx_manager->createSoundSource("goo");
            break ;
            
        case PowerupManager::POWERUP_SWITCH:
            m_sound_use = sfx_manager->createSoundSource("swap");
            break;
            
        case PowerupManager::POWERUP_NOTHING:
        case PowerupManager::POWERUP_CAKE:
        case PowerupManager::POWERUP_PLUNGER:
        default :
            m_sound_use = sfx_manager->createSoundSource("shot");
            break ;
    }
    
}  // set

//-----------------------------------------------------------------------------
/** Returns the icon for the currently collected powerup. Used in the
 *  race_gui to display the collected item.
 */
Material *Powerup::getIcon() const
{
    // Check if it's one of the types which have a separate
    // data file which includes the icon:
    return powerup_manager->getIcon(m_type);
}

//-----------------------------------------------------------------------------
/** Use (fire) this powerup.
 */
void Powerup::use()
{
    // Play custom kart sound when collectible is used
    if (m_type != PowerupManager::POWERUP_NOTHING && 
        m_type != PowerupManager::POWERUP_ZIPPER) 
        m_owner->playCustomSFX(SFXManager::CUSTOM_SHOOT);

    // FIXME - for some collectibles, set() is never called
    if(m_sound_use == NULL)
    {
        //if (m_type == POWERUP_SWITCH) m_sound_use = sfx_manager->newSFX(SFXManager::SOUND_SWAP);
        //else                          
        m_sound_use = sfx_manager->createSoundSource("shot");
    }
    
    m_number--;
    World *world = World::getWorld();
    RaceGUIBase* gui = world->getRaceGUI();
    switch (m_type)
    {
    case PowerupManager::POWERUP_ZIPPER:   
        m_owner->handleZipper();
        break ;
    case PowerupManager::POWERUP_SWITCH:
        {
        item_manager->switchItems();
        m_sound_use->position(m_owner->getXYZ());
        m_sound_use->play();

        gui->addMessage(getSwapperString(), NULL, 3.0f, 40,
                        video::SColor(255, 255, 255, 255), false);
        break;
        }
    case PowerupManager::POWERUP_CAKE:
    case PowerupManager::POWERUP_BOWLING:
    case PowerupManager::POWERUP_PLUNGER:
        
        m_sound_use->position(m_owner->getXYZ());
        m_sound_use->play();
        projectile_manager->newProjectile(m_owner, m_type);
        break ;
        
    case PowerupManager::POWERUP_BUBBLEGUM:
        {
        float up_coord = Track::NOHIT;
        Vec3 normal;
        const Material* unused2;        
        btVector3 pos = m_owner->getXYZ();
        world->getTrack()->getTerrainInfo(pos, &up_coord, &normal, &unused2);
        // This can happen if the kart is 'over nothing' when dropping
        // the bubble gum
        if(up_coord==Track::NOHIT)
            return;
        normal.normalize();
        m_sound_use->position(m_owner->getXYZ());
        m_sound_use->play();
        
        pos.setY(up_coord-0.05f);
        
        item_manager->newItem(Item::ITEM_BUBBLEGUM, pos, normal, m_owner);
        }
        break;
        
    case PowerupManager::POWERUP_ANVIL:
        
        //Attach an anvil(twice as good as the one given
        //by the bananas) to the kart in the 1st position.
        for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
        {
            Kart *kart=world->getKart(i);
            if(kart->isEliminated()) continue;
            if(kart == m_owner) continue;
            if(kart->getPosition() == 1)
            {
                kart->attach(ATTACH_ANVIL, stk_config->m_anvil_time);
                kart->updatedWeight();
                kart->adjustSpeed(stk_config->m_anvil_speed_factor*0.5f);
                
                // should we position the sound at the kart that is hit,
                // or the kart "throwing" the anvil? Ideally it should be both.
                // Meanwhile, don't play it near AI karts since they obviously
                // don't hear anything
                if(kart->getController()->isPlayerController())
                    m_sound_use->position(kart->getXYZ());
                else
                    m_sound_use->position(m_owner->getXYZ());
                
                m_sound_use->play();

                irr::core::stringw anchor_message;
                anchor_message += StringUtils::insertValues(getAnchorString(), kart->getName().c_str()).c_str();
                gui->addMessage(anchor_message, NULL, 3.0f, 40, video::SColor(255, 255, 255, 255), false);
                break;
            }
        }

        break;

    case PowerupManager::POWERUP_PARACHUTE:
        {
            Kart* player_kart = NULL;
            //Attach a parachutte(that last twice as long as the
            //one from the bananas) to all the karts that
            //are in front of this one.
            for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
            {
                Kart *kart=world->getKart(i);
                if(kart->isEliminated() || kart== m_owner) continue;
                if(m_owner->getPosition() > kart->getPosition())
                {
                    kart->attach(ATTACH_PARACHUTE, stk_config->m_parachute_time_other);

                    if(kart->getController()->isPlayerController())
                        player_kart = kart;
                }
            }

            // should we position the sound at the kart that is hit,
            // or the kart "throwing" the anvil? Ideally it should be both.
            // Meanwhile, don't play it near AI karts since they obviously
            // don't hear anything
            if(m_owner->getController()->isPlayerController())
                m_sound_use->position(m_owner->getXYZ());
            else if(player_kart)
                m_sound_use->position(player_kart->getXYZ());
            m_sound_use->play();

            gui->addMessage(getParachuteString(), NULL, 3.0f, 40, video::SColor(255, 255, 255, 255), false);
        }
        break;

    case PowerupManager::POWERUP_NOTHING:
    default :              break ;
    }

    if ( m_number <= 0 )
    {
        m_number = 0;
        m_type   = PowerupManager::POWERUP_NOTHING;
    }
}   // use

//-----------------------------------------------------------------------------
/** This function is called when a bnous box is it. This function can be
 *  called on a server (in which case item and add_info are not used),
 *  or on a client, in which case the item and additional info is used
 *  to make sure server and clients are synched correctly.
 *  \param n
 *  \param item The item (bonux box) that was hit. This is necessary
 *         for servers so that the clients can be informed which item
 *         was collected.
 *  \param add_info Additional information. This is used for network games
 *         so that the server can overwrite which item is collectted 
 *         (otherwise a random choice is done).
 */
void Powerup::hitBonusBox(const Item &item, int add_info)
{
    // Position can be -1 in case of a battle mode (which doesn't have 
    // positions), but this case is properly handled in getRandomPowerup.
    int position = m_owner->getPosition();
    
    unsigned int n=1;
    PowerupManager::PowerupType new_powerup = 
        powerup_manager->getRandomPowerup(position, &n);

    // Always add a new powerup in ITEM_MODE_NEW (or if the kart
    // doesn't have a powerup atm).
    if(m_type == PowerupManager::POWERUP_NOTHING ||
       stk_config->m_same_powerup_mode == STKConfig::POWERUP_MODE_NEW )
    {
        set( new_powerup, n );
    }
    else
    {
        // If powerup mode is 'SAME', or it's ONLY_IF_SAME and it is the
        // same powerup, increase the number of items.
        if(stk_config->m_same_powerup_mode == STKConfig::POWERUP_MODE_SAME ||
            new_powerup==m_type)
        {
            m_number+=n;
            if(m_number > MAX_POWERUPS) 
                m_number = MAX_POWERUPS;
        }
    }
    // Ignore new powerup if it is different from the current one and not
    // POWERUP_MODE_SAME

}   // hitBonusBox
