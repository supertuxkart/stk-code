// Copyright (C) 2009-2010 Amundis
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// and OpenGL ES driver implemented by Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#version 300 es

#define MAX_LIGHTS 2

precision mediump float;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform float uHeightScale;

in vec4 varTexCoord;
in vec3 varLightVector[MAX_LIGHTS];
in vec4 varLightColor[MAX_LIGHTS];
in vec3 varEyeVector;

in vec4 debug;

void main(void)
{
	// fetch color and normal map
	vec4 normalMap = texture(texture1, varTexCoord.xy) *  2.0 - 1.0;

	// height = height * scale
	normalMap *= uHeightScale;
	
	// calculate new texture coord: height * eye + oldTexCoord
	vec2 offset = varEyeVector.xy * normalMap.w + varTexCoord.xy;

	// fetch new textures
	vec4 colorMap  = texture(texture0, offset);
	normalMap = normalize(texture(texture1, offset) * 2.0 - 1.0); 
	
	// calculate color of light 0
	vec4 color = clamp(varLightColor[0], 0.0, 1.0) * dot(normalMap.xyz, normalize(varLightVector[0].xyz));
	
	// calculate color of light 1
	color += clamp(varLightColor[1], 0.0, 1.0) * dot(normalMap.xyz, normalize(varLightVector[1].xyz));

	//luminance * base color
	color *= colorMap;
	color.a = varLightColor[0].a;
	
	out vec4 color;
}
