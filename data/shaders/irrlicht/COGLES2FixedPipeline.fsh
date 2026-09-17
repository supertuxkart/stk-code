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
#define StkGrass 10
#define StkBlend 11

/* Uniforms */

uniform int uMaterialType;

uniform float uHueChange;
uniform vec4 uVertexColor;

uniform bool uTextureUsage0;
//uniform bool uTextureUsage1;

uniform sampler2D uTextureUnit0;
//uniform sampler2D uTextureUnit1;

/* Ins */

in vec2 varTexCoord0;
in vec4 varVertexColor;
in float varEyeDist;

vec3 rgbToHsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsvToRgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 renderSolid()
{
	vec4 Color = vec4(1.0, 1.0, 1.0, 1.0);
	if(uTextureUsage0)
	{
		Color *= texture(uTextureUnit0, varTexCoord0);
		if (uHueChange > 0.0)
		{
			float f_hue_change = 0.66;
			float mask = Color.a;
			vec3 old_hsv = rgbToHsv(Color.rgb);
			float mask_step = step(mask, 0.5);
			float saturation = mask * 1.825;
			vec2 new_xy = mix(vec2(old_hsv.x, old_hsv.y), vec2(uHueChange,
				max(old_hsv.y, saturation)), vec2(mask_step, mask_step));
			Color.rgb = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
		}
		vec3 mixed_color = varVertexColor.rgb * uVertexColor.rgb;
		Color.rgb *= mixed_color;
		Color.a = 1.0;
	}
	else
	{
		Color = varVertexColor * uVertexColor;
		Color.a = 1.0;
	}
	return Color;
}

vec4 render2LayerSolid()
{
	float BlendFactor = varVertexColor.a;
	
	vec4 Texel0 = texture(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture(uTextureUnit1, varTexCoord1);
	
	vec4 Color = Texel0 * BlendFactor;
	//vec4 Color += Texel1 * (1.0 - BlendFactor);
	
	return Color;
}

vec4 renderLightMap()
{
	vec4 Texel0 = texture(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture(uTextureUnit1, varTexCoord1);
	
	vec4 Color = Texel0 * 4.0;
	//Color *= Texel1;
	Color.a = Texel0.a * Texel0.a;
	
	return Color;
}

vec4 renderDetailMap()
{
	vec4 Texel0 = texture(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture(uTextureUnit1, varTexCoord1);
	
	vec4 Color = Texel0;
	//Color += Texel1 - 0.5;
	
	return Color;
}

vec4 renderReflection2Layer()
{
	vec4 Color = varVertexColor;
	
	vec4 Texel0 = texture(uTextureUnit0, varTexCoord0);
	//vec4 Texel1 = texture(uTextureUnit1, varTexCoord1);
	
	Color *= Texel0;
	//Color *= Texel1;
	
	return Color;
}

vec4 renderTransparent()
{
	vec4 Color = vec4(1.0, 1.0, 1.0, 1.0);

	if(uTextureUsage0)
	{
		Color *= texture(uTextureUnit0, varTexCoord0);
		if (uHueChange > 0.0)
		{
			vec3 old_hsv = rgbToHsv(Color.rgb);
			vec2 new_xy = vec2(uHueChange, old_hsv.y);
			vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
			Color.rgb = vec3(new_color.r, new_color.g, new_color.b);
		}
	}

	return Color;
}

vec4 renderTransparentVertexColor()
{
	vec4 Color = vec4(1.0, 1.0, 1.0, 1.0);
	if(uTextureUsage0)
	{
		Color *= texture(uTextureUnit0, varTexCoord0);
		if (uHueChange > 0.0)
		{
			vec3 old_hsv = rgbToHsv(Color.rgb);
			vec2 new_xy = vec2(uHueChange, old_hsv.y);
			vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
			Color.rgb = vec3(new_color.r, new_color.g, new_color.b);
		}
		vec4 mixed_color = varVertexColor * uVertexColor;
		Color *= mixed_color;
	}

	return Color;
}

void main ()
{
    if (uMaterialType == Solid)
		out vec4 renderSolid();
	else if(uMaterialType == Solid2Layer)
		out vec4 render2LayerSolid();
	else if(uMaterialType == LightMap)
		out vec4 renderSolid();
	else if(uMaterialType == DetailMap)
		out vec4 renderDetailMap();
	else if(uMaterialType == SphereMap)
		out vec4 renderSolid();
	else if(uMaterialType == Reflection2Layer)
		out vec4 renderReflection2Layer();
	else if(uMaterialType == TransparentAlphaChannel)
		out vec4 renderTransparent();
	else if(uMaterialType == TransparentAlphaChannelRef)
	{
		vec4 Color = renderTransparentVertexColor();
		if (Color.a < 0.5)
			discard;
		out vec4 Color;
	}
	else if(uMaterialType == StkGrass)
	{
		vec4 Color = renderTransparent();
		if (Color.a < 0.5)
			discard;
		out vec4 Color;
	}
	else if(uMaterialType == StkBlend)
	{
		out vec4 renderTransparentVertexColor();
	}
	else if(uMaterialType == TransparentVertexAlpha)
	{
		vec4 Color = renderTransparent();
		Color.a = varVertexColor.a;
		
		out vec4 Color * uVertexColor;
	}
	else if(uMaterialType == TransparentReflection2Layer)
	{
		vec4 Color = renderReflection2Layer();
		Color.a = varVertexColor.a;
		
		out vec4 Color;
	}
	else
		out vec4 vec4(1.0, 1.0, 1.0, 1.0);
}
