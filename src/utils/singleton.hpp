//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

/*! \file singleton.hpp
 */

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "utils/log.hpp"

/*! \class ProtocolManager
 *  \brief Manages the protocols at runtime.
 *  This has been designed to allow multi-inheritance. This is advised to
 *  re-declare getInstance, but whithout templates parameters in the inheriting
 *  classes.
 */
template <typename T>
class AbstractSingleton
{
    protected:
        /*! \brief Constructor */
        AbstractSingleton() { m_singleton = NULL; }
        /*! \brief Destructor */
        virtual ~AbstractSingleton()
        {
            Log::info("Singleton", "Destroyed singleton.");
        }

    public:
        /*! \brief Used to get the instance, after a dynamic cast.
         *  This is important when making a double-inheritance of this class.
         *  For example, if A is a singleton inherited by B, you can call
         *  B::getInstance<A>() to have the instance returned as a A*.
         *  If the cast fails, a log message will notify it.
         */
        template<typename S>
        static S *getInstance ()
        {
            if (m_singleton == NULL)
                m_singleton = new S;

            S* result = (dynamic_cast<S*> (m_singleton));
            if (result == NULL)
                Log::debug("Singleton", "THE SINGLETON HAS NOT BEEN REALOCATED, IT IS NOT OF THE REQUESTED TYPE.");
            return result;
        }
        /*! \brief Used to get the instance. */
        static T *getInstance()
        {
            return m_singleton;
        }

        /*! \brief Used to kill the singleton, if needed. */
        static void kill ()
        {
            if (m_singleton)
            {
                delete m_singleton;
                m_singleton = NULL;
            }
        }

    private:
        static T *m_singleton;
};

template <typename T> T *AbstractSingleton<T>::m_singleton = NULL;

template <typename T>
class Singleton
{
protected:
    /*! \brief Constructor */
    Singleton() { m_singleton = NULL; }
    /*! \brief Destructor */
    virtual ~Singleton()
    {
        Log::info("Singleton", "Destroyed singleton.");
    }

public:
    /*! \brief Used to get the instance. */
    static T *getInstance()
    {
        if (m_singleton == NULL)
            m_singleton = new T;
        return m_singleton;
    }

    /*! \brief Used to kill the singleton, if needed. */
    static void kill()
    {
        if (m_singleton)
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
