//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2025 Alayan
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

#include "karts/kart_utils.hpp"

#include "achievements/achievements_status.hpp"
#include "config/player_manager.hpp"
#include "items/powerup.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/controller.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/world.hpp"

namespace KartUtils
{
    void createExplosion(Kart *kart, bool small, const Vec3 &pos, bool direct_hit, Kart *author)
    {
        World *world = World::getWorld();
        // No explosion on invulnerable karts or during the goal phase
        if (kart->isInvulnerable() || world->isGoalPhase())
            return;

        // Ignore explosions that are too far away.
        float r = kart->getKartProperties()->getExplosionRadius();
        if(!direct_hit && pos.distance2(kart->getXYZ()) > r*r)
            return;

        if(kart->isShielded())
        {
            kart->decreaseShieldTime();
            return;
        }

        // If the kart is already being exploded or rescued, or in a cannon, do nothing
        if (kart->getKartAnimation() != nullptr)
            return;

        ExplosionAnimation::create(kart, direct_hit, small);

        // Rumble!
        // TODO : different rumble strength on direct_hit?
        Controller *controller = kart->getController();
        if (controller && controller->isLocalPlayerController())
            controller->rumble(0, 0.8f, 500);

        // Play a hurt kart-model animation for the kart that was hit
        // TODO : because of the heavy kart movement due to the main explosion
        //        animation, little is visible from the kart model movement.
        //        Either the main explosion animation should move less, or
        //        the kart hurt animation should last long enough to exceed the
        //        kart animation, or the start of the kart-model animation should
        //        be delayed, or some mix of all three solutions.
        kart->getKartModel()->setAnimation(KartModel::AF_HIT_START);

        if (RaceManager::get()->isFollowMode())
        {
            FollowTheLeaderRace *ftl_world = dynamic_cast<FollowTheLeaderRace*>(world);
            if(ftl_world->isLeader(kart->getWorldKartId()))
                ftl_world->leaderHit();
        }

        // Special actions when there is a direct hit
        if (direct_hit)
        {
            // Register the hit with the world (for example for battle scoring)
            int author_id = (author != nullptr) ? author->getWorldKartId() : -1;
            world->kartHit(kart->getWorldKartId(), author_id);

            // Play a happy kart-model animation for the kart that hit the other kart
            // TODO : prevent the interruption of another kart model animation
            //        The interruption is easily reproducible by simply bowling two
            //        karts in a short time interval.
            if (author != nullptr)
                author->getKartModel()->setAnimation(KartModel::AF_HAPPY_START);

            // Increase achievement counters as appropriate
            if (author != nullptr && author->getController()->canGetAchievements())
            {
                if (author_id != (int)kart->getWorldKartId())
                    PlayerManager::addKartHit(kart->getWorldKartId());
                PlayerManager::increaseAchievement(AchievementsStatus::ALL_HITS, 1);
                if (RaceManager::get()->isLinearRaceMode())
                    PlayerManager::increaseAchievement(AchievementsStatus::ALL_HITS_1RACE, 1);
            }

            // Clear the powerups of the exploding kart in CTF
            if (RaceManager::get()->isCTFMode())
                 kart->getPowerup()->reset();
        } // direct_hit
    } // createExplosion
}   // namespace KartUtils