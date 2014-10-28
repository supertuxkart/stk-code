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
#include <iostream> //debug

namespace Scripting
{

    namespace Track
    {
        //register callbacks
        asIScriptFunction* registerScriptCallbacks(asIScriptEngine *engine, std::string scriptName)
        {
            asIScriptFunction *func;
            std::string function_name = "void " + scriptName + "()";
            func = engine->GetModule(0)->GetFunctionByDecl(function_name.c_str());
            return func;
        }
        asIScriptFunction* registerStartScriptCallbacks(asIScriptEngine *engine)
        {
            asIScriptFunction *func;
            func = engine->GetModule(0)->GetFunctionByDecl("void onStart()");
            return func;
        }
        asIScriptFunction* registerUpdateScriptCallbacks(asIScriptEngine *engine)
        {
            asIScriptFunction *func;
            func = engine->GetModule(0)->GetFunctionByDecl("void onUpdate()");
            return func;
        }
        /*
        void disableAnimation(std::string *name, void *memory)
        {
            std::string *str = name;
            std::string type = "mesh";
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str, type);
        }*/
        //disables track object passed from the script
        void disable(void *memory)
        {
            ((PhysicalObject*)(memory))->removeBody();
        }
        //enables track object passed from the script
        void enable(void *memory)
        {
            ((PhysicalObject*)(memory))->addBody();
        }
        //pause an animation
        void setPaused(bool mode, void *memory)
        {
            ((ThreeDAnimation*)(memory))->setPaused(mode);
        }
        //move objects of type TrackObjectPresentation, to the specified location
        void movePresentation(Vec3 *new_pos, void *memory)
        {
            core::vector3df xyz = new_pos->toIrrVector();
            core::vector3df hpr = core::vector3df(0, 0, 0);
            core::vector3df scale = core::vector3df(1, 1, 1);
            ((TrackObjectPresentation*)(memory))->move(xyz, hpr, scale);
        }
        //stop a sound
        void stop(void *memory)
        {
            ((TrackObjectPresentationSound*)memory)->stopSound();
        }
        //play the specified sound once
        void playOnce(void *memory)
        {
            ((TrackObjectPresentationSound*)memory)->triggerSound(false); //false = once
        }
        //play the specified sound continuously
        void playLoop(void *memory)
        {
            ((TrackObjectPresentationSound*)memory)->triggerSound(true); //true = loop
        }
        //sets a loop for an animation (skeletal)
        void setLoop(int start, int end, void *memory)
        {
            ((TrackObjectPresentationMesh*)(memory))->setLoop(start,end);
        }
        //sets the current frame for a skeletal animation
        void setCurrentFrame(int frame,void *memory)
        {
            ((TrackObjectPresentationMesh*)(memory))->setCurrentFrame(frame);
        }
        //getter for current frame in a skeletal animation
        void getCurrentFrame(void *memory)
        {
            ((TrackObjectPresentationMesh*)(memory))->getCurrentFrame();
        }
        //getter for key binding for player action enums
        void getKeyBinding(asIScriptGeneric *gen)
        {
            int Enum_value = (int)gen->GetArgDWord(0);
            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
            DeviceConfig* config = device->getConfiguration();
            irr::core::stringw control;
            PlayerAction ScriptAction = (PlayerAction)Enum_value;
            control = config->getBindingAsString(ScriptAction);
            std::string key = std::string(irr::core::stringc(control).c_str());
            void *key_pointer = &key;
            gen->SetReturnObject(key_pointer);
        }
        //generic track object getter, Entry point of track objects into scripts
        void getTrackObject(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            TrackObject* t_obj = World::getWorld()->getTrack()->getTrackObjectManager()->getTrackObject(*str);
            gen->SetReturnObject(t_obj);
        }
        //runs the script specified by the given string
        void runScript(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            ScriptEngine* script_engine = World::getWorld()->getScriptEngine();
            script_engine->runScript(*str);
        }
        /*TrackObject* getTrackObject(std::string *name)
        {
            TrackObject* t_obj = World::getWorld()->getTrack()->getTrackObjectManager()->getTrackObject(*name);
            return t_obj;
        }*/


