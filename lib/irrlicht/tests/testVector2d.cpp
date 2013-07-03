// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

template<class T>
static bool compareVectors(const core::vector2d<T> & compare,
						   const core::vector2d<T> & with)
{
	if (!compare.equals(with))
	{
		logTestString("\nERROR: vector2d %.16f, %.16f != vector2d %.16f, %.16f\n",
			(f64)compare.X, (f64)compare.Y, (f64)with.X, (f64)with.Y);
		assert_log(compare == with);
		return false;
	}

	return true;
}

template <class T>
static bool doTests()
{
	#define COMPARE_VECTORS(compare, with)\
		if(!compareVectors(compare, with)) return false;

	vector2d<T> vec(5, 5);
	vector2d<T> otherVec(10, 20);
	if(!equals(vec.getDistanceFrom(otherVec), (T)15.8113883))
	{
		logTestString("vector2d::getDistanceFrom() failed\n");
		assert_log(0);
		return false;
	}


	vec.rotateBy(45); // Test implicit (0, 0) center
	COMPARE_VECTORS(vec, vector2d<T>(0, (T)7.0710678118654755));

	vec.normalize();
	COMPARE_VECTORS(vec, vector2d<T>(0, (T)1.0000000461060017));

	vec.set(10, 10);
	vector2d<T> center(5, 5);
	vec.rotateBy(-5, center);
	// -5 means rotate clockwise slightly, so expect the X to increase
	// slightly and the Y to decrease slightly.
	COMPARE_VECTORS(vec, vector2d<T>((T)10.416752204197017, (T)9.5451947767204359));

	vec.set(5, 5);
	vec.normalize();
	COMPARE_VECTORS(vec, vector2d<T>((T)0.7071068137884140, (T)0.7071068137884140));

	vec.set(5, 5);
	otherVec.set(10, 20);

	logTestString("vector2d interpolation\n");
	vector2d<T> interpolated;
	(void)interpolated.interpolate(vec, otherVec, 0.f);
	COMPARE_VECTORS(interpolated, otherVec); // 0.f means all the second vector

	(void)interpolated.interpolate(vec, otherVec, 0.25f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)8.75, (T)16.25));

	(void)interpolated.interpolate(vec, otherVec, 0.75f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)6.25, (T)8.75));

	(void)interpolated.interpolate(vec, otherVec, 1.f);
	COMPARE_VECTORS(interpolated, vec); // 1.f means all the first vector


	interpolated = vec.getInterpolated(otherVec, 0.f);
	COMPARE_VECTORS(interpolated, otherVec); // 0.f means all the second vector

	interpolated = vec.getInterpolated(otherVec, 0.25f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)8.75, (T)16.25));

	interpolated = vec.getInterpolated(otherVec, 0.75f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)6.25, (T)8.75));

	interpolated = vec.getInterpolated(otherVec, 1.f);
	COMPARE_VECTORS(interpolated, vec); // 1.f means all the first vector


	logTestString("vector2d quadratic interpolation\n");
	vector2d<T> thirdVec(20, 10);
	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.f);
	COMPARE_VECTORS(interpolated, vec); // 0.f means all the 1st vector

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.25f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)7.8125, (T)10.9375));

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.5f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)11.25, (T)13.75));

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 0.75f);
	COMPARE_VECTORS(interpolated, vector2d<T>((T)15.3125, (T)13.4375));

	interpolated = vec.getInterpolated_quadratic(otherVec, thirdVec, 1.f);
	COMPARE_VECTORS(interpolated, thirdVec); // 1.f means all the 3rd vector

	// check if getAngle returns values matching those of the double precision version
	logTestString("vector2d getAngle\n");
	for (s32 i=0; i<200; ++i)
	{
		core::vector2d<T> tmp((T)-1, (T)(-100+i));
		core::vector2d<f64> ref(-1, -100+i);
		if (!equals(tmp.getAngle(),ref.getAngle(), 0.0003))
		{
			logTestString("\nERROR: angle %.16f != angle %.16f\n",
				tmp.getAngle(), ref.getAngle());
			return false;
		}
		f32 val = atan2f((float)tmp.Y, (float)tmp.X)*core::RADTODEG;
		if (val<=0)
			val=-val;
		else
			val=360-val;
		if (!equals((f32)tmp.getAngle(),val, 0.5f))
		{
			logTestString("\nERROR: angle %.16f != atan2 %.16f\n vector %.16f, %.16f\n",
				tmp.getAngle(), val, tmp.X, tmp.Y);
			return false;
		}
		tmp = core::vector2d<T>((T)1, (T)(-100+i));
		ref = core::vector2d<f64>(1, -100+i);
		if (!equals(tmp.getAngle(),ref.getAngle(), 0.0003))
		{
			logTestString("\nERROR: angle %.16f != angle %.16f\n",
				tmp.getAngle(), ref.getAngle());
			return false;
		}
		val = atan2f((f32)tmp.Y, (f32)tmp.X)*core::RADTODEG;
		if (val<=0)
			val=-val;
		else
			val=360-val;
		if (!equals((f32)tmp.getAngle(),val, 0.5f))
		{
			logTestString("\nERROR: angle %.16f != atan2 %.16f\n vector %.16f, %.16f\n",
				tmp.getAngle(), val, tmp.X, tmp.Y);
			return false;
		}
	}
	core::vector2d<T> tmp(0, -100);
	core::vector2d<f64> ref(0, -100);
	if (!equals(tmp.getAngle(),ref.getAngle()))
	{
		logTestString("\nERROR: angle %.16f != angle %.16f\n",
			tmp.getAngle(), ref.getAngle());
		return false;
	}
	tmp = core::vector2d<T>(0, 100);
	ref = core::vector2d<f64>(0, 100);
	if (!equals(tmp.getAngle(),ref.getAngle()))
	{
		logTestString("\nERROR: angle %.16f != angle %.16f\n",
			tmp.getAngle(), ref.getAngle());
		return false;
	}
	tmp = core::vector2d<T>(static_cast<T>(-1.53080559e-16), static_cast<T>(2.49999523));
	ref = core::vector2d<f64>(-1.53080559e-16, 2.49999523);
	if (!equals(tmp.getAngle(),ref.getAngle()))
	{
		logTestString("\nERROR: angle %.16f != angle %.16f\n",
			tmp.getAngle(), ref.getAngle());
		return false;
	}

	core::vector2d<T> zeroZero(0, 0);
	core::vector2d<T> oneOne(1, 1);
	// Check if comparing (0.0, 0.0) with (1.0, 1.0) returns false.
	if(zeroZero == oneOne)
	{
		logTestString("\nERROR: vector2d %.16f, %.16f == vector2d %.16f, %.16f\n",
			(f64)zeroZero.X, (f64)zeroZero.Y, (f64)oneOne.X, (f64)oneOne.Y);
		return false;
	}

	return true;
}

/** Test the functionality of vector2d<T>, particularly methods that
	involve calculations done using different precision than <T>.
	Note that all reference vector2d<T>s are creating using double precision
	values cast to (T), as we need to test <f64>. */
bool testVector2d(void)
{
	bool f32Success = doTests<f32>();
	if(f32Success)
		logTestString("vector2df tests passed\n\n");
	else
		logTestString("\n*** vector2df tests failed ***\n\n");

	bool f64Success = doTests<f64>();
	if(f64Success)
		logTestString("vector2d<f64> tests passed\n\n");
	else
		logTestString("\n*** vector2d<f64> tests failed ***\n\n");

	bool s32Success = doTests<s32>();
	if(s32Success)
		logTestString("vector2di tests passed\n\n");
	else
		logTestString("\n*** vector2di tests failed ***\n\n");

	return f32Success && f64Success && s32Success;
}

