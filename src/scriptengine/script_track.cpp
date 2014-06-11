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
#include "tracks/track_object.hpp"
#include "tracks/track.hpp"
#include <iostream> //debug
namespace Scripting
{

    namespace Track
    {

        asIScriptFunction* registerScriptCallbacks(asIScriptEngine *engine)
        {
            asIScriptFunction *func;
            func = engine->GetModule(0)->GetFunctionByDecl("void onTrigger()");
            return func;
        }
        /*
        void disableAnimation(std::string *name, void *memory)
        {
            std::string *str = name;
            std::string type = "mesh";
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str, type);
        }*/
        void getTrackObject(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            TrackObject* t_obj = World::getWorld()->getTrack()->getTrackObjectManager()->getTrackObject(*str);
            gen->SetReturnObject(t_obj);
        }
        /*TrackObject* getTrackObject(std::string *name)
        {
            TrackObject* t_obj = World::getWorld()->getTrack()->getTrackObjectManager()->getTrackObject(*name);
            return t_obj;
        }*/
        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r;

            r = engine->RegisterGlobalFunction("void displayMessage(string &in)", asFUNCTION(displayMessage), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disableAnimation(string &in)", asFUNCTION(disableAnimation), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableAnimation(string &in)", asFUNCTION(enableAnimation), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableTrigger(string &in)", asFUNCTION(enableTrigger), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disableTrigger(string &in)", asFUNCTION(disableTrigger), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void createTrigger(string &in,float x,float y,float z, float distance)",
                asFUNCTION(createTrigger), asCALL_GENERIC); assert(r >= 0);
            /*
            //Test singleton, and various calling conventions
            // Register the track object manager as a singleton. The script will access it through the global property
       //     r = engine->RegisterObjectType("TrackObjectManager", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
            r = engine->RegisterObjectType("TrackObjectManager", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

            // Register the track object manager's methods
            TrackObjectManager* track_obj_manager = World::getWorld()->getTrack()->getTrackObjectManager();
            r = engine->RegisterGlobalProperty("TrackObjectManager track_obj_manager", track_obj_manager); assert(r >= 0);
            //r = engine->RegisterObjectMethod("TrackObjectManager", "void disable(string name , string type)", asMETHOD(TrackObjectManager, disable), asCALL_THISCALL); assert(r >= 0);
            //r = engine->RegisterObjectMethod("TrackObjectManager", "void disable(string &in name)", asFUNCTION(disableAnimation), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObjectManager", "void disable(string &in)", asFUNCTION(disableAnimation), asCALL_CDECL_OBJLAST); assert(r >= 0);
            */
            r = engine->RegisterObjectType("TrackObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterGlobalFunction("TrackObject @getTrackObject(string &in)", asFUNCTION(getTrackObject), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "void setEnable(bool status)", asMETHOD(TrackObject, setEnable), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectType("PhysicalObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "PhysicalObject @getPhysicalObject()", asMETHOD(TrackObject, getPhysicalObjectForScript), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "bool isFlattener()", asMETHOD(PhysicalObject, isFlattenKartObject), asCALL_THISCALL); assert(r >= 0);

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
        void createTrigger(asIScriptGeneric *gen)
        {
            std::string *script_name = (std::string*)gen->GetArgAddress(0);
            float x = gen->GetArgFloat(1);
            float y = gen->GetArgFloat(2);
            float z = gen->GetArgFloat(3);
            float distance = gen->GetArgFloat(4); //triggering distance
            core::vector3df posi(0, 0, 0);
            core::vector3df hpr(0, 0, 0);
            core::vector3df scale(1.0f, 1.0f, 1.0f);
            TrackObjectPresentationActionTrigger* newtrigger =
                new TrackObjectPresentationActionTrigger(posi, *script_name, distance);
            TrackObject* tobj = new TrackObject(posi, hpr, scale,
                "none", newtrigger, false /* isDynamic */, NULL /* physics settings */);
            World::getWorld()->getTrack()->getTrackObjectManager()->insertObject(tobj);
        }


    }
}
