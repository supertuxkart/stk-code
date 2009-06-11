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
#include "lisp/lisp.hpp"

namespace lisp
{

    Lisp::Lisp(LispType newtype)
            : m_type(newtype)
    {}

//-----------------------------------------------------------------------------

    Lisp::~Lisp()
    {
        if(m_type == TYPE_SYMBOL || m_type == TYPE_STRING)
            delete[] m_v.m_string;
        if(m_type == TYPE_CONS)
        {
            delete m_v.m_cons.m_cdr;
            delete m_v.m_cons.m_car;
        }
    }

//-----------------------------------------------------------------------------

    const Lisp*
    Lisp::getLisp(const char* name) const
    {
        const Lisp* P;

        for(P = getCdr(); P != 0; P = P->getCdr())
        {
            const Lisp* CHILD = P->getCar();
            // Also ignore if the child is not a CONS type, i.e.
            // a TYPE_INTEGER is found, for which car is not defined!
            if(!CHILD || CHILD->m_type!=TYPE_CONS) 
                continue;
            if(!CHILD->getCar())
                continue;
            std::string CHILDName;
            if(!CHILD->getCar()->get(CHILDName))
                continue;
            if(CHILDName == name)
                return CHILD;
        }

        return 0;
    }

//-----------------------------------------------------------------------------

    //FIXME: is the boolean handled by this function? should the argument be
    //removed?
    void
    Lisp::print(int ) const
    {
        if(m_type == TYPE_CONS)
        {
            printf("(");
            if(m_v.m_cons.m_car)
                m_v.m_cons.m_car->print();
            if(m_v.m_cons.m_cdr)
            {
                printf(",");
                if(m_v.m_cons.m_cdr)
                    m_v.m_cons.m_cdr->print();
            }
            printf(")");
        }
        if(m_type == TYPE_STRING)
        {
            printf("'%s' ", m_v.m_string);
        }
        if(m_type == TYPE_INTEGER)
        {
            printf("%d", m_v.m_integer);
        }
        if(m_type == TYPE_REAL)
        {
            printf("%f", m_v.m_real);
        }
        if(m_type == TYPE_SYMBOL)
        {
            printf("%s ", m_v.m_string);
        }
    }

} // end of namespace lisp
