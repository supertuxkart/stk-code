// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

// These tests are also only called for f32 and f64, due to conversion problems
// in the respective methods.
template<class T>
static bool checkCollisions()
{
	aabbox3d<T> one(0,0,0,4,4,4);
	aabbox3d<T> two(2,2,2,4,4,4);

	if (two.getInterpolated(one, 1) != two)
	{
		logTestString("aabbox3d<T> interpolation wrong on 1\n");
		return false;
	}
	if (two.getInterpolated(one, 0) != one)
	{
		logTestString("aabbox3d<T> interpolation wrong on 0\n");
		return false;
	}
	aabbox3d<T> three(two.getInterpolated(one, 0.5f));
	if (two == one)
	{
		logTestString("aabbox3d<T> interpolation wrong on 0.5 (right)\n");
		return false;
	}
	if (two == three)
	{
		logTestString("aabbox3d<T> interpolation wrong on 0.5 (left)\n");
		return false;
	}
	three.reset(aabbox3d<T>(2,2,2,5,5,5));
	if (!two.isFullInside(one))
	{
		logTestString("small aabbox3d<T> is not fully inside\n");
		return false;
	}
	if (three.isFullInside(one))
	{
		logTestString("large aabbox3d<T> is fully inside\n");
		return false;
	}

	if (!two.intersectsWithBox(one))
	{
		logTestString("small aabbox3d<T> does not intersect\n");
		return false;
	}
	if (!three.intersectsWithBox(one))
	{
		logTestString("large aabbox3d<T> does not intersect\n");
		return false;
	}

	core::line3d<T> line(-2,-2,-2,2,2,2);
	if (!one.intersectsWithLine(line))
	{
		logTestString("aabbox3d<T> does not intersect with line(1)\n");
		return false;
	}
	line.end.set(2,2,10);
	if (!one.intersectsWithLine(line))
	{
		logTestString("aabbox3d<T> does not intersect with line(2)\n");
		return false;
	}
	line.end.set(0,2,10);
	if (one.intersectsWithLine(line))
	{
		logTestString("aabbox3d<T> does intersect with line(3)\n");
		return false;
	}
	return true;
}

template<class T>
static bool checkPoints()
{
	aabbox3d<T> one(-1,-2,-3,2,2,2);

	if (!one.isPointInside(core::vector3d<T>(-1,-2,-3)))
	{
		logTestString("isPointInside failed with min vertex\n");
		return false;
	}
	if (!one.isPointInside(core::vector3d<T>(-1,2,-3)))
	{
		logTestString("isPointInside failed with other min vertex\n");
		return false;
	}
	if (!one.isPointInside(core::vector3d<T>(2,-2,2)))
	{
		logTestString("isPointInside failed with other max vertex\n");
		return false;
	}
	if (!one.isPointInside(core::vector3d<T>(2,2,2)))
	{
		logTestString("isPointInside failed with max vertex\n");
		return false;
	}
	if (!one.isPointInside(core::vector3d<T>(0,0,0)))
	{
		logTestString("isPointInside failed with origin\n");
		return false;
	}
	if (!one.isPointInside(core::vector3d<T>((T)1.2,-1,1)))
	{
		logTestString("isPointInside failed with random point inside\n");
		return false;
	}
	if (one.isPointInside(core::vector3d<T>(-2,-2,-3)))
	{
		logTestString("isPointInside failed near min vertex\n");
		return false;
	}
	if (one.isPointInside(core::vector3d<T>(2,3,2)))
	{
		logTestString("isPointInside failed near max vertex\n");
		return false;
	}
	if (one.isPointInside(core::vector3d<T>(3,0,0)))
	{
		logTestString("isPointInside failed near origin\n");
		return false;
	}
	if (one.isPointInside(core::vector3d<T>((T)10.2,-1,1)))
	{
		logTestString("isPointInside failed with random point outside\n");
		return false;
	}
	if (one.isPointTotalInside(core::vector3d<T>(-1,-2,-3)))
	{
		logTestString("isPointTotalInside failed with min vertex\n");
		return false;
	}
	if (one.isPointTotalInside(core::vector3d<T>(-1,2,-3)))
	{
		logTestString("isPointTotalInside failed with other min vertex\n");
		return false;
	}
	if (one.isPointTotalInside(core::vector3d<T>(2,-2,2)))
	{
		logTestString("isPointTotalInside failed with other max vertex\n");
		return false;
	}
	if (one.isPointTotalInside(core::vector3d<T>(2,2,2)))
	{
		logTestString("isPointTotalInside failed with max vertex\n");
		return false;
	}
	if (!one.isPointTotalInside(core::vector3d<T>(0,0,0)))
	{
		logTestString("isPointTotalInside failed with origin\n");
		return false;
	}
	if (!one.isPointTotalInside(core::vector3d<T>((T)1.2,-1,1)))
	{
		logTestString("isPointTotalInside failed with random point inside\n");
		return false;
	}
	if (one.isPointTotalInside(core::vector3d<T>((T)10.2,-1,1)))
	{
		logTestString("isPointTotalInside failed with random point outside\n");
		return false;
	}

	core::plane3d<T> plane(core::vector3d<T>(0,0,-1), -10);
	if (one.classifyPlaneRelation(plane) != core::ISREL3D_BACK)
	{
		logTestString("box not behind\n");
		return false;
	}
	plane.D=0;
	if (one.classifyPlaneRelation(plane) != core::ISREL3D_CLIPPED)
	{
		logTestString("box not clipped\n");
		return false;
	}
	plane.D=10;
	if (one.classifyPlaneRelation(plane) != core::ISREL3D_FRONT)
	{
		logTestString("box not in front\n");
		return false;
	}
	return true;
}

