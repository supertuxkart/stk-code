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
