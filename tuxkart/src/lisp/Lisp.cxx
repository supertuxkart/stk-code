//  $Id: Lisp.cxx,v 1.1 2004/08/24 19:33:11 matzebraun Exp $
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
#include "Lisp.h"

namespace lisp
{
    
Lisp::Lisp(LispType newtype)
    : type(newtype)
{
}

Lisp::~Lisp()
{
    if(type == TYPE_SYMBOL || type == TYPE_STRING)
        delete[] v.string;
    if(type == TYPE_CONS) {
        delete v.cons.cdr;
        delete v.cons.car;
    }
}

const Lisp*
Lisp::getLisp(const char* name) const
{
    const Lisp* p;

    for(p = getCdr(); p != 0; p = p->getCdr()) {
        const Lisp* child = p->getCar();
        if(!child)
            continue;
        if(!child->getCar())
            continue;
        std::string childName;
        if(!child->getCar()->get(childName))
            continue;
        if(childName == name)
            return child;
    }

    return 0;
}

void
Lisp::print(int ) const
{
    if(type == TYPE_CONS) {
        printf("(");
        if(v.cons.car)
            v.cons.car->print();
        if(v.cons.cdr) {
            printf(",");
            if(v.cons.cdr)
                v.cons.cdr->print();
        }
        printf(")");
    }
    if(type == TYPE_STRING) {
        printf("'%s' ", v.string);
    }
    if(type == TYPE_INTEGER) {
        printf("%d", v.integer);
    }
    if(type == TYPE_REAL) {
        printf("%f", v.real);
    }
    if(type == TYPE_SYMBOL) {
        printf("%s ", v.string);
    }
}

} // end of namespace lisp