template <class T>
static bool doTests()
{
	aabbox3d<T> empty;
	aabbox3d<T> one(-1,-1,-1,1,1,1);
	if (empty != one)
	{
		logTestString("default aabbox3d<T> wrong, or comparison failed\n");
		return false;
	}
	if (empty.getCenter() != core::vector3d<T>(0,0,0))
	{
		logTestString("default aabbox3d<T> has wrong Center\n");
		return false;
	}
	if (empty.getExtent() != core::vector3d<T>(2,2,2))
	{
		logTestString("default aabbox3d<T> has wrong Extent\n");
		return false;
	}
	if (empty.isEmpty())
	{
		logTestString("default aabbox3d<T> is empty\n");
		return false;
	}
	if (empty.getVolume() != 8)
	{
		logTestString("default aabbox3d<T> has wrong volume\n");
		return false;
	}
	if (empty.getArea() != 24)
	{
		logTestString("default aabbox3d<T> has wrong area\n");
		return false;
	}
	aabbox3d<T> two(core::vector3d<T>(-1,-1,-1),core::vector3d<T>(2,2,2));
	if (empty == two)
	{
		logTestString("empty aabbox3d<T> too large, or comparison failed\n");
		return false;
	}
	if (two.getCenter() != core::vector3d<T>((T)0.5,(T)0.5,(T)0.5))
	{
		logTestString("extended aabbox3d<T> has wrong Center\n");
		return false;
	}
	if (two.getExtent() != core::vector3d<T>(3,3,3))
	{
		logTestString("extended aabbox3d<T> has wrong Extent\n");
		return false;
	}
	if (two.isEmpty())
	{
		logTestString("extended aabbox3d<T> is empty\n");
		return false;
	}
	if (two.getVolume() != 27)
	{
		logTestString("extended aabbox3d<T> has wrong volume\n");
		return false;
	}
	if (two.getArea() != 54)
	{
		logTestString("extended aabbox3d<T> has wrong area\n");
		return false;
	}
	one.reset(1,1,1);
	if (one==empty)
	{
		logTestString("reset failed, or comparison failed\n");
		return false;
	}
	if (one.getCenter() != core::vector3d<T>(1,1,1))
	{
		logTestString("singular aabbox3d<T> has wrong Center\n");
		return false;
	}
	if (one.getExtent() != core::vector3d<T>(0,0,0))
	{
		logTestString("singular aabbox3d<T> has Extent\n");
		return false;
	}
	if (!one.isEmpty())
	{
		logTestString("empty aabbox3d<T> is not empty\n");
		return false;
	}
	if (one.getVolume() != 0)
	{
		logTestString("empty aabbox3d<T> has wrong volume\n");
		return false;
	}
	if (one.getArea() != 0)
	{
		logTestString("empty aabbox3d<T> has wrong area\n");
		return false;
	}
	one.addInternalPoint(core::vector3d<T>(-1,-1,-1));
	if (one!=empty)
	{
		logTestString("addInternalPoint failed, creating default bbox\n");
		return false;
	}
	one.reset(1,1,1);
	one.reset(empty);
	if (one!=empty)
	{
		logTestString("reset with bbox failed, creating default bbox\n");
		return false;
	}
	one.addInternalPoint(core::vector3d<T>(2,2,2));
	if (one != two)
	{
		logTestString("addInternalPoint for aabbox3d<T> failed.\n");
		return false;
	}
	one.addInternalBox(empty);
	if (one != two)
	{
		logTestString("addInternalBox with smaller box failed.\n");
		return false;
	}
	one.addInternalBox(two);
	if (one != two)
	{
		logTestString("addInternalBox with same box failed.\n");
		return false;
	}
	one.addInternalPoint(-1,-2,-3);
	two.addInternalPoint(-1,-2,-3);
	empty.addInternalBox(one);
	if (empty != two)
	{
		logTestString("addInternalBox with larger box failed\n");
		return false;
	}
	if (one.getCenter() != core::vector3d<T>((T)0.5,0,(T)-0.5))
	{
		logTestString("large aabbox3d<T> has wrong Center\n");
		return false;
	}
	if (one.getExtent() != core::vector3d<T>(3,4,5))
	{
		logTestString("large aabbox3d<T> has wrong Extent\n");
		return false;
	}
	if (one.isEmpty())
	{
		logTestString("large aabbox3d<T> is empty\n");
		return false;
	}
	if (one.getVolume() != 60)
	{
		logTestString("large aabbox3d<T> has wrong volume\n");
		return false;
	}
	if (one.getArea() != 94)
	{
		logTestString("large aabbox3d<T> has wrong area\n");
		return false;
	}
	if (!checkPoints<T>())
		return false;
	return true;
}


/** Test the functionality of aabbox3d<T>. */
bool testaabbox3d(void)
{
	bool f32Success = doTests<f32>();
	f32Success &= checkCollisions<f32>();
	if(f32Success)
		logTestString("aabbox3d<f32> tests passed\n\n");
	else
		logTestString("\n*** aabbox3d<f32> tests failed ***\n\n");

	bool f64Success = doTests<f64>();
	f64Success &= checkCollisions<f64>();
	if(f64Success)
		logTestString("aabbox3d<f64> tests passed\n\n");
	else
		logTestString("\n*** aabbox3d<f64> tests failed ***\n\n");

	bool s32Success = doTests<s32>();
	if(s32Success)
		logTestString("aabbox3d<s32> tests passed\n\n");
	else
		logTestString("\n*** aabbox3d<s32> tests failed ***\n\n");

	return f32Success && f64Success && s32Success;
}
