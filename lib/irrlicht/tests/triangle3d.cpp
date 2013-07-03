// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"
#include <irrlicht.h>

using namespace irr;
using namespace core;

template<class T>
static bool testGetIntersectionWithLine(core::triangle3d<T>& triangle, const core::line3d<T>& ray)
{
	bool allExpected=true;
	const vector3d<T> linevect = ray.getVector().normalize();
	vector3d<T> intersection;
	// When we just raise Y parallel to the ray then either all should fail or all succeed (the latter in our case).
	for (u32 i=0; i<100; ++i)
	{
		if (!triangle.getIntersectionOfPlaneWithLine(ray.start, linevect, intersection))
		{
			allExpected=false;
			logTestString("triangle3d plane test %d failed\n", i);
		}
		//logTestString("intersection: %f %f %f\n", intersection.X, intersection.Y, intersection.Z);
		if (!triangle.isPointInsideFast(intersection))
		{
			allExpected=false;
			logTestString("triangle3d fast point test %d failed\n", i);
		}
		if (!triangle.isPointInside(intersection))
		{
			allExpected=false;
			logTestString("triangle3d point test %d failed\n", i);
		}

		if (!triangle.getIntersectionWithLine(ray.start, linevect, intersection))
		{
			allExpected=false;
			logTestString("triangle3d tri test %d failed\n", i);
		}

		triangle.pointB.Y += 1;
	}
	return allExpected;
}

// modifying the same triangle in diverse ways get some more test-cases automatically
template<class T>
static bool stageModifications(int stage, triangle3d<T>& triangle)
{
	switch ( stage )
	{
		case 0:
			return true;
		case 1:
			swap(triangle.pointB, triangle.pointC);
			return true;
		case 2:
			swap(triangle.pointA, triangle.pointC);
			return true;
		case 3:
			triangle.pointA.Z += 1000;
			triangle.pointB.Z += 1000;
			triangle.pointC.Z += 1000;
			return true;
		case 4:
			swap(triangle.pointA.Y, triangle.pointA.Z);
			swap(triangle.pointB.Y, triangle.pointB.Z);
			swap(triangle.pointC.Y, triangle.pointC.Z);
			return true;
	}
	return false;
}

template<class T>
static void stageModifications(int stage, vector3d<T>& point)
{
	switch ( stage )
	{
		case 3:
			point.Z += 1000;
			break;
		case 4:
			swap(point.Y, point.Z);
			break;
	}
}

template<class T>
static bool isPointInside(triangle3d<T> triangleOrig, bool testIsInside, bool testIsInsideFast)
{
	bool allExpected=true;

	array< vector3d<T> > pointsInside;
	pointsInside.push_back( vector3d<T>(0,0,0) );
	pointsInside.push_back( (triangleOrig.pointA + triangleOrig.pointB + triangleOrig.pointC) / 3 );
	pointsInside.push_back( (triangleOrig.pointA + triangleOrig.pointB)/2 + vector3d<T>(0,1,0) );
	pointsInside.push_back( (triangleOrig.pointA + triangleOrig.pointC)/2 + vector3d<T>(1,0,0) );
	pointsInside.push_back( (triangleOrig.pointB + triangleOrig.pointC)/2 - vector3d<T>(1,0,0) );

	for (u32 stage=0; ; ++stage)
	{
		triangle3d<T> triangle = triangleOrig;
		if ( !stageModifications(stage, triangle) )
			break;

		for ( u32 i=0; i < pointsInside.size(); ++i )
		{
			vector3d<T> point = pointsInside[i];
			stageModifications(stage, point);

			if ( testIsInside )
			{
				allExpected &= triangle.isPointInside( point );
				if ( !allExpected )
				{
					logTestString("triangle3d::isPointInside pointsInside test failed in stage %d point %d\n", stage, i);
					return false;
				}
			}

			if ( testIsInsideFast )
			{
				allExpected &= triangle.isPointInsideFast( point );
				if ( !allExpected )
				{
					logTestString("triangle3d::isPointInsideFast pointsInside test failed in stage %d point %d\n", stage, i);
					return false;
				}
			}
		}
	}

	array< vector3d<T> > pointsOutside;
	pointsOutside.push_back( triangleOrig.pointA - vector3d<T>(1,0,0) );
	pointsOutside.push_back( triangleOrig.pointA - vector3d<T>(0,1,0) );
	pointsOutside.push_back( triangleOrig.pointB + vector3d<T>(1,0,0) );
	pointsOutside.push_back( triangleOrig.pointB - vector3d<T>(0,1,0) );
	pointsOutside.push_back( triangleOrig.pointC - vector3d<T>(1,0,0) );
	pointsOutside.push_back( triangleOrig.pointC + vector3d<T>(1,0,0) );
	pointsOutside.push_back( triangleOrig.pointC + vector3d<T>(0,1,0) );
	pointsOutside.push_back( (triangleOrig.pointA + triangleOrig.pointB)/2 - vector3d<T>(0,1,0) );
	pointsOutside.push_back( (triangleOrig.pointA + triangleOrig.pointC)/2 - vector3d<T>(1,0,0) );
	pointsOutside.push_back( (triangleOrig.pointB + triangleOrig.pointC)/2 + vector3d<T>(1,0,0) );

	for (u32 stage=0; ; ++stage)
	{
		triangle3d<T> triangle = triangleOrig;
		if ( !stageModifications(stage, triangle) )
			break;

		for ( u32 i=0; i < pointsOutside.size(); ++i )
		{
			vector3d<T> point = pointsOutside[i];
			stageModifications(stage, point);

			if ( testIsInside )
			{
				allExpected &= !triangle.isPointInside( point );
				if ( !allExpected )
				{
					logTestString("triangle3d::isPointInside pointsOutside test failed in stage %d point %d\n", stage, i);
					return false;
				}
			}

			if ( testIsInsideFast )
			{
				allExpected &= !triangle.isPointInsideFast( point );
				if ( !allExpected )
				{
					logTestString("triangle3d::isPointInsideFast pointsOutside test failed in stage %d point %d\n", stage, i);
					return false;
				}
			}
		}
	}

	array< vector3d<T> > pointsBorder;
	pointsBorder.push_back( triangleOrig.pointA );
	pointsBorder.push_back( triangleOrig.pointB );
	pointsBorder.push_back( triangleOrig.pointC );
	pointsBorder.push_back( (triangleOrig.pointA + triangleOrig.pointB)/2 );
	pointsBorder.push_back( (triangleOrig.pointA + triangleOrig.pointC)/2 );
	pointsBorder.push_back( (triangleOrig.pointB + triangleOrig.pointC)/2 );

	for (u32 stage=0; ; ++stage)
	{
		triangle3d<T> triangle = triangleOrig;
		if ( !stageModifications(stage, triangle) )
			break;

		for ( u32 i=0; i < pointsBorder.size(); ++i )
		{
			vector3d<T> point = pointsBorder[i];
			stageModifications(stage, point);

			if ( testIsInside )
			{
				allExpected &= triangle.isPointInside( point );
				if ( !allExpected )
				{
					logTestString("triangle3d::isPointInside pointsBorder test failed in stage %d point %d\n", stage, i);
					return false;
				}
			}

			if ( testIsInsideFast )
			{
				allExpected &= triangle.isPointInsideFast( point );
				if ( !allExpected )
				{
					logTestString("triangle3d::isPointInsideFast pointsBorder test failed in stage %d point %d\n", stage, i);
					return false;
				}
			}
		}
	}

	return allExpected;
}

