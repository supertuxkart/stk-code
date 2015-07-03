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
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"

#include <angelscript.h>
#include <assert.h>

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

        /** Get number of challenges that were completed at any difficulty */
        int getCompletedChallengesCount()
        {
            ::Track* track = World::getWorld()->getTrack();
            return track->getNumOfCompletedChallenges();
        }

        /** Get total number of challenges */
        int getChallengeCount()
        {
            ::Track* track = World::getWorld()->getTrack();
            return track->getChallengeList().size();
        }

        /** @}*/
        /** @}*/

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r; // of type asERetCodes

            engine->SetDefaultNamespace("Challenges");

            r = engine->RegisterGlobalFunction("int getCompletedChallengesCount()", asFUNCTION(getCompletedChallengesCount), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getChallengeCount()", asFUNCTION(getChallengeCount), asCALL_CDECL); assert(r >= 0);
        }

    }

    /** \cond DOXYGEN_IGNORE */
}
/** \endcond */

