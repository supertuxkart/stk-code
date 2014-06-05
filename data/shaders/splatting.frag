uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
uniform vec3 ambient;

layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

#if __VERSION__ >= 130
in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;
#else
varying vec2 uv;
varying vec2 uv_bis;
#define FragColor gl_FragColor
#endif


void main() {
	// Splatting part
	vec4 splatting = texture(tex_layout, uv_bis);
	vec4 detail0 = texture(tex_detail0, uv);
	vec4 detail1 = texture(tex_detail1, uv);
	vec4 detail2 = texture(tex_detail2, uv);
	vec4 detail3 = texture(tex_detail3, uv);
	vec4 detail4 = vec4(0.0);

	vec4 splatted = splatting.r * detail0 +
			splatting.g * detail1 +
			splatting.b * detail2 +
			(1.0 - splatting.r - splatting.g - splatting.b) * detail3;

   vec2 tc = gl_FragCoord.xy / screen;
   vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
   vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
  float ao = texture(SSAO, tc).x;
   vec3 LightFactor = ao * ambient + DiffuseComponent + SpecularComponent;
	
	FragColor = vec4(splatted.xyz * LightFactor, 1.);
}
