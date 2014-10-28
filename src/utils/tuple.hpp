#ifndef __STK_TUPLE_HPP__
#define __STK_TUPLE_HPP__

namespace STK {


    // tuple 
    template<typename... _Types> class Tuple;

    // empty tuple
    template<> class Tuple<> {
    public:
        Tuple()
        {
        }
    };

    // recursive tuple definition
    template<typename _This, typename... _Rest>
    class Tuple<_This, _Rest...> : private Tuple<_Rest...>
    {
    public:
        _This _Elem;

        Tuple()
        {
        }

        Tuple(_This val, _Rest... rest) : Tuple<_Rest...>(rest...)
        {
            _Elem = val;
        }
    };

    // tuple_element
    template<size_t _Index, typename _Tuple> struct tuple_element;

    // select first element
    template<typename _This, typename... _Rest>
    struct tuple_element<0, Tuple<_This, _Rest...>>
    {
        typedef _This& type;
        typedef Tuple<_This, _Rest...> _Ttype;
    };

    // recursive tuple_element definition
    template <size_t _Index, typename _This, typename... _Rest>
    struct tuple_element<_Index, Tuple<_This, _Rest...>>
        : public tuple_element<_Index - 1, Tuple<_Rest...> >
    {
    };

    template<size_t _Index, class... _Types> inline
        typename tuple_element<_Index, Tuple<_Types...>>::type
        tuple_get(Tuple<_Types...>& _Tuple)
    {
        typedef typename tuple_element<_Index, Tuple<_Types...>>::_Ttype _Ttype;
        return (((_Ttype&)_Tuple)._Elem);
    }

    template<size_t _Index, class... _Types> inline
    typename tuple_element<_Index, Tuple<_Types...>>::type tuple_get(const Tuple<_Types...>& _Tuple)
    {
        typedef typename tuple_element<_Index, Tuple<_Types...>>::_Ttype _Ttype;
        return (((_Ttype&)_Tuple)._Elem);
    }
    
    template<size_t Index, typename... T> inline
        Tuple<T...> make_tuple(T... values)
    {
        return Tuple<T...>(values...);
    }
    
    template<typename... T> inline
        Tuple<T...> make_tuple(T... values)
    {
        return Tuple<T...>(values...);
    }

    //template<typename... T>
    //int tuple_size(Tuple<T...> tuple)
    //{
    //    return sizeof...(T);
    //}

    template<typename... T>
    struct TupleSize
    {
        const int value;
        TupleSize()
        {
            value = sizeof...(T);
        }
    };
}

#endif
