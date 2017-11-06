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
#include "font/digit_face.hpp"
#include "font/font_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/stk_text_billboard.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "modes/world.hpp"
#include "scriptengine/property_animator.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "states_screens/dialogs/race_paused_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"

#include <IBillboardTextSceneNode.h>
#include <angelscript.h>
#include <assert.h>
#include <ISceneManager.h>

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
        ::TrackObject* getTrackObject(std::string* libraryInstance, std::string* objID)
        {
            return ::Track::getCurrentTrack()->getTrackObjectManager()
                          ->getTrackObject(*libraryInstance, *objID);
        }

        /** Creates a trigger at the specified location */
        void createTrigger(std::string* triggerID, SimpleVec3* creation_loc, float distance)
        {
            float x = creation_loc->getX();
            float y = creation_loc->getY();
            float z = creation_loc->getZ();
            core::vector3df posi(x, y, z);
            core::vector3df hpr(0, 0, 0);
            core::vector3df scale(1.0f, 1.0f, 1.0f);
            TrackObjectPresentationActionTrigger* newtrigger =
                new TrackObjectPresentationActionTrigger(posi, *triggerID, distance);
            ::TrackObject* tobj = new ::TrackObject(posi, hpr, scale,
                "none", newtrigger, false /* isDynamic */, NULL /* physics settings */);
            tobj->setID(*triggerID);
            ::Track::getCurrentTrack()->getTrackObjectManager()->insertObject(tobj);
        }

        void createTextBillboard(std::string* text, SimpleVec3* location)
        {
            core::stringw wtext = StringUtils::utf8ToWide(*text);
            DigitFace* digit_face = font_manager->getFont<DigitFace>();
            core::dimension2d<u32> textsize = digit_face->getDimension(wtext.c_str());

            core::vector3df xyz(location->getX(), location->getY(), location->getZ());
#ifndef SERVER_ONLY
            if (CVS->isGLSL())
            {
                STKTextBillboard* tb = new STKTextBillboard(wtext.c_str(), digit_face,
                    GUIEngine::getSkin()->getColor("font::bottom"),
                    GUIEngine::getSkin()->getColor("font::top"),
                    irr_driver->getSceneManager()->getRootSceneNode(),
                    irr_driver->getSceneManager(), -1, xyz,
                    core::vector3df(1.5f, 1.5f, 1.5f));

                ::Track::getCurrentTrack()->addNode(tb);
                tb->drop();
            }
            else
            {
                assert(GUIEngine::getHighresDigitFont() != NULL);
                scene::ISceneManager* sm = irr_driver->getSceneManager();
                scene::ISceneNode* sn =
                    sm->addBillboardTextSceneNode(GUIEngine::getHighresDigitFont(),
                        wtext.c_str(),
                        NULL,
                        core::dimension2df(textsize.Width / 35.0f,
                            textsize.Height / 35.0f),
                        xyz,
                        -1, // id
                        GUIEngine::getSkin()->getColor("font::bottom"),
                        GUIEngine::getSkin()->getColor("font::top"));
                ::Track::getCurrentTrack()->addNode(sn);
            }
#endif
        }

        /** Function for re-enable a trigger after a specific timeout*/
        void setTriggerReenableTimeout(std::string* triggerID, std::string* lib_id,
                                       float reenable_time)
        {
            ::TrackObject* tobj = ::Track::getCurrentTrack()->getTrackObjectManager()
                ->getTrackObject(*lib_id, *triggerID);
            if (tobj != NULL)
            {
                TrackObjectPresentationActionTrigger* topat =
                    tobj->getPresentation<TrackObjectPresentationActionTrigger>();
                if (topat != NULL)
                {
                    topat->setReenableTimeout(reenable_time);
                }
            }
        }

        /** Exits the race to the main menu */
        void exitRace()
        {
            World::getWorld()->scheduleExitRace();
        }

        void pauseRace()
        {
            new RacePausedDialog(0.8f, 0.6f);
        }

        int getNumberOfKarts()
        {
            return race_manager->getNumberOfKarts();
        }

        int getNumLocalPlayers()
        {
            return race_manager->getNumLocalPlayers();
        }

        bool isTrackReverse()
        {
            return race_manager->getReverseTrack();
        }

        int getMajorRaceMode()
        {
            return race_manager->getMajorMode();
        }

        int getMinorRaceMode()
        {
            return race_manager->getMinorMode();
        }

        bool isDuringDay()
        {
            return ::Track::getCurrentTrack()->getIsDuringDay();
        }

        void setFog(float maxDensity, float start, float end, int r, int g, int b, float duration)
        {
            PropertyAnimator* animator = PropertyAnimator::get();
            ::Track* track = ::Track::getCurrentTrack();
            animator->add(
                new AnimatedProperty(FOG_MAX, 1,
                    new double[1] { track->getFogMax() }, 
                    new double[1] { maxDensity }, duration, track)
            );
            animator->add(
                new AnimatedProperty(FOG_RANGE, 2,
                    new double[2] { track->getFogStart(), track->getFogEnd() },
                    new double[2] { start, end }, duration, track)
            );

            video::SColor color = track->getFogColor();
            animator->add(
                new AnimatedProperty(FOG_COLOR, 3,
                    new double[3] { 
                        (double)color.getRed(), 
                        (double)color.getGreen(), 
                        (double)color.getBlue() 
                    },
                    new double[3] { 
                        (double)r,
                        (double)g,
                        (double)b
                    }, 
                    duration, track)
            );
        }
    }

    /** \cond DOXYGEN_IGNORE */
    namespace Track
    {
    /** \endcond */

        namespace TrackObject
        {
            SimpleVec3 getCenterPosition(::TrackObject* obj)
            {
                core::vector3df pos = obj->getAbsoluteCenterPosition();
                return SimpleVec3(pos.X, pos.Y, pos.Z);
            }

            SimpleVec3 getOrigin(::TrackObject* obj)
            {
                core::vector3df pos = obj->getAbsolutePosition();
                return SimpleVec3(pos.X, pos.Y, pos.Z);
            }
        }

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
            void setFrameLoop(int start, int end /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                if (memory)
                {
                    ((scene::IAnimatedMeshSceneNode*)(memory))->setFrameLoop(start, end);
                }
            }

            /** Sets a loop once for a skeletal animation */
            void setFrameLoopOnce(int start, int end /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                if (memory)
                {
                    ((scene::IAnimatedMeshSceneNode*)(memory))->setFrameLoopOnce(start, end);
                }
            }

            /** Get current frame in a skeletal animation */
            int getFrameNr(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                if (memory)
                {
                    return (int)((scene::IAnimatedMeshSceneNode*)(memory))->getFrameNr();
                }
                return -1;
            }

            /** Gets the animation set for a skeletal animation */
            int getAnimationSet(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                if (memory)
                {
                    return ((scene::IAnimatedMeshSceneNode*)(memory))->getAnimationSet();
                }
                return -1;
            }

            /** Sets the current frame for a skeletal animation */
            void setCurrentFrame(int frame /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                if (memory)
                {
                    ((scene::IAnimatedMeshSceneNode*)(memory))->setCurrentFrame((float)frame);
                }
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

        // ----------- Light Object methods -----------

        namespace Light
        {
            /**
            * @addtogroup Scripting_Light Light (script binding)
            * Type returned by trackObject.getLight()
            * @{
            */

            void setEnergy(float energy, /** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                ((TrackObjectPresentationLight*)memory)->setEnergy(energy);
            }

            void animateEnergy(float energy, float duration, /** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                TrackObjectPresentationLight* light = ((TrackObjectPresentationLight*)memory);
                PropertyAnimator::get()->add(
                    new AnimatedProperty(AP_LIGHT_ENERGY, 1,
                    new double[1] { light->getEnergy() },
                    new double[1] { energy }, duration, light)
                );
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

            /** Stop particle emission */
            void stop(/** \cond DOXYGEN_IGNORE */ void *memory /** \endcond */)
            {
                ((TrackObjectPresentationParticles*)memory)->stop();
            }

            /** Stop particle emission */
            void stopIn(float delay, /** \cond DOXYGEN_IGNORE */ void *memory /** \endcond */)
            {
                ((TrackObjectPresentationParticles*)memory)->stopIn(delay);
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
            r = engine->RegisterObjectType("Light", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

            //r = engine->RegisterGlobalFunction("void disableTrackObject(const string &in)", asFUNCTION(disableTrackObject), asCALL_CDECL); assert(r >= 0);
            //r = engine->RegisterGlobalFunction("void enableTrackObject(const string &in)", asFUNCTION(enableTrackObject), asCALL_CDECL); assert(r >= 0);
            //r = engine->RegisterGlobalFunction("void enableTrigger(const string &in)", asFUNCTION(enableTrigger), asCALL_CDECL); assert(r >= 0);
            //r = engine->RegisterGlobalFunction("void disableTrigger(const string &in)", asFUNCTION(disableTrigger), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void createTrigger(const string &in, const Vec3 &in, float distance)",
                asFUNCTION(createTrigger), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void createTextBillboard(const string &in, const Vec3 &in)",
                asFUNCTION(createTextBillboard), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void setTriggerReenableTimeout(const string &in, const string &in, float reenable_time)",
                asFUNCTION(setTriggerReenableTimeout), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("TrackObject@ getTrackObject(const string &in, const string &in)", asFUNCTION(getTrackObject), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void exitRace()", asFUNCTION(exitRace), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void pauseRace()", asFUNCTION(pauseRace), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void setFog(float maxDensity, float start, float end, int r, int g, int b, float duration)", asFUNCTION(setFog), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getNumberOfKarts()", asFUNCTION(getNumberOfKarts), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getNumLocalPlayers()", asFUNCTION(getNumLocalPlayers), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("bool isReverse()", asFUNCTION(isTrackReverse), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getMajorRaceMode()", asFUNCTION(getMajorRaceMode), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("int getMinorRaceMode()", asFUNCTION(getMinorRaceMode), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("bool isDuringDay()", asFUNCTION(isDuringDay), asCALL_CDECL); assert(r >= 0);

            // TrackObject
            r = engine->RegisterObjectMethod("TrackObject", "void setEnabled(bool status)", asMETHOD(::TrackObject, setEnabled), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "SoundEmitter@ getSoundEmitter()", asMETHOD(::TrackObject, getSoundEmitter), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Light@ getLight()", asMETHOD(::TrackObject, getLight), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "PhysicalObject@ getPhysics()", asMETHOD(::TrackObject, getPhysics), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Mesh@ getMesh()", asMETHOD(::TrackObject, getMesh), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "ParticleEmitter@ getParticleEmitter()", asMETHOD(::TrackObject, getParticleEmitter), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Animator@ getIPOAnimator()", asMETHOD(::TrackObject, getIPOAnimator), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "void moveTo(const Vec3 &in, bool)", asMETHOD(::TrackObject, moveTo), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Vec3 getCenterPosition()", asFUNCTION(TrackObject::getCenterPosition), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("TrackObject", "Vec3 getOrigin()", asFUNCTION(TrackObject::getOrigin), asCALL_CDECL_OBJLAST); assert(r >= 0);

            // PhysicalObject
            r = engine->RegisterObjectMethod("PhysicalObject", "bool isFlattenKartObject()", asMETHOD(PhysicalObject, isFlattenKartObject), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "void disable()", asMETHOD(PhysicalObject, disable), asCALL_THISCALL); assert(r >= 0);
            r = engine->RegisterObjectMethod("PhysicalObject", "void enable()", asMETHOD(PhysicalObject, enable), asCALL_THISCALL); assert(r >= 0);

            // Animated Mesh
            r = engine->RegisterObjectMethod("Mesh", "void setFrameLoop(int start, int end)", asFUNCTION(Mesh::setFrameLoop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "void setFrameLoopOnce(int start, int end)", asFUNCTION(Mesh::setFrameLoopOnce), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "int getFrameNr()", asFUNCTION(Mesh::getFrameNr), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "int getAnimationSet()", asFUNCTION(Mesh::getAnimationSet), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Mesh", "void setCurrentFrame(int frame)", asFUNCTION(Mesh::setCurrentFrame), asCALL_CDECL_OBJLAST); assert(r >= 0);
            //r = engine->RegisterObjectMethod("Mesh", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);

            // Particle Emitter
            r = engine->RegisterObjectMethod("ParticleEmitter", "void stop()", asFUNCTION(ParticleEmitter::stop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("ParticleEmitter", "void stopIn(float)", asFUNCTION(ParticleEmitter::stopIn), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("ParticleEmitter", "void setEmissionRate(float)", asFUNCTION(ParticleEmitter::setEmissionRate), asCALL_CDECL_OBJLAST); assert(r >= 0);

            // Sound Effect
            //r = engine->RegisterObjectMethod("SoundEmitter", "void move(Vec3 &in)", asFUNCTION(movePresentation), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void stop()", asFUNCTION(SoundEmitter::stop), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void playOnce()", asFUNCTION(SoundEmitter::playOnce), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("SoundEmitter", "void playLoop()", asFUNCTION(SoundEmitter::playLoop), asCALL_CDECL_OBJLAST); assert(r >= 0);

            // Light
            r = engine->RegisterObjectMethod("Light", "void setEnergy(float)", asFUNCTION(Light::setEnergy), asCALL_CDECL_OBJLAST); assert(r >= 0);
            r = engine->RegisterObjectMethod("Light", "void animateEnergy(float, float)", asFUNCTION(Light::animateEnergy), asCALL_CDECL_OBJLAST); assert(r >= 0);

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

