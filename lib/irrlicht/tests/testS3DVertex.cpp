#include "testUtils.h"
#include "irrlicht.h"

using namespace irr;
using namespace core;
using namespace video;

// S3DVertex has operators which are used sometimes in sorting for example on loading meshes
bool testSorting()
{
	// Some test-values which did fail in the past.
	// See http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=33391&highlight=
	core::map<video::S3DVertex, int> testmap;
	video::S3DVertex v;
	v.Pos = core::vector3df(1.000000f, -1.000000f, 1.000000f);
	v.Normal = core::vector3df(0.577350f, -0.577350f, 0.577350f);
	v.Color = SColor(255,204,204,204);
	v.TCoords = core::vector2d<f32>(0.f, 0.f);
	testmap.insert(v, 0);

	v.Pos = core::vector3df(-1.000000f, -1.000000f, 1.000000f);
	v.Normal = core::vector3df(-0.577350f, -0.577350f, 0.577350f);
	v.Color = SColor(255,204,204,204);
	v.TCoords = core::vector2d<f32>(0.f, 0.f);
	testmap.insert(v, 1);

	v.Pos = core::vector3df(1.000000f, 1.000000f, 1.000000f);
	v.Normal = core::vector3df(0.577350f, 0.577350f, 0.577350f);
	v.Color = SColor(255,204,204,204);
	v.TCoords = core::vector2d<f32>(0.f, 0.f);
	testmap.insert(v, 2);

	v.Pos = core::vector3df(1.000000f, -1.000000f, 1.000000f);
	v.Normal = core::vector3df(0.577350f, -0.577350f, 0.577350f);
	v.Color = SColor(255,204,204,204);
	v.TCoords = core::vector2d<f32>(0.f, 0.f);

	core::map<video::S3DVertex, int>::Node* n = testmap.find(v);	// look for the vertex just inserted
	return n ? true : false;
}

bool testS3DVertex(void)
{
	bool result = true;
	result &= testSorting();
	return result;
}
