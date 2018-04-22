#ifndef AS_GEN_WRAPPER_H
#define AS_GEN_WRAPPER_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif
#include <new>

#ifdef GetObject
#undef GetObject
#endif

namespace gw {

template <typename T> class Proxy {
	public:
		T value;
		Proxy(T value) : value(value) {}
		static T cast(void * ptr) {
			return reinterpret_cast<Proxy<T> *>(&ptr)->value;
		}
	private:
		Proxy(const Proxy &);
		Proxy & operator=(const Proxy &);
};

template <typename T> struct Wrapper {};
template <typename T> struct ObjFirst {};
template <typename T> struct ObjLast {};
template <typename T> struct Constructor {};

template <typename T>
void destroy(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
	static_cast<T *>(gen->GetObject())->~T();
}
template <>
struct Wrapper<void (*)(void)> {
	template <void (*fp)(void)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)());
	}
};
template <typename R>
struct Wrapper<R (*)(void)> {
	template <R (*fp)(void)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)());
	}
};
template <typename T>
struct Wrapper<void (T::*)(void)> {
	template <void (T::*fp)(void)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)());
	}
};
template <typename T, typename R>
struct Wrapper<R (T::*)(void)> {
	template <R (T::*fp)(void)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)());
	}
};
template <typename T>
struct Wrapper<void (T::*)(void) const> {
	template <void (T::*fp)(void) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)());
	}
};
template <typename T, typename R>
struct Wrapper<R (T::*)(void) const> {
	template <R (T::*fp)(void) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)());
	}
};
template <typename T>
struct ObjFirst<void (*)(T)> {
	template <void (*fp)(T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R>
struct ObjFirst<R (*)(T)> {
	template <R (*fp)(T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T>
struct ObjLast<void (*)(T)> {
	template <void (*fp)(T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R>
struct ObjLast<R (*)(T)> {
	template <R (*fp)(T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T>
struct Constructor <T ()> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T();
	}
};
template <typename A0>
struct Wrapper<void (*)(A0)> {
	template <void (*fp)(A0)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename R, typename A0>
struct Wrapper<R (*)(A0)> {
	template <R (*fp)(A0)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename A0>
struct Wrapper<void (T::*)(A0)> {
	template <void (T::*fp)(A0)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename R, typename A0>
struct Wrapper<R (T::*)(A0)> {
	template <R (T::*fp)(A0)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename A0>
struct Wrapper<void (T::*)(A0) const> {
	template <void (T::*fp)(A0) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename R, typename A0>
struct Wrapper<R (T::*)(A0) const> {
	template <R (T::*fp)(A0) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename A0>
struct ObjFirst<void (*)(T, A0)> {
	template <void (*fp)(T, A0)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename R, typename A0>
struct ObjFirst<R (*)(T, A0)> {
	template <R (*fp)(T, A0)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value));
	}
};
template <typename T, typename A0>
struct ObjLast<void (*)(A0, T)> {
	template <void (*fp)(A0, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0>
struct ObjLast<R (*)(A0, T)> {
	template <R (*fp)(A0, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0>
struct Constructor <T (A0)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value);
	}
};
template <typename A0, typename A1>
struct Wrapper<void (*)(A0, A1)> {
	template <void (*fp)(A0, A1)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename R, typename A0, typename A1>
struct Wrapper<R (*)(A0, A1)> {
	template <R (*fp)(A0, A1)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename A0, typename A1>
struct Wrapper<void (T::*)(A0, A1)> {
	template <void (T::*fp)(A0, A1)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename R, typename A0, typename A1>
struct Wrapper<R (T::*)(A0, A1)> {
	template <R (T::*fp)(A0, A1)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename A0, typename A1>
struct Wrapper<void (T::*)(A0, A1) const> {
	template <void (T::*fp)(A0, A1) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename R, typename A0, typename A1>
struct Wrapper<R (T::*)(A0, A1) const> {
	template <R (T::*fp)(A0, A1) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename A0, typename A1>
struct ObjFirst<void (*)(T, A0, A1)> {
	template <void (*fp)(T, A0, A1)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename R, typename A0, typename A1>
struct ObjFirst<R (*)(T, A0, A1)> {
	template <R (*fp)(T, A0, A1)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value));
	}
};
template <typename T, typename A0, typename A1>
struct ObjLast<void (*)(A0, A1, T)> {
	template <void (*fp)(A0, A1, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0, typename A1>
struct ObjLast<R (*)(A0, A1, T)> {
	template <R (*fp)(A0, A1, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0, typename A1>
struct Constructor <T (A0, A1)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value);
	}
};
template <typename A0, typename A1, typename A2>
struct Wrapper<void (*)(A0, A1, A2)> {
	template <void (*fp)(A0, A1, A2)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename R, typename A0, typename A1, typename A2>
struct Wrapper<R (*)(A0, A1, A2)> {
	template <R (*fp)(A0, A1, A2)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2>
struct Wrapper<void (T::*)(A0, A1, A2)> {
	template <void (T::*fp)(A0, A1, A2)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2>
struct Wrapper<R (T::*)(A0, A1, A2)> {
	template <R (T::*fp)(A0, A1, A2)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2>
struct Wrapper<void (T::*)(A0, A1, A2) const> {
	template <void (T::*fp)(A0, A1, A2) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2>
struct Wrapper<R (T::*)(A0, A1, A2) const> {
	template <R (T::*fp)(A0, A1, A2) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2>
struct ObjFirst<void (*)(T, A0, A1, A2)> {
	template <void (*fp)(T, A0, A1, A2)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2>
struct ObjFirst<R (*)(T, A0, A1, A2)> {
	template <R (*fp)(T, A0, A1, A2)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2>
struct ObjLast<void (*)(A0, A1, A2, T)> {
	template <void (*fp)(A0, A1, A2, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2>
struct ObjLast<R (*)(A0, A1, A2, T)> {
	template <R (*fp)(A0, A1, A2, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0, typename A1, typename A2>
struct Constructor <T (A0, A1, A2)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value);
	}
};
template <typename A0, typename A1, typename A2, typename A3>
struct Wrapper<void (*)(A0, A1, A2, A3)> {
	template <void (*fp)(A0, A1, A2, A3)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename R, typename A0, typename A1, typename A2, typename A3>
struct Wrapper<R (*)(A0, A1, A2, A3)> {
	template <R (*fp)(A0, A1, A2, A3)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3>
struct Wrapper<void (T::*)(A0, A1, A2, A3)> {
	template <void (T::*fp)(A0, A1, A2, A3)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
struct Wrapper<R (T::*)(A0, A1, A2, A3)> {
	template <R (T::*fp)(A0, A1, A2, A3)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3>
struct Wrapper<void (T::*)(A0, A1, A2, A3) const> {
	template <void (T::*fp)(A0, A1, A2, A3) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
struct Wrapper<R (T::*)(A0, A1, A2, A3) const> {
	template <R (T::*fp)(A0, A1, A2, A3) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3>
struct ObjFirst<void (*)(T, A0, A1, A2, A3)> {
	template <void (*fp)(T, A0, A1, A2, A3)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
struct ObjFirst<R (*)(T, A0, A1, A2, A3)> {
	template <R (*fp)(T, A0, A1, A2, A3)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3>
struct ObjLast<void (*)(A0, A1, A2, A3, T)> {
	template <void (*fp)(A0, A1, A2, A3, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3>
struct ObjLast<R (*)(A0, A1, A2, A3, T)> {
	template <R (*fp)(A0, A1, A2, A3, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3>
struct Constructor <T (A0, A1, A2, A3)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value);
	}
};
template <typename A0, typename A1, typename A2, typename A3, typename A4>
struct Wrapper<void (*)(A0, A1, A2, A3, A4)> {
	template <void (*fp)(A0, A1, A2, A3, A4)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct Wrapper<R (*)(A0, A1, A2, A3, A4)> {
	template <R (*fp)(A0, A1, A2, A3, A4)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
struct Wrapper<void (T::*)(A0, A1, A2, A3, A4)> {
	template <void (T::*fp)(A0, A1, A2, A3, A4)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct Wrapper<R (T::*)(A0, A1, A2, A3, A4)> {
	template <R (T::*fp)(A0, A1, A2, A3, A4)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
struct Wrapper<void (T::*)(A0, A1, A2, A3, A4) const> {
	template <void (T::*fp)(A0, A1, A2, A3, A4) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct Wrapper<R (T::*)(A0, A1, A2, A3, A4) const> {
	template <R (T::*fp)(A0, A1, A2, A3, A4) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
struct ObjFirst<void (*)(T, A0, A1, A2, A3, A4)> {
	template <void (*fp)(T, A0, A1, A2, A3, A4)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct ObjFirst<R (*)(T, A0, A1, A2, A3, A4)> {
	template <R (*fp)(T, A0, A1, A2, A3, A4)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
struct ObjLast<void (*)(A0, A1, A2, A3, A4, T)> {
	template <void (*fp)(A0, A1, A2, A3, A4, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct ObjLast<R (*)(A0, A1, A2, A3, A4, T)> {
	template <R (*fp)(A0, A1, A2, A3, A4, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4>
struct Constructor <T (A0, A1, A2, A3, A4)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value);
	}
};
template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Wrapper<void (*)(A0, A1, A2, A3, A4, A5)> {
	template <void (*fp)(A0, A1, A2, A3, A4, A5)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Wrapper<R (*)(A0, A1, A2, A3, A4, A5)> {
	template <R (*fp)(A0, A1, A2, A3, A4, A5)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Wrapper<void (T::*)(A0, A1, A2, A3, A4, A5)> {
	template <void (T::*fp)(A0, A1, A2, A3, A4, A5)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Wrapper<R (T::*)(A0, A1, A2, A3, A4, A5)> {
	template <R (T::*fp)(A0, A1, A2, A3, A4, A5)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Wrapper<void (T::*)(A0, A1, A2, A3, A4, A5) const> {
	template <void (T::*fp)(A0, A1, A2, A3, A4, A5) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Wrapper<R (T::*)(A0, A1, A2, A3, A4, A5) const> {
	template <R (T::*fp)(A0, A1, A2, A3, A4, A5) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct ObjFirst<void (*)(T, A0, A1, A2, A3, A4, A5)> {
	template <void (*fp)(T, A0, A1, A2, A3, A4, A5)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct ObjFirst<R (*)(T, A0, A1, A2, A3, A4, A5)> {
	template <R (*fp)(T, A0, A1, A2, A3, A4, A5)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct ObjLast<void (*)(A0, A1, A2, A3, A4, A5, T)> {
	template <void (*fp)(A0, A1, A2, A3, A4, A5, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct ObjLast<R (*)(A0, A1, A2, A3, A4, A5, T)> {
	template <R (*fp)(A0, A1, A2, A3, A4, A5, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct Constructor <T (A0, A1, A2, A3, A4, A5)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value);
	}
};
template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Wrapper<void (*)(A0, A1, A2, A3, A4, A5, A6)> {
	template <void (*fp)(A0, A1, A2, A3, A4, A5, A6)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Wrapper<R (*)(A0, A1, A2, A3, A4, A5, A6)> {
	template <R (*fp)(A0, A1, A2, A3, A4, A5, A6)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Wrapper<void (T::*)(A0, A1, A2, A3, A4, A5, A6)> {
	template <void (T::*fp)(A0, A1, A2, A3, A4, A5, A6)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Wrapper<R (T::*)(A0, A1, A2, A3, A4, A5, A6)> {
	template <R (T::*fp)(A0, A1, A2, A3, A4, A5, A6)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Wrapper<void (T::*)(A0, A1, A2, A3, A4, A5, A6) const> {
	template <void (T::*fp)(A0, A1, A2, A3, A4, A5, A6) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Wrapper<R (T::*)(A0, A1, A2, A3, A4, A5, A6) const> {
	template <R (T::*fp)(A0, A1, A2, A3, A4, A5, A6) const>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((static_cast<T *>(gen->GetObject())->*fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct ObjFirst<void (*)(T, A0, A1, A2, A3, A4, A5, A6)> {
	template <void (*fp)(T, A0, A1, A2, A3, A4, A5, A6)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct ObjFirst<R (*)(T, A0, A1, A2, A3, A4, A5, A6)> {
	template <R (*fp)(T, A0, A1, A2, A3, A4, A5, A6)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				Proxy<T>::cast(gen->GetObject()),
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct ObjLast<void (*)(A0, A1, A2, A3, A4, A5, A6, T)> {
	template <void (*fp)(A0, A1, A2, A3, A4, A5, A6, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct ObjLast<R (*)(A0, A1, A2, A3, A4, A5, A6, T)> {
	template <R (*fp)(A0, A1, A2, A3, A4, A5, A6, T)>
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetAddressOfReturnLocation()) Proxy<R>((fp)(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value,
				Proxy<T>::cast(gen->GetObject())));
	}
};
template <typename T, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct Constructor <T (A0, A1, A2, A3, A4, A5, A6)> {
	static void f(AS_NAMESPACE_QUALIFIER asIScriptGeneric * gen) {
		new (gen->GetObject()) T(
				static_cast<Proxy <A0> *>(gen->GetAddressOfArg(0))->value,
				static_cast<Proxy <A1> *>(gen->GetAddressOfArg(1))->value,
				static_cast<Proxy <A2> *>(gen->GetAddressOfArg(2))->value,
				static_cast<Proxy <A3> *>(gen->GetAddressOfArg(3))->value,
				static_cast<Proxy <A4> *>(gen->GetAddressOfArg(4))->value,
				static_cast<Proxy <A5> *>(gen->GetAddressOfArg(5))->value,
				static_cast<Proxy <A6> *>(gen->GetAddressOfArg(6))->value);
	}
};
template <typename T>
struct Id {
	template <T fn_ptr> AS_NAMESPACE_QUALIFIER asSFuncPtr  f(void) { return asFUNCTION(&Wrapper<T>::template f<fn_ptr>); }
	template <T fn_ptr> AS_NAMESPACE_QUALIFIER asSFuncPtr of(void) { return asFUNCTION(&ObjFirst<T>::template f<fn_ptr>); }
	template <T fn_ptr> AS_NAMESPACE_QUALIFIER asSFuncPtr ol(void) { return asFUNCTION(&ObjLast<T>::template f<fn_ptr>); }
};

template <typename T>
Id<T> id(T fn_ptr) { return Id<T>(); }

// On some versions of GNUC it is necessary to use the template keyword as disambiguator,
// on others the template keyword gives an error, hence the need for the following define.
// MSVC on the other hand seems to accept both with or without the template keyword.
#if defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 4))
	// GNUC 4.4.3 doesn't need the template keyword, and
	// hopefully upcoming versions won't need it either
	#define TMPL template
#else
	#define TMPL
#endif

#define WRAP_FN(name)             (::gw::id(name).TMPL f< name >())
#define WRAP_MFN(ClassType, name) (::gw::id(&ClassType::name).TMPL f< &ClassType::name >())
#define WRAP_OBJ_FIRST(name)      (::gw::id(name).TMPL of< name >())
#define WRAP_OBJ_LAST(name)       (::gw::id(name).TMPL ol< name >())

#define WRAP_FN_PR(name, Parameters, ReturnType)             asFUNCTION((::gw::Wrapper<ReturnType (*)Parameters>::TMPL f< name >))
#define WRAP_MFN_PR(ClassType, name, Parameters, ReturnType) asFUNCTION((::gw::Wrapper<ReturnType (ClassType::*)Parameters>::TMPL f< &ClassType::name >))
#define WRAP_OBJ_FIRST_PR(name, Parameters, ReturnType)      asFUNCTION((::gw::ObjFirst<ReturnType (*)Parameters>::TMPL f< name >))
#define WRAP_OBJ_LAST_PR(name, Parameters, ReturnType)       asFUNCTION((::gw::ObjLast<ReturnType (*)Parameters>::TMPL f< name >))

#define WRAP_CON(ClassType, Parameters) asFUNCTION((::gw::Constructor<ClassType Parameters>::f))
#define WRAP_DES(ClassType)             asFUNCTION((::gw::destroy<ClassType>))

} // end namespace gw

#endif
