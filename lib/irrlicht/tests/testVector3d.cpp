// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

#define EQUAL_VECTORS(compare, with)\
	if(!equalVectors(cmp_equal<core::vector3d<T> >(compare), with)) {assert(false); return false;}

#define LESS_VECTORS(compare, with)\
	if(!equalVectors(cmp_less<core::vector3d<T> >(compare), with)) return false;

#define LESS_EQUAL_VECTORS(compare, with)\
	if(!equalVectors(cmp_less_equal<core::vector3d<T> >(compare), with)) return false;

#define MORE_VECTORS(compare, with)\
	if(!equalVectors(cmp_more<core::vector3d<T> >(compare), with)) return false;

#define MORE_EQUAL_VECTORS(compare, with)\
	if(!equalVectors(cmp_more_equal<core::vector3d<T> >(compare), with)) return false;

// check if the vector contains a NAN (a==b is guaranteed to return false in this case)
template<class T>
static bool is_nan(const core::vector3d<T> &vec )
{
	return ( !(vec.X == vec.X)
			|| !(vec.Y == vec.Y)
			|| !(vec.Z == vec.Z) );
}

template<class T>
struct cmp_less
{
	cmp_less(const T& a) : val(a) {}
	bool operator()(const T& other) const
	{
		return val<other;
	}
	const char* getName() const {return "<";}
	const T val;
};

template<class T>
struct cmp_less_equal
{
	cmp_less_equal(const T& a) : val(a) {}
	bool operator()(const T& other) const
	{
		return val<=other;
	}
	const char* getName() const {return "<=";}
	const T val;
};

template<class T>
struct cmp_more
{
	cmp_more(const T& a) : val(a) {}
	bool operator()(const T& other) const
	{
		return val>other;
	}
	const char* getName() const {return ">";}
	const T val;
};

template<class T>
struct cmp_more_equal
{
	cmp_more_equal(const T& a) : val(a) {}
	bool operator()(const T& other) const
	{
		return val>=other;
	}
	const char* getName() const {return ">=";}
	const T val;
};

template<class T>
struct cmp_equal
{
	cmp_equal(const T& a) : val(a) {}
	bool operator()(const T& other) const
	{
		return val.equals(other);
	}
	const char* getName() const {return "==";}
	const T val;
};

template<class S, class T>
static bool equalVectors(const S& compare,
			   const core::vector3d<T> & with)
{
	if (!compare(with))
	{
		logTestString("\nERROR: vector3d %.16f, %.16f, %.16f %s vector3d %.16f, %.16f, %.16f\n",
			(f64)compare.val.X, (f64)compare.val.Y, (f64)compare.val.Z, compare.getName(),
			(f64)with.X, (f64)with.Y, (f64)with.Z);
		assert_log(compare(with));
		return false;
	}

	return true;
}

template <class T>
static bool checkInterpolation()
{
	core::vector3d<T> vec(5, 5, 0);
	core::vector3d<T> otherVec(10, 20, 40);

	vector3d<T> interpolated;
	(void)interpolated.interpolate(vec, otherVec, 0.f);
	EQUAL_VECTORS(interpolated, otherVec); // 0.f means all the second vector

	(void)interpolated.interpolate(vec, otherVec, 0.25f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)8.75, (T)16.25, 30));

	(void)interpolated.interpolate(vec, otherVec, 0.75f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)6.25, (T)8.75, 10));

	(void)interpolated.interpolate(vec, otherVec, 1.f);
	EQUAL_VECTORS(interpolated, vec); // 1.f means all the first vector


	interpolated = vec.getInterpolated(otherVec, 0.f);
	EQUAL_VECTORS(interpolated, otherVec); // 0.f means all the second vector

	interpolated = vec.getInterpolated(otherVec, 0.25f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)8.75, (T)16.25, 30));

	interpolated = vec.getInterpolated(otherVec, 0.75f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)6.25, (T)8.75, 10));

	interpolated = vec.getInterpolated(otherVec, 1.f);
	EQUAL_VECTORS(interpolated, vec); // 1.f means all the first vector


	vector3d<T> thirdVec(20, 10, -30);
	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.f);
	EQUAL_VECTORS(interpolated, vec); // 0.f means all the 1st vector

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.25f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)7.8125, (T)10.9375, (T)13.125));

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.5f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)11.25, (T)13.75, (T)12.5));

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.75f);
	EQUAL_VECTORS(interpolated, vector3d<T>((T)15.3125, (T)13.4375, (T)-1.875));

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 1.f);
	EQUAL_VECTORS(interpolated, thirdVec); // 1.f means all the 3rd vector
	return true;
}

template <class T>
static bool checkAngleCalculations()
{
	core::vector3d<T> vec(5, 5, 0);
	EQUAL_VECTORS(vec.getHorizontalAngle(), vector3d<T>(315, (T)90.0, 0));
	EQUAL_VECTORS(vec.getSphericalCoordinateAngles(), vector3d<T>((T)45.0, 0, 0));
	return true;
}

