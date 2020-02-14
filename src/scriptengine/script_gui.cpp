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

#include "config/user_config.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "modes/world.hpp"
#include "scriptengine/aswrappedcall.hpp"
#include "scriptengine/script_track.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "guiengine/message_queue.hpp"

#include <angelscript.h>
#include "scriptarray.hpp"

#include <assert.h>
#include <iostream> //debug

/** \cond DOXYGEN_IGNORE */
namespace Scripting
{
/** \endcond */

    namespace GUI
    {
        enum RaceGUIType
        {
            RGT_KEYBOARD_GAMEPAD = 0,
            RGT_STEERING_WHEEL = 1,
            RGT_ACCELEROMETER = 2,
            RGT_GYROSCOPE = 3,
        };

        enum MsgType
        {

            MSG_FRIEND = 0,
            MSG_ACHIEVEMENT = 1,
            MSG_GENERIC = 2,
            MSG_ERROR = 3
        };

        /** \addtogroup Scripting
        * @{
        */
        /** \addtogroup GUI
        * @{
        */

        /** Get the key bound to a player action (enum GUI::PlayerAction)*/
        std::string getKeyBinding(int Enum_value)
        {
            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
            DeviceConfig* config = device->getConfiguration();
            PlayerAction ScriptAction = (PlayerAction)Enum_value;
            irr::core::stringw control = config->getBindingAsString(ScriptAction);
            std::string key = StringUtils::wideToUtf8(control);
            return key;
        }

        /** Show the specified message in a popup */
        void displayModalMessage(std::string* input)
        {
            irr::core::stringw out = StringUtils::utf8ToWide(*input);
            new TutorialMessageDialog((out), true);
        }

        /** Display a Message using MessageQueue (enum GUI::MsgType)*/
        void displayMessage(std::string* input, int Enum_value)
        {
            irr::core::stringw msg = StringUtils::utf8ToWide(*input);
            MsgType msg_type = (MsgType)Enum_value;
            MessageQueue::MessageType type;
            switch (msg_type)
            {
                case MSG_ERROR:
                    type = MessageQueue::MT_ERROR;
                    break;
                case MSG_FRIEND:
                    type = MessageQueue::MT_FRIEND;
                    break;
                case MSG_ACHIEVEMENT:
                    type = MessageQueue::MT_ACHIEVEMENT;
                    break;
                default:
                    type = MessageQueue::MT_GENERIC;
                    break;
            }
            MessageQueue::add(type, msg);
        }

        /** Displays an static Message. (enum GUI::MsgType)
         *  This Message has to be discarded by discardStaticMessage() manually.
         *  Otherwise it can be overridden.
         */
        void displayStaticMessage(std::string* input, int Enum_value)
        {
            irr::core::stringw msg = StringUtils::utf8ToWide(*input);
            MsgType msg_type = (MsgType)Enum_value;
            MessageQueue::MessageType type;
            switch (msg_type)
            {
                case MSG_ERROR:
                    type = MessageQueue::MT_ERROR;
                    break;
                case MSG_FRIEND:
                    type = MessageQueue::MT_FRIEND;
                    break;
                case MSG_ACHIEVEMENT:
                    type = MessageQueue::MT_ACHIEVEMENT;
                    break;
                default:
                    type = MessageQueue::MT_GENERIC;
                    break;
            }
            MessageQueue::addStatic(type, msg);
        }

        void discardStaticMessage()
        {
            MessageQueue::discardStatic();
        }

        void clearOverlayMessages()
        {
            if (World::getWorld()->getRaceGUI())
                World::getWorld()->getRaceGUI()->clearAllMessages();
        }

        /** Display text in the center of the screen for a few seconds */
        void displayOverlayMessage(std::string* input)
        {
            if (!World::getWorld()->getRaceGUI())
                return;
            irr::core::stringw msg = StringUtils::utf8ToWide(*input);
            std::vector<core::stringw> parts =
                StringUtils::split(msg, '\n', false);
            for (unsigned int n = 0; n < parts.size(); n++)
            {
                World::getWorld()->getRaceGUI()
                                    ->addMessage(parts[n], NULL, 4.0f,
                                                video::SColor(255, 255,255,255),
                                                true, true);
            }   // for n<parts.size()
        }

        /** Get translated version of string */
        std::string translate(std::string* input)
        {
            irr::core::stringw out = translations->w_gettext(input->c_str());

            return StringUtils::wideToUtf8(out);
        }

