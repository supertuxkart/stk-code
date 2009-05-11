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
#ifndef __LISPREADER_H__
#define __LISPREADER_H__

#include <string>
#include <vector>
#ifdef HAVE_IRRLICHT
#include "irrlicht.h"
#endif
#include "utils/vec3.hpp"

namespace lisp
{

    class Lisp
    {
    public:
        ~Lisp();

        enum LispType
        {
            TYPE_CONS,
            TYPE_SYMBOL,
            TYPE_INTEGER,
            TYPE_STRING,
            TYPE_REAL,
            TYPE_BOOLEAN
        };

        LispType getType() const
            { return m_type; }

        const Lisp* getCar() const
            { return m_v.m_cons.m_car; }
        const Lisp* getCdr() const
            { return m_v.m_cons.m_cdr; }
        bool get(std::string& val) const
            {
                if(m_type != TYPE_STRING && m_type != TYPE_SYMBOL)
                    return false;
                val = m_v.m_string;
                return true;
            }
        bool get(int& val) const
            {
                if(m_type != TYPE_INTEGER)
                    return false;
                val = m_v.m_integer;
                return true;
            }
        bool get(float& val) const
            {
                if(m_type != TYPE_REAL && m_type != TYPE_INTEGER)
                    return false;
                val = m_type==TYPE_REAL ? m_v.m_real : m_v.m_integer;
                return true;
            }
        bool get(bool& val) const
            {
                if(m_type != TYPE_BOOLEAN)
                    return false;
                val = m_v.m_boolean;
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
        template<class T>
        bool get(const std::string &name, T& val) const
        {
            return get(name.c_str(), val);
        }


        bool get(const char* name, float* val, int num) const
        {
            const Lisp* lisp = getLisp(name);
            if(!lisp)
                return false;
            
            lisp = lisp->getCdr();
            if(!lisp)
                return false;
            for(int i = 0; i < num && lisp; ++i)
            {
                const Lisp* m_car = lisp->getCar();
                if(!m_car)
                    return false;
                m_car->get(val[i]);
                lisp = lisp->getCdr();
            }
            return true;
        }


        /*
        bool get(const char* name, sgVec3& val) const
            {
                const Lisp* lisp = getLisp(name);
                if(!lisp)
                    return false;

                lisp = lisp->getCdr();
                if(!lisp)
                    return false;
                for(int i = 0; i < 3 && lisp; ++i)
                {
                    const Lisp* m_car = lisp->getCar();
                    if(!m_car)
                        return false;
                    m_car->get(val[i]);
                    lisp = lisp->getCdr();
                }
                return true;
            }
         */
        
#ifdef HAVE_IRRLICHT
        bool get(const char* name, core::vector3df& val) const
            {
                const Lisp* lisp = getLisp(name);
                if(!lisp)
                    return false;

                lisp = lisp->getCdr();
                if(!lisp) return false;

                const Lisp* m_car = lisp->getCar();
                if(!m_car) return false;
                m_car->get(val.X);
                lisp = lisp->getCdr();
                m_car = lisp->getCar();
                if(!m_car) return false;
                m_car->get(val.Y);
                lisp = lisp->getCdr();
                m_car = lisp->getCar();
                if(!m_car) return false;
                m_car->get(val.Z);
                lisp = lisp->getCdr();

                return true;
            }
#endif
        bool get(const char* name, Vec3& val) const
            {
                const Lisp* lisp = getLisp(name);
                if(!lisp)
                    return false;

                lisp = lisp->getCdr();
                if(!lisp)
                    return false;
                float f[3];
                for(int i = 0; i < 3 && lisp; ++i)
                {
                    const Lisp* m_car = lisp->getCar();
                    if(!m_car)
                        return false;
                    m_car->get(f[i]);
                    lisp = lisp->getCdr();
                }
                // Lists are stored in reverse order, so reverse to original order
                val.setValue(f[2],f[1],f[0]);
                return true;
            }

        template<class T>
        bool getVector(const char* name, std::vector<T>& vec) const
        {
            const Lisp* child = getLisp(name);
            if(!child)
                return false;

            for(child = child->getCdr(); child != 0; child = child->getCdr())
            {
                T val;
                if(!child->getCar())
                    continue;
                if(child->getCar()->get(val))
                {
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

        LispType m_type;
        union
        {
            struct
            {
                Lisp* m_car;
                Lisp* m_cdr;
            }
            m_cons;

            char* m_string;
            int m_integer;
            bool m_boolean;
            float m_real;
        }
        m_v;
    };

} // end of namespace lisp

#endif

