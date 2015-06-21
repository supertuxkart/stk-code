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
    /** Represents a scripting function to execute after a given time */
    struct PendingTimeout
    {
        double m_time;
        std::string m_callback_name;

        PendingTimeout(double time, const std::string& callback_name)
        {
            m_time = time;
            m_callback_name = callback_name;
        }
    };

    class ScriptEngine
    {
    public:

        ScriptEngine();
        ~ScriptEngine();

        void runFunction(std::string function_name);
        void runFunction(std::string function_name,
            std::function<void(asIScriptContext*)> callback);
        void runFunction(std::string function_name,
            std::function<void(asIScriptContext*)> callback,
            std::function<void(asIScriptContext*)> get_return_value);
        void evalScript(std::string script_fragment);
        void cleanupCache();

        bool loadScript(std::string script_path, bool clear_previous);
        bool compileLoadedScripts();

        void addPendingTimeout(double time, const std::string& callback_name);
        void update(double dt);

    private:
        asIScriptEngine *m_engine;
        std::map<std::string, asIScriptFunction*> m_functions_cache;
        std::vector<PendingTimeout> m_pending_timeouts;

        void configureEngine(asIScriptEngine *engine);
    };   // class ScriptEngine

}
#endif