        /** Translate string and insert values. e.g. GUI::translate("Hello %s !", "John") */
        std::string translate(std::string* formatString, std::string* arg1)
        {
            irr::core::stringw out = translations->w_gettext(formatString->c_str());

            out = StringUtils::insertValues(out,
                                            StringUtils::utf8ToWide(*arg1));

            return StringUtils::wideToUtf8(out);
        }

        /** Translate string and insert values. e.g. GUI::translate("Hello %s !", "John") */
        std::string translate(std::string* formatString, std::string* arg1, std::string* arg2)
        {
            irr::core::stringw out = translations->w_gettext(formatString->c_str());

            out = StringUtils::insertValues(out,
                                            StringUtils::utf8ToWide(*arg1),
                                            StringUtils::utf8ToWide(*arg2));

            return StringUtils::wideToUtf8(out);
        }

        /** Translate string and insert values. e.g. GUI::translate("Hello %s !", "John") */
        std::string translate(std::string* formatString, std::string* arg1, std::string* arg2,
            std::string* arg3)
        {
            irr::core::stringw out = translations->w_gettext(formatString->c_str());

            out = StringUtils::insertValues(out,
                                            StringUtils::utf8ToWide(*arg1),
                                            StringUtils::utf8ToWide(*arg2),
                                            StringUtils::utf8ToWide(*arg3));

            return StringUtils::wideToUtf8(out);
        }
        /** @}*/
        /** @}*/

        // UNDOCUMENTED PROXIES : Use proxies to have different signatures, then redirect to the
        // documented function whose name is exposed in angelscript (these proxies exist so that
        // angelscript can properly resolve overloads, but doxygen can still generate the right docs
        /** \cond DOXYGEN_IGNORE */
        void proxy_displayMessage(std::string* msgString)
        {
            return displayMessage(msgString, MSG_GENERIC);
        }

        void proxy_displayMessageAndInsertValues1(std::string* msgString, int msgType)
        {
            return displayMessage(msgString, msgType);
        }

        void proxy_displayStaticMessage(std::string* msgString)
        {
            return displayStaticMessage(msgString, MSG_GENERIC);
        }

        void proxy_displayStaticMessageAndInsertValues1(std::string* msgString, int msgType)
        {
            return displayStaticMessage(msgString, msgType);
        }

        void proxy_discardStaticMessage()
        {
            return discardStaticMessage();
        }

        std::string proxy_translate(std::string* formatString)
        {
            return translate(formatString);
        }

        std::string proxy_translateAndInsertValues1(std::string* formatString, std::string* arg1)
        {
            return translate(formatString, arg1);
        }

        std::string proxy_translateAndInsertValues2(std::string* formatString, std::string* arg1, std::string* arg2)
        {
            return translate(formatString, arg1, arg2);
        }

        std::string proxy_translateAndInsertValues3(std::string* formatString, std::string* arg1, std::string* arg2,
            std::string* arg3)
        {
            return translate(formatString, arg1, arg2, arg3);
        }

        RaceGUIType getRaceGUIType()
        {
            if (UserConfigParams::m_multitouch_draw_gui)
            {
                if (UserConfigParams::m_multitouch_controls == 1)
                    return RGT_STEERING_WHEEL;
                else if (UserConfigParams::m_multitouch_controls == 2)
                    return RGT_ACCELEROMETER;
                else if (UserConfigParams::m_multitouch_controls == 3)
                    return RGT_GYROSCOPE;
            }
            return RGT_KEYBOARD_GAMEPAD;
        }
        /** \endcond */
        
