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
#include "config/user_config.hpp"
#include "font/digit_face.hpp"
#include "font/font_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_text_billboard.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "scriptengine/property_animator.hpp"
#include "scriptengine/aswrappedcall.hpp"
#include "scriptengine/scriptarray.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "states_screens/dialogs/race_paused_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/string_utils.hpp"

#include <IAnimatedMeshSceneNode.h>
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

        CScriptArray* getTrackObjectList()
        {
            std::vector<TrackObject*>& tl = ::Track::getCurrentTrack()
                ->getTrackObjectManager()->getObjects().m_contents_vector;
            asIScriptContext* ctx = asGetActiveContext();
            asIScriptEngine* engine = ctx->GetEngine();
            asITypeInfo* t = engine->GetTypeInfoByDecl("array<Track::TrackObject@>");
            CScriptArray* script_array = CScriptArray::Create(t, tl.size());
            for (unsigned int i = 0; i < tl.size(); ++i)
                script_array->SetValue(i, &tl[i]);

            return script_array;
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
            core::vector3df xyz(location->getX(), location->getY(), location->getZ());
#ifndef SERVER_ONLY
            STKTextBillboard* tb = new STKTextBillboard(
                GUIEngine::getSkin()->getColor("font::bottom"),
                GUIEngine::getSkin()->getColor("font::top"),
                irr_driver->getSceneManager()->getRootSceneNode(),
                irr_driver->getSceneManager(), -1, xyz,
                core::vector3df(1.5f, 1.5f, 1.5f));
            if (CVS->isGLSL())
                tb->init(wtext.c_str(), digit_face);
            else
                tb->initLegacy(wtext.c_str(), digit_face);
            ::Track::getCurrentTrack()->addNode(tb);
            tb->drop();
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
            return RaceManager::get()->getNumberOfKarts();
        }

        int getNumLocalPlayers()
        {
            return RaceManager::get()->getNumLocalPlayers();
        }
        
        /**
          * Gets the kart type, such as local player, networked player, AI, etc.
          * @return A KartType enum as defined in race_manager.hpp, implicitly casted to an int
          */
        int getKartType(int kartId)
        {
            return RaceManager::get()->getKartType(kartId);
        }
        
        bool isTrackReverse()
        {
            return RaceManager::get()->getReverseTrack();
        }
        
        /**
          * Gets the difficulty setting for this race.
          * @return A Difficulty enum as defined in race_manager.hpp, implicitly casted to an int
          */
        int getDifficulty()
        {
            return RaceManager::get()->getDifficulty();
        }

        int getMajorRaceMode()
        {
            return RaceManager::get()->getMajorMode();
        }

        int getMinorRaceMode()
        {
            return RaceManager::get()->getMinorMode();
        }

        int getGeometryLevel()
        {
            return UserConfigParams::m_geometry_level;
        }

        bool isDuringDay()
        {
            return ::Track::getCurrentTrack()->getIsDuringDay();
        }

        uint32_t getItemManagerRandomSeed()
        {
            return ItemManager::getRandomSeed();
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

            /** Gets the animation set id for a skeletal animation */
            int getAnimationSet(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                if (memory)
                {
                    return ((scene::IAnimatedMeshSceneNode*)(memory))->getAnimationSet();
                }
                return -1;
            }

            /** Gets the animation set frames for a skeletal animation */
            CScriptArray* getAnimationSetFrames(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                asIScriptContext* ctx = asGetActiveContext();
                asIScriptEngine* engine = ctx->GetEngine();
                asITypeInfo* t = engine->GetTypeInfoByDecl("array<uint>");
                if (memory)
                {
                    scene::IAnimatedMeshSceneNode* node =
                        ((scene::IAnimatedMeshSceneNode*)(memory));
                    core::array<u32>& f = node->getAnimationSetFrames();
                    CScriptArray* script_array = CScriptArray::Create(t, f.size());
                    for (unsigned int i = 0; i < f.size(); ++i)
                        script_array->SetValue(i, &f[i]);

                    return script_array;
                }
                return CScriptArray::Create(t, (unsigned)0);
            }
            /** Gets the animation set count for a skeletal animation */
            int getAnimationSetNum(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                if (memory)
                {
                    return ((scene::IAnimatedMeshSceneNode*)(memory))->getAnimationSetNum();
                }
                return -1;
            }

            /** Remove all animation set for a skeletal animation */
            void removeAllAnimationSet(/** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                if (memory)
                {
                    ((scene::IAnimatedMeshSceneNode*)(memory))->removeAllAnimationSet();
                }
            }

            /** Add an animation set for a skeletal animation */
            void addAnimationSet(int start/** \cond DOXYGEN_IGNORE */, int end/** \cond DOXYGEN_IGNORE */, /** \cond DOXYGEN_IGNORE */void *memory /** \endcond */)
            {
                if (memory)
                {
                    ((scene::IAnimatedMeshSceneNode*)(memory))->addAnimationSet(start, end);
                }
            }

            /** use an current frame for a skeletal animation */
            void useAnimationSet(int set_num /** \cond DOXYGEN_IGNORE */, void *memory /** \endcond */)
            {
                if (memory)
                {
                    ((scene::IAnimatedMeshSceneNode*)(memory))->useAnimationSet(set_num);
                }
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
            engine->SetDefaultNamespace("Track");
            
            bool mp = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
            asDWORD call_conv = mp ? asCALL_GENERIC : asCALL_CDECL;
            asDWORD call_conv_objlast = mp ? asCALL_GENERIC : asCALL_CDECL_OBJLAST;
            asDWORD call_conv_thiscall = mp ? asCALL_GENERIC : asCALL_THISCALL;
            int r; // of type asERetCodes

            r = engine->RegisterObjectType("TrackObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("PhysicalObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("Mesh", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0); // TrackObjectPresentationMesh
            r = engine->RegisterObjectType("ParticleEmitter", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("SoundEmitter", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("Animator", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
            r = engine->RegisterObjectType("Light", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

            //r = engine->RegisterGlobalFunction("void disableTrackObject(const string &in)", 
            //                                   mp ? WRAP_FN(disableTrackObject) : asFUNCTION(disableTrackObject), 
            //                                   call_conv); assert(r >= 0);
            
            //r = engine->RegisterGlobalFunction("void enableTrackObject(const string &in)", 
            //                                   mp ? WRAP_FN(enableTrackObject) : asFUNCTION(enableTrackObject), 
            //                                   call_conv); assert(r >= 0);
            
            //r = engine->RegisterGlobalFunction("void enableTrigger(const string &in)", 
            //                                   mp ? WRAP_FN(enableTrigger) : asFUNCTION(enableTrigger), 
            //                                   call_conv); assert(r >= 0);
            
            //r = engine->RegisterGlobalFunction("void disableTrigger(const string &in)", 
            //                                   mp ? WRAP_FN(disableTrigger) : asFUNCTION(disableTrigger), 
            //                                   call_conv); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("void createTrigger(const string &in, const Vec3 &in, float distance)",
                                               mp ? WRAP_FN(createTrigger) : asFUNCTION(createTrigger), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void createTextBillboard(const string &in, const Vec3 &in)",
                                               mp ? WRAP_FN(createTextBillboard) : asFUNCTION(createTextBillboard), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void setTriggerReenableTimeout(const string &in, const string &in, float reenable_time)",
                                               mp ? WRAP_FN(setTriggerReenableTimeout) : asFUNCTION(setTriggerReenableTimeout), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("TrackObject@ getTrackObject(const string &in, const string &in)", 
                                               mp ? WRAP_FN(getTrackObject) : asFUNCTION(getTrackObject), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("array<TrackObject@>@ getTrackObjectList()",
                                               mp ? WRAP_FN(getTrackObjectList) : asFUNCTION(getTrackObjectList),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void exitRace()", 
                                               mp ? WRAP_FN(exitRace) : asFUNCTION(exitRace), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void pauseRace()", 
                                               mp ? WRAP_FN(pauseRace) : asFUNCTION(pauseRace), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void setFog(float maxDensity, float start, float end, int r, int g, int b, float duration)", 
                                               mp ? WRAP_FN(setFog) : asFUNCTION(setFog), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("int getNumberOfKarts()", 
                                               mp ? WRAP_FN(getNumberOfKarts) : asFUNCTION(getNumberOfKarts), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("int getNumLocalPlayers()", 
                                               mp ? WRAP_FN(getNumLocalPlayers) : asFUNCTION(getNumLocalPlayers), 
                                               call_conv); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("int getKartType(int kartId)", 
                                               mp ? WRAP_FN(getKartType) : asFUNCTION(getKartType), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("bool isReverse()", 
                                               mp ? WRAP_FN(isTrackReverse) : asFUNCTION(isTrackReverse), 
                                               call_conv); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("int getDifficulty()", 
                                               mp ? WRAP_FN(getDifficulty) : asFUNCTION(getDifficulty), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("int getMajorRaceMode()", 
                                               mp ? WRAP_FN(getMajorRaceMode) : asFUNCTION(getMajorRaceMode), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("uint getItemManagerRandomSeed()",
                                               mp ? WRAP_FN(getItemManagerRandomSeed) : asFUNCTION(getItemManagerRandomSeed),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("int getMinorRaceMode()", 
                                               mp ? WRAP_FN(getMinorRaceMode) : asFUNCTION(getMinorRaceMode), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("int getGeometryLevel()",
                                               mp ? WRAP_FN(getGeometryLevel) : asFUNCTION(getGeometryLevel),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("bool isDuringDay()", 
                                               mp ? WRAP_FN(isDuringDay) : asFUNCTION(isDuringDay), 
                                               call_conv); assert(r >= 0);

            // TrackObject
            r = engine->RegisterObjectMethod("TrackObject", "void setEnabled(bool status)", 
                                             mp ? WRAP_MFN(::TrackObject, setEnabled) : asMETHOD(::TrackObject, setEnabled), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "SoundEmitter@ getSoundEmitter()", 
                                             mp ? WRAP_MFN(::TrackObject, getSoundEmitter) : asMETHOD(::TrackObject, getSoundEmitter), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "Light@ getLight()", 
                                             mp ? WRAP_MFN(::TrackObject, getLight) : asMETHOD(::TrackObject, getLight), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "PhysicalObject@ getPhysics()", 
                                             mp ? WRAP_MFN(::TrackObject, getPhysics): asMETHOD(::TrackObject, getPhysics), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "Mesh@ getMesh()", 
                                             mp ? WRAP_MFN(::TrackObject, getMesh) : asMETHOD(::TrackObject, getMesh), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "ParticleEmitter@ getParticleEmitter()", 
                                             mp ? WRAP_MFN(::TrackObject, getParticleEmitter) : asMETHOD(::TrackObject, getParticleEmitter), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "Animator@ getIPOAnimator()", 
                                             mp ? WRAP_MFN(::TrackObject, getIPOAnimator): asMETHOD(::TrackObject, getIPOAnimator), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "void moveTo(const Vec3 &in, bool)", 
                                             mp ? WRAP_MFN(::TrackObject, moveTo) : asMETHOD(::TrackObject, moveTo), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "void reset()", 
                                             mp ? WRAP_MFN(::TrackObject, reset) : asMETHOD(::TrackObject, reset), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "Vec3 getCenterPosition()", 
                                             mp ? WRAP_OBJ_LAST(TrackObject::getCenterPosition): asFUNCTION(TrackObject::getCenterPosition), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "Vec3 getOrigin()", 
                                             mp ? WRAP_OBJ_LAST(TrackObject::getOrigin) : asFUNCTION(TrackObject::getOrigin), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "TrackObject@ getParentLibrary()", 
                                             mp ? WRAP_MFN(::TrackObject, getParentLibrary): asMETHOD(::TrackObject, getParentLibrary), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("TrackObject", "string getName()", 
                                             mp ? WRAP_MFN(::TrackObject, getName) : asMETHOD(::TrackObject, getName), 
                                             call_conv_thiscall); assert(r >= 0);

            r = engine->RegisterObjectMethod("TrackObject", "string getID()",
                                             mp ? WRAP_MFN(::TrackObject, getID) : asMETHOD(::TrackObject, getID),
                                             call_conv_thiscall); assert(r >= 0);

            // PhysicalObject
            r = engine->RegisterObjectMethod("PhysicalObject", "bool isFlattenKartObject()", 
                                             mp ? WRAP_MFN(PhysicalObject, isFlattenKartObject) : asMETHOD(PhysicalObject, 
                                             isFlattenKartObject), call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("PhysicalObject", "void disable()", 
                                             mp ? WRAP_MFN(PhysicalObject, disable) : asMETHOD(PhysicalObject, disable), 
                                             call_conv_thiscall); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("PhysicalObject", "void enable()", 
                                             mp ? WRAP_MFN(PhysicalObject, enable) : asMETHOD(PhysicalObject, enable), 
                                             call_conv_thiscall); assert(r >= 0);

            // Animated Mesh
            r = engine->RegisterObjectMethod("Mesh", "void setFrameLoop(int start, int end)", 
                                             mp ? WRAP_OBJ_LAST(Mesh::setFrameLoop) : asFUNCTION(Mesh::setFrameLoop), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Mesh", "void setFrameLoopOnce(int start, int end)", 
                                             mp ? WRAP_OBJ_LAST(Mesh::setFrameLoopOnce) : asFUNCTION(Mesh::setFrameLoopOnce), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Mesh", "int getFrameNr()", 
                                             mp ? WRAP_OBJ_LAST(Mesh::getFrameNr) : asFUNCTION(Mesh::getFrameNr), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Mesh", "int getAnimationSet()", 
                                             mp ? WRAP_OBJ_LAST(Mesh::getAnimationSet) : asFUNCTION(Mesh::getAnimationSet), 
                                             call_conv_objlast); assert(r >= 0);

            r = engine->RegisterObjectMethod("Mesh", "array<uint>@ getAnimationSetFrames()",
                                             mp ? WRAP_OBJ_LAST(Mesh::getAnimationSetFrames) : asFUNCTION(Mesh::getAnimationSetFrames),
                                             call_conv_objlast); assert(r >= 0);

            r = engine->RegisterObjectMethod("Mesh", "int getAnimationSetNum()",
                                             mp ? WRAP_OBJ_LAST(Mesh::getAnimationSetNum) : asFUNCTION(Mesh::getAnimationSetNum),
                                             call_conv_objlast); assert(r >= 0);

            r = engine->RegisterObjectMethod("Mesh", "void useAnimationSet(int set_num)", 
                                             mp ? WRAP_OBJ_LAST(Mesh::useAnimationSet) : asFUNCTION(Mesh::useAnimationSet), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Mesh", "void addAnimationSet(int start, int end)", 
                                             mp ? WRAP_OBJ_LAST(Mesh::addAnimationSet) : asFUNCTION(Mesh::addAnimationSet), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Mesh", "void removeAllAnimationSet()", 
                                             mp ? WRAP_OBJ_LAST(Mesh::removeAllAnimationSet) : asFUNCTION(Mesh::removeAllAnimationSet), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Mesh", "void setCurrentFrame(int frame)", 
                                             mp ? WRAP_OBJ_LAST(Mesh::setCurrentFrame) : asFUNCTION(Mesh::setCurrentFrame), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            //r = engine->RegisterObjectMethod("Mesh", "void move(Vec3 &in)", 
            //                                 mp ? WRAP_OBJ_LAST(movePresentation) : asFUNCTION(movePresentation), 
            //                                 call_conv_objlast); assert(r >= 0);

            // Particle Emitter
            r = engine->RegisterObjectMethod("ParticleEmitter", "void stop()", 
                                             mp ? WRAP_OBJ_LAST(ParticleEmitter::stop) : asFUNCTION(ParticleEmitter::stop), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("ParticleEmitter", "void stopIn(float)", 
                                             mp ? WRAP_OBJ_LAST(ParticleEmitter::stopIn) : asFUNCTION(ParticleEmitter::stopIn), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("ParticleEmitter", "void setEmissionRate(float)", 
                                             mp ? WRAP_OBJ_LAST(ParticleEmitter::setEmissionRate) : asFUNCTION(ParticleEmitter::setEmissionRate), 
                                             call_conv_objlast); assert(r >= 0);

            // Sound Effect
            //r = engine->RegisterObjectMethod("SoundEmitter", "void move(Vec3 &in)", 
            //                                 mp ? WRAP_OBJ_LAST(movePresentation) : asFUNCTION(movePresentation), 
            //                                 call_conv_objlast); assert(r >= 0);
            
            r = engine->RegisterObjectMethod("SoundEmitter", "void stop()", 
                                             mp ? WRAP_OBJ_LAST(SoundEmitter::stop) : asFUNCTION(SoundEmitter::stop), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("SoundEmitter", "void playOnce()", 
                                             mp ? WRAP_OBJ_LAST(SoundEmitter::playOnce) : asFUNCTION(SoundEmitter::playOnce), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("SoundEmitter", "void playLoop()", 
                                             mp ? WRAP_OBJ_LAST(SoundEmitter::playLoop) : asFUNCTION(SoundEmitter::playLoop), 
                                             call_conv_objlast); assert(r >= 0);

            // Light
            r = engine->RegisterObjectMethod("Light", "void setEnergy(float)", 
                                             mp ? WRAP_OBJ_LAST(Light::setEnergy) : asFUNCTION(Light::setEnergy), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            r = engine->RegisterObjectMethod("Light", "void animateEnergy(float, float)", 
                                             mp ? WRAP_OBJ_LAST(Light::animateEnergy) : asFUNCTION(Light::animateEnergy), 
                                             call_conv_objlast); assert(r >= 0);

            // Curve based Animation
            //fails due to insufficient visibility to scripts TODO : Decide whether to fix visibility or introduce wrappers
            //r = engine->RegisterObjectMethod("Animator", "void setPaused(bool mode)", 
            //                                 mp ? WRAP_MFN(ThreeDAnimation, setPaused) : asMETHOD(ThreeDAnimation, setPaused), 
            //                                 call_conv_thiscall); assert(r >= 0);
            
            r = engine->RegisterObjectMethod("Animator", "void setPaused(bool mode)", 
                                             mp ? WRAP_OBJ_LAST(Animator::setPaused) : asFUNCTION(Animator::setPaused), 
                                             call_conv_objlast); assert(r >= 0);
                                             
            // TODO: add method to set current frame
            // TODO: add method to launch playback from frame X to frame Y
            // TODO: add method to register onAnimationComplete notifications ?
        }

/** \cond DOXYGEN_IGNORE */
    }
}
/** \endcond */

