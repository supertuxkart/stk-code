//  $Id$
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//  code in this file based on lispreader from Mark Probst
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
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>

#include "lisp/writer.hpp"
#include "utils/translation.hpp"

namespace lisp
{

    Writer::Writer(const std::string& filename)
            : m_indent_depth(0)
    {
        m_owner = true;
#ifdef WIN32
    // With mingw, the files are written dos style (crlf), but
    // these files can't be read with the lexer here. So we have
    // to force the file to be written as binary for windows.
        m_out = new std::ofstream(filename.c_str(),::std::ios_base::binary);
#else
        m_out = new std::ofstream(filename.c_str());
#endif
        if(!m_out->good())
        {
            std::ostringstream msg;
            msg << "LispWriter Error: Couldn't open file '" << filename
                << "' for writing.";
            throw std::runtime_error(msg.str());
        }
    }

//-----------------------------------------------------------------------------

    Writer::Writer(std::ostream& newout)
            : m_indent_depth(0)
    {
        m_owner = false;
        m_out = &newout;
    }

//-----------------------------------------------------------------------------

    Writer::~Writer()
    {
        if(m_lists.size() > 0)
        {
            std::cerr << "Warning: Not all sections closed in lispwriter!\n";
        }

        if(m_owner)
            delete m_out;
    }

//-----------------------------------------------------------------------------

    void
    Writer::writeComment(const std::string& comment)
    {
        indent();
        *m_out << "; " << comment << "\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::beginList(const std::string& listname)
    {
        indent();
        *m_out << '(' << listname << '\n';
        m_indent_depth += 2;

        m_lists.push_back(listname);
    }

//-----------------------------------------------------------------------------

    void
    Writer::endList(const std::string& listname)
    {
        if(m_lists.size() == 0)
        {
            fprintf(stderr, "Trying to close list '%s, which is not open.\n",
                    listname.c_str());
            return;
        }
        if(m_lists.back() != listname)
        {
            fprintf(stderr, 
                    "Warning: trying to close list '%s' while list '%s' is open.\n",
                    listname.c_str(),  m_lists.back().c_str());
            return;
        }
        m_lists.pop_back();

        m_indent_depth -= 2;
        indent();
        *m_out << ")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, int value)
    {
        indent();
        *m_out << '(' << name << ' ' << value << ")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, float value)
    {
        indent();
        *m_out << '(' << name << ' ' << value << ")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, const std::string& value)
    {
        indent();
        *m_out << '(' << name << " \"" << value << "\")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, const char* value)
    {
        indent();
        *m_out << '(' << name << " \"" << value << "\")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, bool value)
    {
        indent();
        *m_out << '(' << name << ' ' << (value ? "#t" : "#f") << ")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, const std::vector<int>& value)
    {
        indent();
        *m_out << '(' << name;
        for(std::vector<int>::const_iterator i = value.begin(); i != value.end(); ++i)
            *m_out << " " << *i;
        *m_out << ")\n";
    }

//-----------------------------------------------------------------------------

    void
    Writer::write(const std::string& name, const std::vector<unsigned int>& value)
    {
        indent();
        *m_out << '(' << name;
        for(std::vector<unsigned int>::const_iterator i = value.begin(); i != value.end(); ++i)
            *m_out << " " << *i;
        *m_out << ")\n";
    }

//-----------------------------------------------------------------------------
    
    void
    Writer::write(const std::string& name, const std::vector<std::string>& value)
    {
        indent();
        *m_out << '(' << name;
        for(std::vector<std::string>::const_iterator i = value.begin(); i != value.end(); ++i)
            *m_out << " " << "\"" << *i << "\"";
        *m_out << ")\n";
    }

//-----------------------------------------------------------------------------
    
    void
    Writer::indent()
    {
        for(int i = 0; i<m_indent_depth; ++i)
            *m_out << ' ';
    }

} // end of namespace lisp