        // Displays the message specified in displayMessage( string message ) within the script
        void displayMessage(asIScriptGeneric *gen)
        {
            std::string *input = (std::string*)gen->GetArgAddress(0);
            irr::core::stringw out = irr::core::stringw((*input).c_str()); //irr::core::stringw supported by message dialogs
            new TutorialMessageDialog((out), true);
        }
        //generic disable method for track objects
        void disableTrackObject(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str);
        }
        //generic enable method for track objects
        void enableTrackObject(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            World::getWorld()->getTrack()->getTrackObjectManager()->enable(*str);
        }
        //disables an action trigger of specified ID
        void disableTrigger(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str);
        }
        //enables an action trigger of specified ID
        void enableTrigger(asIScriptGeneric *gen)
        {
            std::string *str = (std::string*)gen->GetArgAddress(0);
            World::getWorld()->getTrack()->getTrackObjectManager()->enable(*str);
        }
        //Creates a trigger at the specified location
        void createTrigger(asIScriptGeneric *gen)
        {
            std::string *script_name = (std::string*)gen->GetArgAddress(0);
            Vec3 *creation_loc = (Vec3*)gen->GetArgAddress(1);
            float x = creation_loc->getX();
            float y = creation_loc->getY();
            float z = creation_loc->getZ();
            float distance = gen->GetArgFloat(2); //triggering distance
            core::vector3df posi(x, y, z);
            core::vector3df hpr(0, 0, 0);
            core::vector3df scale(1.0f, 1.0f, 1.0f);
            TrackObjectPresentationActionTrigger* newtrigger =
                new TrackObjectPresentationActionTrigger(posi, *script_name, distance);
            TrackObject* tobj = new TrackObject(posi, hpr, scale,
                "none", newtrigger, false /* isDynamic */, NULL /* physics settings */);
            tobj->setID(*script_name);
            World::getWorld()->getTrack()->getTrackObjectManager()->insertObject(tobj);
        }

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r;

            r = engine->RegisterGlobalFunction("void displayMessage(string &in)", asFUNCTION(displayMessage), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disable(string &in)", asFUNCTION(disableTrackObject), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enable(string &in)", asFUNCTION(enableTrackObject), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableTrigger(string &in)", asFUNCTION(enableTrigger), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disableTrigger(string &in)", asFUNCTION(disableTrigger), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void createTrigger(string &in,Vec3 &in, float distance)",
                asFUNCTION(createTrigger), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string getKeyBinding(int input)", asFUNCTION(getKeyBinding), asCALL_GENERIC); assert(r >= 0);


            /*
            //Test singleton, and various calling conventions
            // Register the track object manager as a singleton. The script will access it through the global property
            //r = engine->RegisterObjectType("TrackObjectManager", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(r >= 0);
            r = engine->RegisterObjectType("TrackObjectManager", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

            // Register the track object manager's methods
            TrackObjectManager* track_obj_manager = World::getWorld()->getTrack()->getTrackObjectManager();
            r = engine->RegisterGlobalProperty("TrackObjectManager track_obj_manager", track_obj_manager); assert(r >= 0);
            //r = engine->RegisterObjectMethod("TrackObjectManager", "void disable(string name , string type)", asMETHOD(TrackObjectManager, disable), asCALL_THISCALL); assert(r >= 0);
            //r = engine->RegisterObjectMethod("TrackObjectManager", "void disable(string &in name)", asFUNCTION(disableAnimation), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObjectManager", "void disable(string &in)", asFUNCTION(disableAnimation), asCALL_CDECL_OBJLAST); assert(r >= 0);
            */
            //TrackObject
            r = engine->RegisterObjectType("TrackObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterGlobalFunction("TrackObject @getTrackObject(string &in)", asFUNCTION(getTrackObject), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "void setEnable(bool status)", asMETHOD(TrackObject, setEnable), asCALL_THISCALL); assert(r >= 0);


            //PhysicalObject
            r = engine->RegisterObjectType("PhysicalObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "PhysicalObject @getPhysicalObject()", asMETHOD(TrackObject, getPhysicalObjectForScript), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "bool isFlattener()", asMETHOD(PhysicalObject, isFlattenKartObject), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "void disable()", asFUNCTION(disable), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "void enable()", asFUNCTION(enable), asCALL_CDECL_OBJLAST); assert(r >= 0);


            //Mesh or Skeletal Animation
            r = engine->RegisterObjectType("Mesh", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Mesh @getMesh()", asMETHOD(TrackObject, getMesh), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "void setLoop(int start, int end)", asFUNCTION(setLoop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "int getCurrentFrame()", asFUNCTION(getCurrentFrame), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "void setCurrentFrame(int frame)", asFUNCTION(setCurrentFrame), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);

            //Particle Emitter
            r = engine->RegisterObjectType("ParticleEmitter", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "ParticleEmitter @getParticleEmitter()", asMETHOD(TrackObject, getParticles), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("ParticleEmitter", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);

            //Sound Effect
            r = engine->RegisterObjectType("SoundEmitter", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "SoundEmitter @getSoundEmitter()", asMETHOD(TrackObject, getSound), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void stop()", asFUNCTION(stop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void playOnce()", asFUNCTION(playOnce), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void playLoop()", asFUNCTION(playLoop), asCALL_CDECL_OBJLAST); assert(r >= 0);



            //Curve based Animation
            r = engine->RegisterObjectType("Animator", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Animator @getAnimator()", asMETHOD(TrackObject, getAnimatorForScript), asCALL_THISCALL); assert(r >= 0);
            //fails due to insufficient visibility to scripts TODO : Decide whether to fix visibility or introduce wrappers
            //r = engine->RegisterObjectMethod("Animator", "void setPaused(bool mode)", asMETHOD(ThreeDAnimation, setPaused), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("Animator", "void setPaused(bool mode)", asFUNCTION( setPaused ), asCALL_CDECL_OBJLAST); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void runScript(string &in)", asFUNCTION(runScript), asCALL_GENERIC); assert(r >= 0);

        }

        void registerScriptEnums(asIScriptEngine *engine)
        {

            engine->RegisterEnum("PA");
            engine->RegisterEnumValue("PA", "STEER_LEFT", PA_STEER_LEFT);
            engine->RegisterEnumValue("PA", "STEER_RIGHT", PA_STEER_RIGHT);
            engine->RegisterEnumValue("PA", "ACCEL", PA_ACCEL);
            engine->RegisterEnumValue("PA", "BRAKE", PA_BRAKE);
            engine->RegisterEnumValue("PA", "NITRO", PA_NITRO);
            engine->RegisterEnumValue("PA", "DRIFT", PA_DRIFT);
            engine->RegisterEnumValue("PA", "RESCUE", PA_RESCUE);
            engine->RegisterEnumValue("PA", "FIRE", PA_FIRE);
            engine->RegisterEnumValue("PA", "LOOK_BACK", PA_LOOK_BACK);
            engine->RegisterEnumValue("PA", "PAUSE_RACE", PA_PAUSE_RACE);
            engine->RegisterEnumValue("PA", "MENU_UP", PA_MENU_UP);
            engine->RegisterEnumValue("PA", "MENU_DOWN", PA_MENU_DOWN);
            engine->RegisterEnumValue("PA", "MENU_LEFT", PA_MENU_LEFT);
            engine->RegisterEnumValue("PA", "MENU_RIGHT", PA_MENU_RIGHT);
            engine->RegisterEnumValue("PA", "MENU_SELECT", PA_MENU_SELECT);
            engine->RegisterEnumValue("PA", "MENU_CANCEL", PA_MENU_CANCEL);

        }





    }
}
