precision mediump float;

/* Definitions */

#define Solid 0
#define Solid2Layer 1
#define LightMap 2
#define DetailMap 3
#define SphereMap 4
#define Reflection2Layer 5
#define TransparentAlphaChannel 6
#define TransparentAlphaChannelRef 7
#define TransparentVertexAlpha 8
#define TransparentReflection2Layer 9

/* Uniforms */

uniform int uMaterialType;

uniform bool uTextureUsage0;
//uniform bool uTextureUsage1;

uniform sampler2D uTextureUnit0;
//uniform sampler2D uTextureUnit1;

/* Varyings */

varying vec2 varTexCoord0;
//varying vec2 varTexCoord1;
varying vec4 varVertexColor;
varying float varEyeDist;

vec4 renderSolid()
{
	vec4 Color = varVertexColor;

	if(uTextureUsage0)
		Color *= texture2D(uTextureUnit0, varTexCoord0);
		
	//Color.a = 1.0;
		
	return Color;
}

vec4 render2LayerSolid()
{
	float BlendFactor = varVertexColor.a;
	
	vec4 Texel0 = texture2D(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture2D(uTextureUnit1, varTexCoord1);
	
	vec4 Color = Texel0 * BlendFactor;
	//vec4 Color += Texel1 * (1.0 - BlendFactor);
	
	return Color;
}

vec4 renderLightMap()
{
	vec4 Texel0 = texture2D(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture2D(uTextureUnit1, varTexCoord1);
	
	vec4 Color = Texel0 * 4.0;
	//Color *= Texel1;
	Color.a = Texel0.a * Texel0.a;
	
	return Color;
}

vec4 renderDetailMap()
{
	vec4 Texel0 = texture2D(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture2D(uTextureUnit1, varTexCoord1);
	
	vec4 Color = Texel0;
	//Color += Texel1 - 0.5;
	
	return Color;
}

vec4 renderReflection2Layer()
{
	vec4 Color = varVertexColor;
	
	vec4 Texel0 = texture2D(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture2D(uTextureUnit1, varTexCoord1);
	
	Color *= Texel0;
	//Color *= Texel1;
	
	return Color;
}

vec4 renderTransparent()
{
	vec4 Color = vec4(1.0, 1.0, 1.0, 1.0);

	if(uTextureUsage0)
		Color *= texture2D(uTextureUnit0, varTexCoord0);

	return Color;
}

void main ()
{
    if (uMaterialType == Solid)
		gl_FragColor = renderSolid();
	else if(uMaterialType == Solid2Layer)
		gl_FragColor = render2LayerSolid();
	else if(uMaterialType == LightMap)
		gl_FragColor = renderSolid();
	else if(uMaterialType == DetailMap)
		gl_FragColor = renderDetailMap();
	else if(uMaterialType == SphereMap)
		gl_FragColor = renderSolid();
	else if(uMaterialType == Reflection2Layer)
		gl_FragColor = renderReflection2Layer();
	else if(uMaterialType == TransparentAlphaChannel)
		gl_FragColor = renderTransparent();
	else if(uMaterialType == TransparentAlphaChannelRef)
	{
		vec4 Color = renderTransparent();
		
		if (Color.a < 0.5)
			discard;
		
		gl_FragColor = Color;
	}
	else if(uMaterialType == TransparentVertexAlpha)
	{
		vec4 Color = renderTransparent();
		Color.a = varVertexColor.a;
		
		gl_FragColor = Color;
	}
	else if(uMaterialType == TransparentReflection2Layer)
	{
		vec4 Color = renderReflection2Layer();
		Color.a = varVertexColor.a;
		
		gl_FragColor = Color;
	}
	else
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}