        void registerScriptFunctions(asIScriptEngine *engine)
        {
            engine->SetDefaultNamespace("GUI");
            
            bool mp = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
            asDWORD call_conv = mp ? asCALL_GENERIC : asCALL_CDECL;
            int r; // of type asERetCodes

            r = engine->RegisterGlobalFunction("void displayMessage(const string &in)",
                                               mp ? WRAP_FN(proxy_displayMessage) : asFUNCTION(proxy_displayMessage),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void displayMessage(const string &in, int MessageType)",
                                               mp ? WRAP_FN(proxy_displayMessageAndInsertValues1)
                                                  : asFUNCTION(proxy_displayMessageAndInsertValues1),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void displayStaticMessage(const string &in)",
                                               mp ? WRAP_FN(proxy_displayStaticMessage) : asFUNCTION(proxy_displayStaticMessage),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void displayStaticMessage(const string &in, int MessageType)",
                                               mp ? WRAP_FN(proxy_displayStaticMessageAndInsertValues1)
                                                  : asFUNCTION(proxy_displayStaticMessageAndInsertValues1),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void discardStaticMessage()",
                                               mp ? WRAP_FN(discardStaticMessage) : asFUNCTION(discardStaticMessage),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void displayModalMessage(const string &in)",
                                               mp ? WRAP_FN(displayModalMessage) : asFUNCTION(displayModalMessage),
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void displayOverlayMessage(const string &in)", 
                                               mp ? WRAP_FN(displayOverlayMessage) : asFUNCTION(displayOverlayMessage), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void clearOverlayMessages()", 
                                               mp ? WRAP_FN(clearOverlayMessages) : asFUNCTION(clearOverlayMessages), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("RaceGUIType getRaceGUIType()",
                                               mp ? WRAP_FN(getRaceGUIType) : asFUNCTION(getRaceGUIType),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("string getKeyBinding(int input)", 
                                               mp ? WRAP_FN(getKeyBinding) : asFUNCTION(getKeyBinding), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string translate(const string &in)", 
                                               mp ? WRAP_FN(proxy_translate) : asFUNCTION(proxy_translate), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_translateAndInsertValues1) 
                                                  : asFUNCTION(proxy_translateAndInsertValues1), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_translateAndInsertValues2) 
                                                  : asFUNCTION(proxy_translateAndInsertValues2), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in, const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_translateAndInsertValues3) 
                                                  : asFUNCTION(proxy_translateAndInsertValues3), 
                                               call_conv); assert(r >= 0);

        }

        void registerScriptEnums(asIScriptEngine *engine)
        {
            // TODO: document enum in doxygen-generated scripting docs
            engine->SetDefaultNamespace("GUI");
            engine->RegisterEnum("PlayerAction");
            engine->RegisterEnumValue("PlayerAction", "STEER_LEFT", PA_STEER_LEFT);
            engine->RegisterEnumValue("PlayerAction", "STEER_RIGHT", PA_STEER_RIGHT);
            engine->RegisterEnumValue("PlayerAction", "ACCEL", PA_ACCEL);
            engine->RegisterEnumValue("PlayerAction", "BRAKE", PA_BRAKE);
            engine->RegisterEnumValue("PlayerAction", "NITRO", PA_NITRO);
            engine->RegisterEnumValue("PlayerAction", "DRIFT", PA_DRIFT);
            engine->RegisterEnumValue("PlayerAction", "RESCUE", PA_RESCUE);
            engine->RegisterEnumValue("PlayerAction", "FIRE", PA_FIRE);
            engine->RegisterEnumValue("PlayerAction", "LOOK_BACK", PA_LOOK_BACK);
            engine->RegisterEnumValue("PlayerAction", "PAUSE_RACE", PA_PAUSE_RACE);
            engine->RegisterEnumValue("PlayerAction", "MENU_UP", PA_MENU_UP);
            engine->RegisterEnumValue("PlayerAction", "MENU_DOWN", PA_MENU_DOWN);
            engine->RegisterEnumValue("PlayerAction", "MENU_LEFT", PA_MENU_LEFT);
            engine->RegisterEnumValue("PlayerAction", "MENU_RIGHT", PA_MENU_RIGHT);
            engine->RegisterEnumValue("PlayerAction", "MENU_SELECT", PA_MENU_SELECT);
            engine->RegisterEnumValue("PlayerAction", "MENU_CANCEL", PA_MENU_CANCEL);
            engine->RegisterEnum("RaceGUIType");
            engine->RegisterEnumValue("RaceGUIType", "KEYBOARD_GAMEPAD", RGT_KEYBOARD_GAMEPAD);
            engine->RegisterEnumValue("RaceGUIType", "STEERING_WHEEL", RGT_STEERING_WHEEL);
            engine->RegisterEnumValue("RaceGUIType", "ACCELEROMETER", RGT_ACCELEROMETER);
            engine->RegisterEnumValue("RaceGUIType", "GYROSCOPE", RGT_GYROSCOPE);
            engine->RegisterEnum("MsgType");
            engine->RegisterEnumValue("MsgType", "GENERIC", MSG_GENERIC);
            engine->RegisterEnumValue("MsgType", "ERROR", MSG_ERROR);
            engine->RegisterEnumValue("MsgType", "ACHIEVEMENT", MSG_ACHIEVEMENT);
            engine->RegisterEnumValue("MsgType", "FRIEND", MSG_FRIEND);
        }
    }

/** \cond DOXYGEN_IGNORE */
}
/** \endcond */
