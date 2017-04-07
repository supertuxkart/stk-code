//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015  SuperTuxKart Team
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

#include "script_track.hpp"

#include "animations/three_d_animation.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "modes/world.hpp"
#include "config/player_manager.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"

#include <angelscript.h>
#include <assert.h>
#include <ISceneManager.h>
#include <IBillboardTextSceneNode.h>

/** \cond DOXYGEN_IGNORE */
namespace Scripting
{
    /** \endcond */
    namespace Challenges
    {
        /** \addtogroup Scripting
        * @{
        */
        /** \addtogroup Scripting_Challenges Challenges
        * @{
        */

        // --------------------------------------------------------------------
        /** Get total number of challenges */
        int getChallengeCount()
        {
            return (int)::Track::getCurrentTrack()->getChallengeList().size();
        }   // getChallengeCount

        // --------------------------------------------------------------------
        /** Get number of challenges that were completed at any difficulty */
        int getCompletedChallengesCount()
        {
            if (UserConfigParams::m_everything_unlocked)
                return getChallengeCount();

            return ::Track::getCurrentTrack()->getNumOfCompletedChallenges();
        }   // getCompletedChallengesCount

        // --------------------------------------------------------------------
        int getChallengeRequiredPoints(std::string* challenge_name)
        {
            const ChallengeData* challenge =
                             unlock_manager->getChallengeData(*challenge_name);
            if (challenge == NULL)
            {
                if (*challenge_name != "tutorial")
                    Log::error("track", "Cannot find challenge named '%s'\n",
                               challenge_name->c_str());
                return false;
            }

            return challenge->getNumTrophies();
        }   // getChallengeRequiredPoints

        // --------------------------------------------------------------------
        bool isChallengeUnlocked(std::string* challenge_name)
        {
            if (UserConfigParams::m_everything_unlocked)
                return true;

            const ChallengeData* challenge =
                             unlock_manager->getChallengeData(*challenge_name);
            if (challenge == NULL)
            {
                if (*challenge_name != "tutorial")
                    Log::error("track", "Cannot find challenge named '%s'\n",
                    challenge_name->c_str());
                return false;
            }

            const unsigned int val = challenge->getNumTrophies();
            bool shown = (PlayerManager::getCurrentPlayer()->getPoints() >= val);
            return shown;
        }   // isChallengeUnlocked

        // --------------------------------------------------------------------
        /** @}*/
        /** @}*/

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r; // of type asERetCodes

            engine->SetDefaultNamespace("Challenges");

            r = engine->RegisterGlobalFunction("int getCompletedChallengesCount()", 
                                               asFUNCTION(getCompletedChallengesCount),
                                               asCALL_CDECL);
            assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getChallengeCount()", 
                                               asFUNCTION(getChallengeCount),
                                               asCALL_CDECL);
            assert(r >= 0);
            r = engine->RegisterGlobalFunction("bool isChallengeUnlocked(string &in)",
                                               asFUNCTION(isChallengeUnlocked),
                                               asCALL_CDECL);
            assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getChallengeRequiredPoints(string &in)",
                                               asFUNCTION(getChallengeRequiredPoints),
                                               asCALL_CDECL);
            assert(r >= 0);
        }   // registerScriptFunctions

    }   // namespace Challenges

    /** \cond DOXYGEN_IGNORE */
}   // namespace Scripting
/** \endcond */

