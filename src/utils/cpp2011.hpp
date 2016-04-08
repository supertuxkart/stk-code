//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef CPP2011_HPP
#define CPP2011_HPP
#include <vector>
#if __cplusplus >= 201103 || _MSC_VER >=1800

    #define OVERRIDE override

#else

    #define OVERRIDE

#endif

#if (__cplusplus >= 201103 || _MSC_VER >=1800) && !(defined(__clang__) && defined(__APPLE__))
#define STDCPP2011
#else
#define STDCPP2003
#endif


template<typename T, typename... Args>
void pushVector(std::vector<T> *vec, Args ...args)
{
#ifdef STDCPP2003
    vec->push_back(T(args...));
#else
    vec->emplace_back(args...);
#endif
}

struct Util
{
    template <typename T>
    static void populate(std::vector<T> &v)
    { }

    template <typename T, typename...Args>
    static void populate(std::vector<T> &v, T t, Args... args)
    {
        v.push_back(t);
        populate<T>(v, args...);
    }
};


template<typename T, typename...Args>
static std::vector<T> createVector(Args...args)
{
    std::vector<T> result = std::vector<T>();
    Util::template populate<T>(result, args...);
    return result;
}

#endif
