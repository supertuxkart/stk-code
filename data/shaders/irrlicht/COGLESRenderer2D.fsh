// Copyright (C) 2009-2010 Amundis
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// and OpenGL ES driver implemented by Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

precision mediump float;

uniform bool uUseTexture;
uniform sampler2D uTextureUnit;

varying vec4 vVertexColor;
varying vec2 vTexCoord;

void main(void)
{
	vec4 Color = vVertexColor;

	if(uUseTexture)
		Color *= texture2D(uTextureUnit, vTexCoord);
	
	gl_FragColor = Color;
}
