//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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


#ifndef HEADER_COMMAND_LINE_HPP
#define HEADER_COMMAND_LINE_HPP

#include "utils/string_utils.hpp"

#include <string>
#include <vector>

/** A small class to manage the 'argv' parameters of a program. That includes
 *  the name of the executable (argv[0]) and all command line parameters.
 *  Example usage
 *  \code
 *     CommandLine::init(argc, argv);
 *     if( CommandLine::has("--help") ||
 *         CommandLine::has("-h")         )  ...
 *     int n;
 *     if(CommandLine::has("--log", &n))
 *         Log::setLogLevel(n);
 *     ...
 *     CommandLine::reportInvalidParameters();
 *  \endcode
 *  The two 'has' functions will remove a parameter from the list of all
 *  parameters, so any parameters remaining at the end are invalid
 *  parameters, which will be listed by reportInvalidParameters.
 */
class CommandLine
{
private:
    /** The array with all command line options. */
    static std::vector<std::string>  m_argv;

    /** Name of the executable. */
    static std::string m_exec_name;

    // ------------------------------------------------------------------------
    /** Searches for an option 'option=XX'. If found, *t will contain 'XX'.
     *  If the value was found, the entry is removed from the list of all
     *  command line arguments.
     *  \param option The option (must include '-' or '--' as required).
     *  \param t Address of a variable to store the value.
     *  \param format The '%' format to sscanf the value in t with.
     *  \return true if the value was found, false otherwise.
     */
    static bool has(const std::string &option, void *t, const char* const format)
    {
        if(m_argv.size()==0) return false;

        std::string equal=option+"="+format;
        std::vector<std::string>::iterator i;
        for(i=m_argv.begin(); i<m_argv.end(); i++)
        {
            if(sscanf(i->c_str(), equal.c_str(), t)==1)
            {
                m_argv.erase(i);
                return true;
            }
        }
        return false;
    }   // has

public:
    static void init(unsigned int argc, char *argv[]);
    static void addArgsFromUserConfig();
    static void reportInvalidParameters();
    static bool has(const std::string &option);
    // ------------------------------------------------------------------------
    /** Searches for an option 'option=XX'. If found, *value will contain 'XX'.
     *  If the value was found and the type of XX and value matches each other,
     *  the entry is removed from the list of all command line arguments.
     *  \param option The option (must include '-' or '--' as required).
     *  \param value Pointer to store the value.
     *  \return true if the value was found, false otherwise.
     */
    template<typename T> static bool has(const std::string& option, T* value)
    {
        std::string equal = option + "=";
        std::vector<std::string>::iterator i;
        for (i = m_argv.begin(); i < m_argv.end(); i++)
        {
            if (i->compare(0, equal.size(), equal) == 0)
            {
                std::string result =
                    i->substr(equal.size(), std::string::npos);
                if (StringUtils::fromString(result, *value))
                {
                    m_argv.erase(i);
                    return true;
                }
            }
        }
        return false;
    }
    // ------------------------------------------------------------------------
    /** Searches for an option 'option=XX'. If found, *value will contain 'XX'.
     *  If the value was found and the type of XX and value matches each other,
     *  the entry is removed from the list of all command line arguments.
     *  It copies the result directly to value to include space.
     *  \param option The option (must include '-' or '--' as required).
     *  \param value String pointer to store the value.
     *  \return true if the value was found, false otherwise.
     */
    static bool has(const std::string& option, std::string* value)
    {
        std::string equal = option + "=";
        std::vector<std::string>::iterator i;
        for (i = m_argv.begin(); i < m_argv.end(); i++)
        {
            if (i->compare(0, equal.size(), equal) == 0)
            {
                *value = i->substr(equal.size(), std::string::npos);
                m_argv.erase(i);
                return true;
            }
        }
        return false;
    }   // has<std::string>
    // ------------------------------------------------------------------------
    /** Returns the name of the executable. */
    static const std::string& getExecName() { return m_exec_name; }
};   // CommandLine
#endif
