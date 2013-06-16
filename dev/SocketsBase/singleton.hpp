#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <iostream>

template <typename T>
class Singleton
{
    protected:
        Singleton () { }
        ~Singleton () { std::cout << "destroying singleton." << std::endl; }

    public:
        static T *getInstance ()
        {
            if (NULL == m_singleton)
            {
                std::cout << "creating singleton." << std::endl;
                m_singleton = new T;
            }
            else
            {
                std::cout << "singleton already created!" << std::endl;
            }
            return (static_cast<T*> (m_singleton));
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

template <typename T> T *Singleton<T>::_singleton = NULL;

#endif // SINGLETON_HPP
