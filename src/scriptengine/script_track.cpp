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
    namespace Track
    {
        /** \addtogroup Scripting
        * @{
        */
        /** \addtogroup Scripting_Track Track
        * @{
        */

        /*
        void disableAnimation(std::string *name, void *memory)
        {
        std::string *str = name;
        std::string type = "mesh";
        World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str, type);
        }*/

        /**
          * Get a track object by ID.
          * @return An object of type @ref Scripting_TrackObject
          */
        TrackObject* getTrackObject(std::string* objID)
        {
            return World::getWorld()->getTrack()->getTrackObjectManager()->getTrackObject(*objID);
        }

        /** Hide/disable a track object */
        void disableTrackObject(std::string* objID)
        {
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*objID);
        }

        /** Show/enable a track objects */
        void enableTrackObject(std::string* objID)
        {
            World::getWorld()->getTrack()->getTrackObjectManager()->enable(*objID);
        }

        /** Disables an action trigger of specified ID */
        void disableTrigger(std::string* triggerID)
        {
            World::getWorld()->getTrack()->getTrackObjectManager()->disable(*triggerID);
        }

        /** Enables an action trigger of specified ID */
        void enableTrigger(std::string* triggerID)
        {
            World::getWorld()->getTrack()->getTrackObjectManager()->enable(*triggerID);
        }

        /** Creates a trigger at the specified location */
        void createTrigger(std::string* triggerID, Vec3* creation_loc, float distance)
        {
            float x = creation_loc->getX();
            float y = creation_loc->getY();
            float z = creation_loc->getZ();
            core::vector3df posi(x, y, z);
            core::vector3df hpr(0, 0, 0);
            core::vector3df scale(1.0f, 1.0f, 1.0f);
            TrackObjectPresentationActionTrigger* newtrigger =
                new TrackObjectPresentationActionTrigger(posi, *triggerID, distance);
            TrackObject* tobj = new TrackObject(posi, hpr, scale,
                "none", newtrigger, false /* isDynamic */, NULL /* physics settings */);
            tobj->setID(*triggerID);
            World::getWorld()->getTrack()->getTrackObjectManager()->insertObject(tobj);
        }

        /** Exits the race to the main menu */
        void exitRace()
        {
            World::getWorld()->scheduleExitRace();
        }
    }

    /** \cond DOXYGEN_IGNORE */
    namespace Track
    {
    /** \endcond */

        // ----------- TrackObjectPresentationMesh methods -----------
        // TODO: this method is WRONG, we should in most cases move not the presentation but the entire object
        //void movePresentation(Vec3 *new_pos, void *memory)
        //{
        //    core::vector3df xyz = new_pos->toIrrVector();
        //    core::vector3df hpr = core::vector3df(0, 0, 0);
        //    core::vector3df scale = core::vector3df(1, 1, 1);
        //    ((TrackObjectPresentation*)(memory))->move(xyz, hpr, scale, false);
        //}

        namespace Mesh
        {
            /**
            * \addtogroup Scripting_Mesh Mesh (script binding)
            * Type returned by trackObject.getMesh()
            * @{
            */

            /** Sets a loop for a skeletal animation */
            // TODO: can we use a type and avoid void* ?
            void setLoop(int start, int end /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                ((TrackObjectPresentationMesh*)(memory))->setLoop(start, end);
            }

            /** Sets the current frame for a skeletal animation */
            void setCurrentFrame(int frame /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                ((TrackObjectPresentationMesh*)(memory))->setCurrentFrame(frame);
            }

            /** Get current frame in a skeletal animation */
            int getCurrentFrame(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                return ((TrackObjectPresentationMesh*)(memory))->getCurrentFrame();
            }
            /** @} */
        }

        // ----------- Animator Object methods -----------

        namespace Animator
        {
            /**
            * \addtogroup Scripting_Animator Animator (script binding)
            * Type returned by trackObject.getIPOAnimator()
            * @{
            */

            /** Pause/resumes a curve-based animation */
            void setPaused(bool mode /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                ((ThreeDAnimation*)(memory))->setPaused(mode);
            }

            /** @} */
        }

        // ----------- Sound Object methods -----------

        namespace SoundEmitter
        {
            /**
            * @addtogroup Scripting_SoundEmitter SoundEmitter (script binding)
            * Type returned by trackObject.getSoundEmitter()
            * @{
            */

            // TODO: adjust all signatures to type "void*" parameters if possible
            /** Stop a sound */
            void stop(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                ((TrackObjectPresentationSound*)memory)->stopSound();
            }

            /** Play the specified sound once */
            void playOnce(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                ((TrackObjectPresentationSound*)memory)->triggerSound(false); //false = once
            }

            /** Play the specified sound continuously */
            void playLoop(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                ((TrackObjectPresentationSound*)memory)->triggerSound(true); //true = loop
            }
            /** @} */
        }

        // ----------- ParticleEmitter Object methods -----------

        namespace ParticleEmitter
        {
            /**
            * @addtogroup Scripting_ParticleEmitter ParticleEmitter (script binding)
            * Type returned by trackObject.getParticleEmitter()
            * @{
            */

            // TODO: adjust all signatures to type "void*" parameters if possible
            /** Stop particle emission */
            void stop(/** \cond DOXYGEN_IGNORE */ void *memory /** \endcond */)
            {
                ((TrackObjectPresentationParticles*)memory)->stop();
            }

            /** Play the specified sound once */
            void setEmissionRate(float rate /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                ((TrackObjectPresentationParticles*)memory)->setRate(rate);
            }
            /** @} */
        }

        /** @}*/
        /** @}*/

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r; // of type asERetCodes

            engine->SetDefaultNamespace("Track");

            r = engine->RegisterObjectType("TrackObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("PhysicalObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("Mesh", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0); // TrackObjectPresentationMesh
            r = engine->RegisterObjectType("ParticleEmitter", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("SoundEmitter", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("Animator", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void disableTrackObject(const string &in)", asFUNCTION(disableTrackObject), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableTrackObject(const string &in)", asFUNCTION(enableTrackObject), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void enableTrigger(const string &in)", asFUNCTION(enableTrigger), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void disableTrigger(const string &in)", asFUNCTION(disableTrigger), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void createTrigger(const string &in, const Vec3 &in, float distance)",
                asFUNCTION(createTrigger), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("TrackObject@ getTrackObject(const string &in)", asFUNCTION(getTrackObject), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void exitRace()", asFUNCTION(exitRace), asCALL_CDECL); assert(r >= 0);

            // TrackObject
            r = engine->RegisterObjectMethod("TrackObject", "void setEnable(bool status)", asMETHOD(TrackObject, setEnable), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "SoundEmitter@ getSoundEmitter()", asMETHOD(TrackObject, getSoundEmitter), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "PhysicalObject@ getPhysics()", asMETHOD(TrackObject, getPhysics), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Mesh@ getMesh()", asMETHOD(TrackObject, getMesh), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "ParticleEmitter@ getParticleEmitter()", asMETHOD(TrackObject, getParticleEmitter), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Animator@ getIPOAnimator()", asMETHOD(TrackObject, getIPOAnimator), asCALL_THISCALL); assert(r >= 0);
            // TODO: add move method

            // PhysicalObject
            r = engine->RegisterObjectMethod("PhysicalObject", "bool isFlattenKartObject()", asMETHOD(PhysicalObject, isFlattenKartObject), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "void disable()", asMETHOD(PhysicalObject, disable), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "void enable()", asMETHOD(PhysicalObject, enable), asCALL_THISCALL); assert(r >= 0);

            // TrackObjectPresentationMesh (Mesh or Skeletal Animation)
            r = engine->RegisterObjectMethod("Mesh", "void setLoop(int start, int end)", asFUNCTION(Mesh::setLoop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "int getCurrentFrame()", asFUNCTION(Mesh::getCurrentFrame), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "void setCurrentFrame(int frame)", asFUNCTION(Mesh::setCurrentFrame), asCALL_CDECL_OBJLAST); assert(r >= 0);
            //r = engine->RegisterObjectMethod("Mesh", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);

            // Particle Emitter
            r = engine->RegisterObjectMethod("ParticleEmitter", "void stop()", asFUNCTION(ParticleEmitter::stop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("ParticleEmitter", "void setEmissionRate(float)", asFUNCTION(ParticleEmitter::setEmissionRate), asCALL_CDECL_OBJLAST); assert(r >= 0);

            // Sound Effect
            //r = engine->RegisterObjectMethod("SoundEmitter", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void stop()", asFUNCTION(SoundEmitter::stop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void playOnce()", asFUNCTION(SoundEmitter::playOnce), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void playLoop()", asFUNCTION(SoundEmitter::playLoop), asCALL_CDECL_OBJLAST); assert(r >= 0);


            // Curve based Animation
            //fails due to insufficient visibility to scripts TODO : Decide whether to fix visibility or introduce wrappers
            //r = engine->RegisterObjectMethod("Animator", "void setPaused(bool mode)", asMETHOD(ThreeDAnimation, setPaused), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("Animator", "void setPaused(bool mode)", asFUNCTION(Animator::setPaused), asCALL_CDECL_OBJLAST); assert(r >= 0);
            // TODO: add method to set current frame
            // TODO: add method to launch playback from frame X to frame Y
            // TODO: add method to register onAnimationComplete notifications ?
        }

/** \cond DOXYGEN_IGNORE */
    }
}
/** \endcond */