// Test the functionality of triangle3d<T>
bool testTriangle3d(void)
{
	bool allExpected = true;

	logTestString("Test getIntersectionWithLine with f32\n");
	{
		triangle3df triangle(
				vector3df(11300.f, 129.411758f, 200.f),
				vector3df(11200.f, 94.117645f, 300.f),
				vector3df(11300.f, 129.411758f, 300.f));
		line3df ray;
		ray.start = vector3df(11250.f, 329.f, 250.f);
		ray.end = vector3df(11250.f, -1000.f, 250.f);
		allExpected &= testGetIntersectionWithLine(triangle, ray);
	}
	logTestString("Test getIntersectionWithLine with f64\n");
	{
		triangle3d<f64> triangle(
				vector3d<f64>(11300., 129.411758, 200.),
				vector3d<f64>(11200., 94.117645, 300.),
				vector3d<f64>(11300., 129.411758, 300.));
		line3d<f64> ray;
		ray.start = vector3d<f64>(11250., 329., 250.);
		ray.end = vector3d<f64>(11250., -1000., 250.);
		allExpected &= testGetIntersectionWithLine(triangle, ray);
	}

	bool testEigen = triangle3di(vector3di(250, 0, 0), vector3di(0, 0, 500), vector3di(500, 0, 500)).isPointInside(vector3di(300,0,300));
	if ( !testEigen )	// test from Eigen from here: http://irrlicht.sourceforge.net/forum/viewtopic.php?f=7&t=44372&p=254331#p254331
		logTestString("Test isPointInside fails with integers\n");
	allExpected &= testEigen;

	logTestString("Test isPointInside with f32\n");
	{
		triangle3d<f32> t(vector3d<f32>(-1000,-1000,0), vector3d<f32>(1000,-1000,0), vector3d<f32>(0,1000,0));
		allExpected &= isPointInside(t, true, true);
	}

	logTestString("Test isPointInside with f64\n");
	{
		triangle3d<f64> t(vector3d<f64>(-1000,-1000,0), vector3d<f64>(1000,-1000,0), vector3d<f64>(0,1000,0));
		allExpected &= isPointInside(t, true, true);
	}

	logTestString("Test isPointInside with s32\n");
	{
		triangle3d<s32> t(vector3d<s32>(-1000,-1000,0), vector3d<s32>(1000,-1000,0), vector3d<s32>(0,1000,0));
		allExpected &= isPointInside(t, false, true);
	}

	if(allExpected)
		logTestString("\nAll tests passed\n");
	else
		logTestString("\nFAIL!\n");

	return allExpected;
}