template <class T>
static bool checkRotations()
{
	core::vector3d<T> vec(5, 5, 0);
	vector3d<T> center(0, 0, 0);

	vec.rotateXYBy(45, center);
	EQUAL_VECTORS(vec, vector3d<T>(0, (T)7.0710678118654755, 0));

	vec.normalize();
	// TODO: This breaks under Linux/gcc due to FP differences, but is no bug
	if (((T)0.5f)>0.f)
		EQUAL_VECTORS(vec, vector3d<T>(0, (T)1.0, 0));

	vec.set(10, 10, 10);
	center.set(5, 5, 10);
	vec.rotateXYBy(-5, center);
	// -5 means rotate clockwise slightly, so expect the X to increase
	// slightly and the Y to decrease slightly.
	EQUAL_VECTORS(vec, vector3d<T>((T)10.416752204197017, (T)9.5451947767204359, 10));

	vec.set(10, 10, 10);
	center.set(5, 10, 5);
	vec.rotateXZBy(-5, center);
	EQUAL_VECTORS(vec, vector3d<T>((T)10.416752204197017, 10, (T)9.5451947767204359));

	vec.set(10, 10, 10);
	center.set(10, 5, 5);
	vec.rotateYZBy(-5, center);
	EQUAL_VECTORS(vec, vector3d<T>(10, (T)10.416752204197017, (T)9.5451947767204359));

	vec.set(5, 5, 0);
	vec.normalize();
	EQUAL_VECTORS(vec, vector3d<T>((T)0.70710681378841400, (T)0.70710681378841400, 0));
	return true;
}

template <class T>
static bool doTests()
{
	vector3d<T> vec(-5, 5, 0);
	vector3d<T> otherVec((T)-5.1, 5, 0);
	if(!vec.equals(otherVec, (T)0.1))
	{
		logTestString("vector3d::equals failed\n");
		assert_log(0);
		return false;
	}

	vec.set(5, 5, 0);
	otherVec.set(10, 20, 0);
	if(!equals(vec.getDistanceFrom(otherVec), (T)15.8113883))
	{
		logTestString("vector3d::getDistanceFrom() failed\n");
		assert_log(0);
		return false;
	}

	if (!checkRotations<T>())
		return false;

	if (!checkInterpolation<T>())
		return false;

	if (!checkAngleCalculations<T>())
		return false;

	vec.set(0,0,0);
	vec.setLength(99);
	if ( is_nan(vec) )
		return false;

	core::vector3d<T> zeroZero(0, 0, 0);
	core::vector3d<T> oneOne(1, 1, 1);
	// Check if comparing (0.0, 0.0, 0.0) with (1.0, 1.0, 1.0) returns false.
	if(zeroZero == oneOne)
	{
		logTestString("\nERROR: vector3d %.16f, %.16f, %.16f == vector3d %.16f, %.16f, %.16f\n",
			(f64)zeroZero.X, (f64)zeroZero.Y, (f64)zeroZero.Z,
			(f64)oneOne.X, (f64)oneOne.Y, (f64)oneOne.Z);
		return false;
	}

	vec.set(5, 5, 0);

	otherVec.set(10, 20, 40);
	LESS_VECTORS(vec, otherVec);
	LESS_EQUAL_VECTORS(vec, otherVec);
	MORE_VECTORS(otherVec, vec);
	MORE_EQUAL_VECTORS(otherVec, vec);

	vec.set(-1,-1,1);
	otherVec.set(1,-1,1);
	LESS_VECTORS(vec, otherVec);
	LESS_EQUAL_VECTORS(vec, otherVec);
	MORE_VECTORS(otherVec, vec);
	MORE_EQUAL_VECTORS(otherVec, vec);

	LESS_EQUAL_VECTORS(vec, vec);
	MORE_EQUAL_VECTORS(vec, vec);

	return true;
}


/** Test the functionality of vector3d<T>, particularly methods that
	involve calculations done using different precision than <T>.
	Note that all reference vector3d<T>s are creating using double precision
	values cast to (T), as we need to test <f64>. */
bool testVector3d(void)
{
	const bool f32Success = doTests<f32>();
	if (f32Success)
		logTestString("vector3df tests passed\n\n");
	else
		logTestString("\n*** vector3df tests failed ***\n\n");

	const bool f64Success = doTests<f64>();
	if (f64Success)
		logTestString("vector3d<f64> tests passed\n\n");
	else
		logTestString("\n*** vector3d<f64> tests failed ***\n\n");

	const bool s32Success = doTests<s32>();
	if (s32Success)
		logTestString("vector3di tests passed\n\n");
	else
		logTestString("\n*** vector3di tests failed ***\n\n");

	return f32Success && f64Success && s32Success;
}

