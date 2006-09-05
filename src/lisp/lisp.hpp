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
#ifndef __LISPREADER_H__
#define __LISPREADER_H__

#include <string>
#include <vector>
#include <plib/sg.h>

namespace lisp
{

class Lisp
{
public:
    ~Lisp();
    
    enum LispType {
        TYPE_CONS,
        TYPE_SYMBOL,
        TYPE_INTEGER,
        TYPE_STRING,
        TYPE_REAL,
        TYPE_BOOLEAN
    };

    LispType getType() const
    { return type; } 

    const Lisp* getCar() const
    { return v.cons.car; }
    const Lisp* getCdr() const
    { return v.cons.cdr; }
    bool get(std::string& val) const
    { 
        if(type != TYPE_STRING && type != TYPE_SYMBOL)
            return false;
        val = v.string;
        return true;
    }
    bool get(int& val) const
    {
        if(type != TYPE_INTEGER)
            return false;
        val = v.integer;
        return true;
    }
    bool get(float& val) const
    {
      if(type != TYPE_REAL && type != TYPE_INTEGER)
            return false;
        val = type==TYPE_REAL ? v.real : v.integer;
        return true;
    }
    bool get(bool& val) const
    {
        if(type != TYPE_BOOLEAN)
            return false;
        val = v.boolean;
        return true;
    }

    /* conveniance functions which traverse the list until a child with a
     * specified name is found. The value part is then interpreted in a specific
     * way. The functions return true, if a child was found and could be
     * interpreted correctly, otherwise false is returned and the variable value
     * is not changed.
     * (Please note that searching the lisp structure is O(n) so these functions
     *  are no good idea for performance critical areas)
     */
    template<class T>
    bool get(const char* name, T& val) const
    {
        const Lisp* lisp = getLisp(name);
        if(!lisp)
            return false;
        
        lisp = lisp->getCdr();
        if(!lisp)
            return false;
        lisp = lisp->getCar();
        if(!lisp)
            return false;
        return lisp->get(val);
    }
    bool get(const char* name, sgVec4& val) const
    {
        const Lisp* lisp = getLisp(name);
        if(!lisp)
            return false;

        lisp = lisp->getCdr();
        if(!lisp)
            return false;
        for(int i = 0; i < 4 && lisp; ++i) {
            const Lisp* car = lisp->getCar();
            if(!car)
                return false;
            car->get(val[i]);
            lisp = lisp->getCdr();
        }
        return true;
    }
    bool get(const char* name, sgVec3& val) const
    {
        const Lisp* lisp = getLisp(name);
        if(!lisp)
            return false;

        lisp = lisp->getCdr();
        if(!lisp)
            return false;
        for(int i = 0; i < 3 && lisp; ++i) {
            const Lisp* car = lisp->getCar();
            if(!car)
                return false;
            car->get(val[i]);
            lisp = lisp->getCdr();
        }
        return true;
    }

    template<class T>
    bool getVector(const char* name, std::vector<T>& vec) const
    {
        const Lisp* child = getLisp(name);
        if(!child)
            return false;

        for(child = child->getCdr(); child != 0; child = child->getCdr()) {
            T val;
            if(!child->getCar())
                continue;
            if(child->getCar()->get(val)) {
	      vec.insert(vec.begin(),val);
            }
        }

        return true;
    }
    
    const Lisp* getLisp(const char* name) const;
    const Lisp* getLisp(const std::string& name) const
    { return getLisp(name.c_str()); }

    // for debugging
    void print(int indent = 0) const;

private:
    friend class Parser;
    Lisp(LispType newtype);

    LispType type;
    union
    {
        struct
        {
            Lisp* car;
            Lisp* cdr;
        } cons;

        char* string;
        int integer;
        bool boolean;
        float real;
    } v;
};

} // end of namespace lisp

#endif

