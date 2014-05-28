//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014  SuperTuxKart Team
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

#include <assert.h>
#include <angelscript.h>
#include "modes/world.hpp"
#include "script_track.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track_object_manager.hpp"
#include "tracks/track.hpp"

namespace Scripting{

    namespace Track{

        asIScriptFunction* registerScriptCallbacks(asIScriptEngine *engine)
        {
            asIScriptFunction *func;
            func = engine->GetModule(0)->GetFunctionByDecl("void onTrigger()");
            return func;
        }
        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r;

            r = engine->RegisterGlobalFunction("void displayMessage(string &in)", asFUNCTION(displayMessage), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disableAnimation(string &in)", asFUNCTION(disableAnimation), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableAnimation(string &in)", asFUNCTION(enableAnimation), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableTrigger(string &in)", asFUNCTION(enableTrigger), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disableTrigger(string &in)", asFUNCTION(disableTrigger), asCALL_GENERIC); assert(r >= 0);

        }





        // Displays the message specified in displayMessage( string message ) within the script
        void displayMessage(asIScriptGeneric *gen)
        {
            std::string *input = (std::string*)gen->GetArgAddress(0);
            irr::core::stringw out = irr::core::stringw((*input).c_str()); //irr::core::stringw supported by message dialogs
            new TutorialMessageDialog((out), true);
        }
        void disableAnimation(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            std::string type = "mesh";
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str, type);
        }
        void enableAnimation(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            std::string type = "mesh";
            World::getWorld()->getTrack()->getTrackObjectManager()->enable(*str, type);
        }
        void disableTrigger(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            std::string type = "action-trigger";
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str, type);
        }
        void enableTrigger(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            std::string type = "action-trigger";
            World::getWorld()->getTrack()->getTrackObjectManager()->enable(*str, type);
        }



    }
}
