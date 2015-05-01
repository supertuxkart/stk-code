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

#ifndef HEADER_SCRIPT_ENGINE_HPP
#define HEADER_SCRIPT_ENGINE_HPP

#include <string>
#include <angelscript.h>
#include <functional>

class TrackObjectPresentation;

namespace Scripting
{

    namespace Physics
    {
        void registerScriptFunctions(asIScriptEngine *engine);
        void setCollision(int collider1,int collider2);
        void setCollision(std::string collider1, std::string collider2);
    }

    namespace Kart
    {
        void registerScriptFunctions(asIScriptEngine *engine);
    }

    namespace Track
    {
        void registerScriptFunctions(asIScriptEngine *engine);

        void registerScriptEnums(asIScriptEngine *engine);

        asIScriptFunction*
            registerScriptCallbacks(asIScriptEngine *engine, std::string scriptName);
    }
    
    class ScriptEngine
    {
    public:

        ScriptEngine();
        ~ScriptEngine();
        
        void runScript(std::string scriptName);
        void runScript(std::string scriptName, std::function<void(asIScriptContext*)> callback);
        void cleanupCache();
        
    private:
        asIScriptEngine *m_engine;
        std::map<std::string, asIScriptFunction*> m_script_cache;

        void configureEngine(asIScriptEngine *engine);
        int  compileScript(asIScriptEngine *engine,std::string scriptName);
    };   // class ScriptEngine

}
#endif

