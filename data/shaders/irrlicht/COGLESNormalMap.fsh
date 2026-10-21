// Copyright (C) 2009-2010 Amundis
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// and OpenGL ES driver implemented by Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h
#define MAX_LIGHTS 2

precision mediump float;

uniform sampler2D texture0;
uniform sampler2D texture1;

varying vec4 varTexCoord;
varying vec3 varLightVector[MAX_LIGHTS];
varying vec4 varLightColor[MAX_LIGHTS];

varying vec4 debug;

void main(void)
{
	// fetch color and normal map
	vec4 normalMap = texture2D(texture1, varTexCoord.xy) *  2.0 - 1.0;
	vec4 colorMap  = texture2D(texture0, varTexCoord.xy);
	
	// calculate color of light 0
	vec4 color = clamp(varLightColor[0], 0.0, 1.0) * dot(normalMap.xyz, normalize(varLightVector[0].xyz));
	
	// calculate color of light 1
	color += clamp(varLightColor[1], 0.0, 1.0) * dot(normalMap.xyz, normalize(varLightVector[1].xyz));

	//luminance * base color
	color *= colorMap;
	color.a = varLightColor[0].a;
	
	gl_FragColor = color;
}