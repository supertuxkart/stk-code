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
