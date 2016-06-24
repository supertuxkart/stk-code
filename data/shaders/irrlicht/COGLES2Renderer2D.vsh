// Copyright (C) 2009-2010 Amundis
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// and OpenGL ES driver implemented by Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

attribute vec4 inVertexPosition;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;

uniform mat4 uOrthoMatrix;

varying vec4 vVertexColor;
varying vec2 vTexCoord;

void main(void)
{
	gl_Position = uOrthoMatrix * inVertexPosition;
	vVertexColor = inVertexColor.bgra;
	vTexCoord = inTexCoord0;
}
