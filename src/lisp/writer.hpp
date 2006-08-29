//  $Id$
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//  code in this file based on lispreader from Mark Probst
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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
#ifndef __LISPWRITER_H__
#define __LISPWRITER_H__

#include <iostream>
#include <string>
#include <vector>

namespace lisp
{

class Writer
{
public:
    Writer(const std::string& filename);
    Writer(std::ostream& out);
    ~Writer();

    void writeComment(const std::string& comment);

    void beginList(const std::string& listname);

    void write(const std::string& name, int value);
    void write(const std::string& name, float value);
    void write(const std::string& name, const std::string& value);
    void write(const std::string& name, bool value);
    void write(const std::string& name, const std::vector<int>& value);
    void write(const std::string& name, const std::vector<unsigned int>& value);
    // add more write-functions when needed...

    void endList(const std::string& listname);

private:
    void indent();

    bool owner;
    std::ostream* out;
    int indent_depth;
    std::vector<std::string> lists;
};

} //namespace lisp

#endif

