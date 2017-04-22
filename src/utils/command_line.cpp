//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include "utils/command_line.hpp"

#include "config/user_config.hpp"
#include "utils/log.hpp"

std::vector<std::string>  CommandLine::m_argv;
std::string               CommandLine::m_exec_name="";

/** The constructor takes the standard C arguments argc and argv and
 *  stores the information internally.
 *  \param argc Number of arguments (in argv).
 *  \param argv Array of char* with all command line arguments.
 */
void CommandLine::init(unsigned int argc, char *argv[])
{
    if (argc > 0)
        m_exec_name = argv[0];

    for(unsigned int i=1; i<argc; i++)
        m_argv.push_back(argv[i]);
}   // CommandLine

// ----------------------------------------------------------------------------
bool CommandLine::has(const std::string &option)
{
    std::vector<std::string>::iterator i;
    for(i=m_argv.begin(); i!=m_argv.end(); i++)
    {
        if(*i==option)
        {
            m_argv.erase(i);
            return true;
        }
    }  // for i in m_argv

    return false;
}   // has

// ----------------------------------------------------------------------------
/** Reports any parameters that have not been handled yet to be an error.
 */
void CommandLine::reportInvalidParameters()
{
    for(unsigned int i=0; i<m_argv.size(); i++)
    {
        // invalid param needs to go to console
        UserConfigParams::m_log_errors_to_console = true;

        Log::error("CommandLine", "Invalid parameter: %s.", m_argv[i].c_str() );
    }
}   // reportInvalidParameters
