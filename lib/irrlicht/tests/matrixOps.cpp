// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace
{

// Basic tests for identity matrix
bool identity(void)
{
	bool result = true;
	matrix4 m;
	// Check default init
	result &= (m==core::IdentityMatrix);
	result &= (core::IdentityMatrix==m);
	assert_log(result);
	// Since the last test can be made with isDefinitelyIdentityMatrix we set it to false here
	m.setDefinitelyIdentityMatrix(false);
	result &= (m==core::IdentityMatrix);
	result &= (core::IdentityMatrix==m);
	assert_log(result);
	// also equals should see this
	result &= m.equals(core::IdentityMatrix);
	result &= core::IdentityMatrix.equals(m);
	assert_log(result);
	// Check inequality
	m[12]=5.f;
	result &= (m!=core::IdentityMatrix);
	result &= (core::IdentityMatrix!=m);
	result &= !m.equals(core::IdentityMatrix);
	result &= !core::IdentityMatrix.equals(m);
	assert_log(result);

	// Test multiplication
	result &= (m==(core::IdentityMatrix*m));
	result &= m.equals(core::IdentityMatrix*m);
	result &= (m==(m*core::IdentityMatrix));
	result &= m.equals(m*core::IdentityMatrix);
	assert_log(result);

	return result;
}

// Test rotations
bool transformations(void)
{
	bool result = true;
	matrix4 m, s;
	m.setRotationDegrees(core::vector3df(30,40,50));
	s.setScale(core::vector3df(2,3,4));
	m *= s;
	m.setTranslation(core::vector3df(5,6,7));
	result &= (core::vector3df(5,6,7).equals(m.getTranslation()));
	assert_log(result);
	result &= (core::vector3df(2,3,4).equals(m.getScale()));
	assert_log(result);
	core::vector3df newRotation = m.getRotationDegrees();
	result &= (core::vector3df(30,40,50).equals(newRotation, 0.000004f));
	assert_log(result);
	m.setRotationDegrees(vector3df(90.0001f, 270.85f, 180.0f));
	s.setRotationDegrees(vector3df(0,0, 0.860866f));
	m *= s;
	newRotation = m.getRotationDegrees();
	result &= (core::vector3df(0,270,270).equals(newRotation, 0.0001f));
	assert_log(result);
	m.setRotationDegrees(vector3df(270.0f, 89.8264f, 0.000100879f));
	s.setRotationDegrees(vector3df(0,0, 0.189398f));
	m *= s;
	newRotation = m.getRotationDegrees();
	result &= (core::vector3df(0,90,90).equals(newRotation, 0.0001f));
	assert_log(result);
	m.setRotationDegrees(vector3df(270.0f, 89.0602f, 359.999f));
	s.setRotationDegrees(vector3df(0,0, 0.949104f));
	m *= s;
	newRotation = m.getRotationDegrees();
	result &= (core::vector3df(0,90,89.999f).equals(newRotation));
	assert_log(result);

	return result;
}

// Test rotations
bool rotations(void)
{
	bool result = true;
	matrix4 rot1,rot2,rot3,rot4,rot5;
	core::vector3df vec1(1,2,3),vec12(1,2,3);
	core::vector3df vec2(-5,0,0),vec22(-5,0,0);
	core::vector3df vec3(20,0,-20), vec32(20,0,-20);
	// Make sure the matrix multiplication and rotation application give same results
	rot1.setRotationDegrees(core::vector3df(90,0,0));
	rot2.setRotationDegrees(core::vector3df(0,90,0));
	rot3.setRotationDegrees(core::vector3df(0,0,90));
	rot4.setRotationDegrees(core::vector3df(90,90,90));
	rot5 = rot3*rot2*rot1;
	result &= (rot4.equals(rot5, ROUNDING_ERROR_f32));
	assert_log(result);
	rot4.transformVect(vec1);rot5.transformVect(vec12);
	rot4.transformVect(vec2);rot5.transformVect(vec22);
	rot4.transformVect(vec3);rot5.transformVect(vec32);
	result &= (vec1.equals(vec12));
	result &= (vec2.equals(vec22));
	result &= (vec3.equals(vec32));
	assert_log(result);

	vec1.set(1,2,3);vec12.set(1,2,3);
	vec2.set(-5,0,0);vec22.set(-5,0,0);
	vec3.set(20,0,-20);vec32.set(20,0,-20);
	rot1.setRotationDegrees(core::vector3df(45,0,0));
	rot2.setRotationDegrees(core::vector3df(0,45,0));
	rot3.setRotationDegrees(core::vector3df(0,0,45));
	rot4.setRotationDegrees(core::vector3df(45,45,45));
	rot5 = rot3*rot2*rot1;
	result &= (rot4.equals(rot5, ROUNDING_ERROR_f32));
	assert_log(result);
	rot4.transformVect(vec1);rot5.transformVect(vec12);
	rot4.transformVect(vec2);rot5.transformVect(vec22);
	rot4.transformVect(vec3);rot5.transformVect(vec32);
	result &= (vec1.equals(vec12));
	result &= (vec2.equals(vec22));
	result &= (vec3.equals(vec32, 2*ROUNDING_ERROR_f32));
	assert_log(result);

	vec1.set(1,2,3);vec12.set(1,2,3);
	vec2.set(-5,0,0);vec22.set(-5,0,0);
	vec3.set(20,0,-20);vec32.set(20,0,-20);
	rot1.setRotationDegrees(core::vector3df(-60,0,0));
	rot2.setRotationDegrees(core::vector3df(0,-60,0));
	rot3.setRotationDegrees(core::vector3df(0,0,-60));
	rot4.setRotationDegrees(core::vector3df(-60,-60,-60));
	rot5 = rot3*rot2*rot1;
	result &= (rot4.equals(rot5, ROUNDING_ERROR_f32));
	assert_log(result);
	rot4.transformVect(vec1);rot5.transformVect(vec12);
	rot4.transformVect(vec2);rot5.transformVect(vec22);
	rot4.transformVect(vec3);rot5.transformVect(vec32);
	result &= (vec1.equals(vec12));
	result &= (vec2.equals(vec22));
	// this one needs higher tolerance due to rounding issues
	result &= (vec3.equals(vec32, 0.000002f));
	assert_log(result);

	vec1.set(1,2,3);vec12.set(1,2,3);
	vec2.set(-5,0,0);vec22.set(-5,0,0);
	vec3.set(20,0,-20);vec32.set(20,0,-20);
	rot1.setRotationDegrees(core::vector3df(113,0,0));
	rot2.setRotationDegrees(core::vector3df(0,-27,0));
	rot3.setRotationDegrees(core::vector3df(0,0,193));
	rot4.setRotationDegrees(core::vector3df(113,-27,193));
	rot5 = rot3*rot2*rot1;
	result &= (rot4.equals(rot5, ROUNDING_ERROR_f32));
	assert_log(result);
	rot4.transformVect(vec1);rot5.transformVect(vec12);
	rot4.transformVect(vec2);rot5.transformVect(vec22);
	rot4.transformVect(vec3);rot5.transformVect(vec32);
	// these ones need higher tolerance due to rounding issues
	result &= (vec1.equals(vec12, 0.000002f));
	assert_log(result);
	result &= (vec2.equals(vec22));
	assert_log(result);
	result &= (vec3.equals(vec32, 0.000002f));
	assert_log(result);

	rot1.setRotationDegrees(core::vector3df(0,0,34));
	rot2.setRotationDegrees(core::vector3df(0,43,0));
	vec1=(rot2*rot1).getRotationDegrees();
	result &= (vec1.equals(core::vector3df(27.5400505f, 34.4302292f, 42.6845398f), 0.000002f));
	assert_log(result);

	// corner cases
    rot1.setRotationDegrees(irr::core::vector3df(180.0f, 0.f, 0.f));
    vec1=rot1.getRotationDegrees();
	result &= (vec1.equals(core::vector3df(180.0f, 0.f, 0.f), 0.000002f));
	assert_log(result);
    rot1.setRotationDegrees(irr::core::vector3df(0.f, 180.0f, 0.f));
    vec1=rot1.getRotationDegrees();
	result &= (vec1.equals(core::vector3df(180.0f, 360, 180.0f), 0.000002f));
	assert_log(result);
    rot1.setRotationDegrees(irr::core::vector3df(0.f, 0.f, 180.0f));
    vec1=rot1.getRotationDegrees();
	result &= (vec1.equals(core::vector3df(0.f, 0.f, 180.0f), 0.000002f));
	assert_log(result);

	rot1.makeIdentity();
	rot1.setRotationDegrees(core::vector3df(270.f,0,0));
	rot2.makeIdentity();
	rot2.setRotationDegrees(core::vector3df(-90.f,0,0));
	vec1=(rot1*rot2).getRotationDegrees();
	result &= (vec1.equals(core::vector3df(180.f, 0.f, 0.0f)));
	assert_log(result);

	return result;
}

// Test isOrthogonal
bool isOrthogonal(void)
{
	matrix4 rotationMatrix;
	if (!rotationMatrix.isOrthogonal())
	{
		logTestString("irr::core::matrix4::isOrthogonal() failed with Identity.\n");
		return false;
	}

	rotationMatrix.setRotationDegrees(vector3df(90, 0, 0));
	if (!rotationMatrix.isOrthogonal())
	{
		logTestString("irr::core::matrix4::isOrthogonal() failed with rotation.\n");
		return false;
	}

	matrix4 translationMatrix;
	translationMatrix.setTranslation(vector3df(0, 3, 0));
	if (translationMatrix.isOrthogonal())
	{
		logTestString("irr::core::matrix4::isOrthogonal() failed with translation.\n");
		return false;
	}

	matrix4 scaleMatrix;
	scaleMatrix.setScale(vector3df(1, 2, 3));
	if (!scaleMatrix.isOrthogonal())
	{
		logTestString("irr::core::matrix4::isOrthogonal() failed with scale.\n");
		return false;
	}

	return true;
}

bool checkMatrixRotation(irr::core::matrix4& m, const vector3df& vector, const vector3df& expectedResult)
{
	vector3df v(vector);
	m.rotateVect(v);
	if ( expectedResult.equals(v) )
		return true;
	logTestString("checkMatrixRotation failed for vector %f %f %f. Expected %f %f %f, got %f %f %f \n"
			, vector.X, vector.Y, vector.Z, expectedResult.X, expectedResult.Y, expectedResult.Z, v.X, v.Y, v.Z);
	logTestString("matrix: ");
	for ( int i=0; i<16; ++i )
		logTestString("%.2f ", m[i]);
	logTestString("\n");	

	return false;
}

bool setRotationAxis()
{
	matrix4 m;
	vector3df v;
	
	// y up, x right, z depth (as usual)

	// y rotated around x-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(1,0,0)), vector3df(0,1,0), vector3df(0, 0, 1)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	if ( !checkMatrixRotation( m.setRotationAxisRadians(180.f*DEGTORAD, vector3df(1,0,0)), vector3df(0,1,0), vector3df(0, -1, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	
	// y rotated around negative x-axis
	m.makeIdentity();
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(-1,0,0)), vector3df(0,1,0), vector3df(0, 0, -1)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	
	// x rotated around x-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(1,0,0)), vector3df(1,0,0), vector3df(1, 0, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}

	// x rotated around y-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,1,0)), vector3df(1,0,0), vector3df(0, 0, -1)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	if ( !checkMatrixRotation( m.setRotationAxisRadians(180.f*DEGTORAD, vector3df(0,1,0)), vector3df(1,0,0), vector3df(-1, 0, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	
	// x rotated around negative y-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,-1,0)), vector3df(1,0,0), vector3df(0, 0, 1)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	} 
	
	// y rotated around y-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,1,0)), vector3df(0,1,0), vector3df(0, 1, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}

	// x rotated around z-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,0,1)), vector3df(1,0,0), vector3df(0, 1, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	if ( !checkMatrixRotation( m.setRotationAxisRadians(180.f*DEGTORAD, vector3df(0,0,1)), vector3df(1,0,0), vector3df(-1, 0, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}

	// x rotated around negative z-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,0,-1)), vector3df(1,0,0), vector3df(0, -1, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	
	// y rotated around z-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,0,1)), vector3df(0,1,0), vector3df(-1, 0, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	if ( !checkMatrixRotation( m.setRotationAxisRadians(180.f*DEGTORAD, vector3df(0,0,1)), vector3df(0,1,0), vector3df(0, -1, 0)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}
	
	// z rotated around z-axis
	if ( !checkMatrixRotation( m.setRotationAxisRadians(90.f*DEGTORAD, vector3df(0,0,1)), vector3df(0,0,1), vector3df(0, 0, 1)) )
	{
		logTestString("%s:%d", __FILE__, __LINE__);
		return false;
	}

	
	return true;
}

