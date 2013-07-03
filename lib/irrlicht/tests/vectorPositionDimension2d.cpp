// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

/** This test verifies that position2d and vector2d are interchangeable,
	and that they can convert from dimension2d */

#include "testUtils.h"

using namespace irr;
using namespace core;


template <class DIMENSION, class VECTOR, class POSITION, class T>
static bool doTest(void)
{
	bool result = true;

	DIMENSION dimension((T)99.9, (T)99.9);
	VECTOR vector(dimension);
	POSITION position(vector);
	DIMENSION dimension2(vector);

	result &= (vector == position);
	result &= (vector == dimension); // The conversion should be explicit.
	result &= (dimension2 == position);
	result &= (position == POSITION((T)99.9, (T)99.9));
	assert_log(result);

	dimension = (T)2 * position;
	result &= (dimension == VECTOR(2 * (T)99.9, 2 * (T)99.9));
	assert_log(result);

	dimension /= (T)2;
	result &= (dimension == POSITION((T)99.9, (T)99.9));
	assert_log(result);

	dimension += vector;
	result &= (dimension == VECTOR(2 * (T)99.9, 2 * (T)99.9));
	assert_log(result);

	dimension -= position;
	result &= (dimension == POSITION((T)99.9, (T)99.9));
	assert_log(result);

	position = dimension;
	result &= (position == VECTOR((T)99.9, (T)99.9));
	assert_log(result);

	vector += position;
	result &= (vector == POSITION(2 * (T)99.9, 2 * (T)99.9));
	assert_log(result);

	vector -= position;
	result &= (vector == dimension);
	assert_log(result);

	position *= (T)3.5;
	result &= (position == VECTOR((T)3.5 * (T)99.9, (T)3.5 * (T)99.9));
	assert_log(result);

	vector += dimension;
	result &= (vector == VECTOR(2 * (T)99.9, 2 * (T)99.9));
	assert_log(result);

	return result;
}

bool vectorPositionDimension2d(void)
{
	bool result = true;

	logTestString("vector,position,dimension test with s32\n\n");
	result &= doTest<dimension2di, vector2di, position2di, s32>();
	if (result)
		logTestString("tests passed\n\n");
	else
		logTestString("\ntests failed\n\n");
	logTestString("vector,position,dimension test with f32\n\n");
	result &= doTest<dimension2df, vector2df, position2df, f32>();
	if (result)
		logTestString("tests passed\n\n");
	else
		logTestString("\ntests failed\n\n");
	logTestString("vector,position,dimension test with f64\n\n");
	result &= doTest<dimension2d<f64>, vector2d<f64>, position2d<f64>, f64>();
	if (result)
		logTestString("tests passed\n\n");
	else
		logTestString("\ntests failed\n\n");

	return result;
}

