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

#include "script_audio.hpp"
#include "audio/sfx_manager.hpp"
#include "scriptengine/aswrappedcall.hpp"

#include <angelscript.h>
#include <assert.h>
#include <cstring>

/** \cond DOXYGEN_IGNORE */
namespace Scripting
{
    /** \endcond */

    namespace Audio
    {
        /** \addtogroup Scripting
        * @{
        */
        /** \addtogroup Scripting_Audio Audio
        * @{
        */

        /** Plays a sound by name */

        void playSound(const std::string* sound_name)
        {
            SFXManager::get()->quickSound(*sound_name);
        }

        /** @}*/
        /** @}*/

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            engine->SetDefaultNamespace("Audio");
            
            bool mp = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
            asDWORD call_conv = mp ? asCALL_GENERIC : asCALL_CDECL;
            int r; // of type asERetCodes
            
            r = engine->RegisterGlobalFunction("void playSound(const string &in)", 
                                               mp ? WRAP_FN(playSound) : asFUNCTION(playSound), 
                                               call_conv); assert(r >= 0);
        }
    }

/** \cond DOXYGEN_IGNORE */
}
/** \endcond */