// just calling each function once to find compile problems
void calltest()
{
	matrix4 mat;
	matrix4 mat2(mat);
	f32& f1 = mat(0,0);
	const f32& f2 = mat(0,0);
	f32& f3 = mat[0];
	const f32& f4 = mat[0];
	mat = mat;
	mat = 1.f;
	const f32 * pf1 = mat.pointer();
	f32 * pf2 = mat.pointer();
	bool b = mat == mat2;
	b = mat != mat2;
	mat = mat + mat2;
	mat += mat2;
	mat = mat - mat2;
	mat -= mat2;
	mat.setbyproduct(mat, mat2);
	mat.setbyproduct_nocheck(mat, mat2);
	mat = mat * mat2;
	mat *= mat2;
	mat = mat * 10.f;
	mat *= 10.f;
	mat.makeIdentity();
	b = mat.isIdentity();
	b = mat.isOrthogonal();
	b = mat.isIdentity_integer_base ();
	mat.setTranslation(vector3df(1.f, 1.f, 1.f) );
	vector3df v1 = mat.getTranslation();
	mat.setInverseTranslation(vector3df(1.f, 1.f, 1.f) );
	mat.setRotationRadians(vector3df(1.f, 1.f, 1.f) );
	mat.setRotationDegrees(vector3df(1.f, 1.f, 1.f) );
	vector3df v2 = mat.getRotationDegrees();
	mat.setInverseRotationRadians(vector3df(1.f, 1.f, 1.f) );
	mat.setInverseRotationDegrees(vector3df(1.f, 1.f, 1.f) );
	mat.setRotationAxisRadians(1.f, vector3df(1.f, 1.f, 1.f) );
	mat.setScale(vector3df(1.f, 1.f, 1.f) );
	mat.setScale(1.f);
	vector3df v3 = mat.getScale();
	mat.inverseTranslateVect(v1);
	mat.inverseRotateVect(v1);
	mat.rotateVect(v1);
	mat.rotateVect(v1, v2);
	f32 fv3[3];
	mat.rotateVect(fv3, v1);
	mat.transformVect(v1);
	mat.transformVect(v1, v1);
	f32 fv4[4];
	mat.transformVect(fv4, v1);
	mat.transformVec3(fv3, fv3);
	mat.translateVect(v1);
	plane3df p1;
	mat.transformPlane(p1);
	mat.transformPlane(p1, p1);
	aabbox3df bb1;
	mat.transformBox(bb1);
	mat.transformBoxEx(bb1);
	mat.multiplyWith1x4Matrix(fv4);
	mat.makeInverse();
	b = mat.getInversePrimitive(mat2);
	b = mat.getInverse(mat2);
	mat.buildProjectionMatrixPerspectiveFovRH(1.f, 1.f, 1.f, 1000.f);
	mat.buildProjectionMatrixPerspectiveFovLH(1.f, 1.f, 1.f, 1000.f);
	mat.buildProjectionMatrixPerspectiveFovInfinityLH(1.f, 1.f, 1.f);
	mat.buildProjectionMatrixPerspectiveRH(100.f, 100.f, 1.f, 1000.f);
	mat.buildProjectionMatrixPerspectiveLH(10000.f, 10000.f, 1.f, 1000.f);
	mat.buildProjectionMatrixOrthoLH(10000.f, 10000.f, 1.f, 1000.f);
	mat.buildProjectionMatrixOrthoRH(10000.f, 10000.f, 1.f, 1000.f);
	mat.buildCameraLookAtMatrixLH(vector3df(1.f, 1.f, 1.f), vector3df(0.f, 0.f, 0.f), vector3df(0.f, 1.f, 0.f) );
	mat.buildCameraLookAtMatrixRH(vector3df(1.f, 1.f, 1.f), vector3df(0.f, 0.f, 0.f), vector3df(0.f, 1.f, 0.f) );
	mat.buildShadowMatrix(vector3df(1.f, 1.f, 1.f), p1);
	core::rect<s32> a1(0,0,100,100);
	mat.buildNDCToDCMatrix(a1, 1.f);
	mat.interpolate(mat2, 1.f);
	mat = mat.getTransposed();
	mat.getTransposed(mat2);
	mat.buildRotateFromTo(vector3df(1.f, 1.f, 1.f), vector3df(1.f, 1.f, 1.f));
	mat.setRotationCenter(vector3df(1.f, 1.f, 1.f), vector3df(1.f, 1.f, 1.f));
	mat.buildAxisAlignedBillboard(vector3df(1.f, 1.f, 1.f), vector3df(1.f, 1.f, 1.f), vector3df(1.f, 1.f, 1.f), vector3df(1.f, 1.f, 1.f), vector3df(1.f, 1.f, 1.f));
	mat.buildTextureTransform( 1.f,vector2df(1.f, 1.f), vector2df(1.f, 1.f), vector2df(1.f, 1.f));
	mat.setTextureRotationCenter( 1.f );
	mat.setTextureTranslate( 1.f, 1.f );
	mat.setTextureTranslateTransposed(1.f, 1.f);
	mat.setTextureScale( 1.f, 1.f );
	mat.setTextureScaleCenter( 1.f, 1.f );
	f32 fv16[16];
	mat.setM(fv16);
	mat.setDefinitelyIdentityMatrix(false);
	b = mat.getDefinitelyIdentityMatrix();
	b = mat.equals(mat2);
	f1 = f1+f2+f3+f4+*pf1+*pf2; // getting rid of unused variable warnings.
}

}

bool matrixOps(void)
{
	bool result = true;
	calltest();
	result &= identity();
	result &= rotations();
	result &= isOrthogonal();
	result &= transformations();
	result &= setRotationAxis();
	return result;
}

