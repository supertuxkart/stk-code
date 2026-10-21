// Copyright (C) 2009-2010 Amundis
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// and OpenGL ES driver implemented by Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h
#define MAX_LIGHTS 2

attribute vec4 inVertexPosition;
attribute vec4 inVertexColor;
attribute vec4 inTexCoord0;
attribute vec3 inVertexNormal;
attribute vec3 inVertexTangent;
attribute vec3 inVertexBinormal;

uniform mat4 uMvpMatrix;
uniform vec4 uLightPos[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS];

varying vec4 varTexCoord;
varying vec3 varLightVector[MAX_LIGHTS];
varying vec4 varLightColor[MAX_LIGHTS];

varying vec4 debug;

void main(void)
{
	debug = vec4(inVertexNormal, 1.0);
	// transform position to clip space
	gl_Position = uMvpMatrix * inVertexPosition;
	
	// vertex - lightpositions
	vec4 tempLightVector0 = uLightPos[0] - inVertexPosition;
	vec4 tempLightVector1 = uLightPos[1] - inVertexPosition;
	
	// transform the light vector 1 with U, V, W
	varLightVector[0].x = dot(inVertexTangent,  tempLightVector0.xyz);
	varLightVector[0].y = dot(inVertexBinormal, tempLightVector0.xyz);
	varLightVector[0].z = dot(inVertexNormal,   tempLightVector0.xyz);

	
	// transform the light vector 2 with U, V, W
	varLightVector[1].x = dot(inVertexTangent,  tempLightVector1.xyz);
	varLightVector[1].y = dot(inVertexBinormal, tempLightVector1.xyz);
	varLightVector[1].z = dot(inVertexNormal,   tempLightVector1.xyz);

	// calculate attenuation of light 0
	varLightColor[0].w = 0.0;
	varLightColor[0].x = dot(tempLightVector0, tempLightVector0);
	varLightColor[0].x *= uLightColor[0].w;
	varLightColor[0] = vec4(inversesqrt(varLightColor[0].x));
	varLightColor[0] *= uLightColor[0];
	
	// normalize light vector 0
	varLightVector[0] = normalize(varLightVector[0]);
	
	// calculate attenuation of light 1
	varLightColor[1].w = 0.0;
	varLightColor[1].x = dot(tempLightVector1, tempLightVector1);
	varLightColor[1].x *= uLightColor[1].w;
	varLightColor[1] = vec4(inversesqrt(varLightColor[1].x));
	varLightColor[1] *= uLightColor[1];
	
	// normalize light vector 1
	varLightVector[1] = normalize(varLightVector[1]);
	
	// move out texture coordinates and original alpha value
	varTexCoord = inTexCoord0;
	varLightColor[0].a = inVertexColor.a;
}