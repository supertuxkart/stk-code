//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <iostream>

template <typename T>
class Singleton
{
    protected:
        Singleton () { }
        virtual ~Singleton () { std::cout << "destroying singleton." << std::endl; delete m_singleton; }

    public:
        template<typename S>
        static S *getInstance ()
        {
            if (m_singleton == NULL)
                m_singleton = new S;
            
            S* result = (dynamic_cast<S*> (m_singleton));
            if (result == NULL)
            {
                std::cout << "BE CAREFUL : Reallocating singleton" << std::endl;
                m_singleton = new S;
            }
            return (dynamic_cast<S*> (m_singleton));
        }
        static T *getInstance()
        {
            return (dynamic_cast<T*> (m_singleton));
        }

        static void kill ()
        {
            if (NULL != m_singleton)
            {
                delete m_singleton;
                m_singleton = NULL;
            }
        }

    private:
        static T *m_singleton;
};

template <typename T> T *Singleton<T>::m_singleton = NULL;

#endif // SINGLETON_HPP
