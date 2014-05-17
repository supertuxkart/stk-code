uniform sampler2D Albedo;
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
in vec3 nor;
in vec2 uv;
out vec4 FragColor;
#else
varying vec3 nor;
varying vec2 uv;
#define FragColor gl_FragColor
#endif

void main() {
	float rim = 1.0 - dot(nor, vec3(0., 0., -1));
	rim = smoothstep(0.5, 1.5, rim) * 0.15;

	vec4 color = texture(Albedo, uv);
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    float ao = texture(SSAO, tc).x;
    vec3 LightFactor = ao * ambient + DiffuseComponent + SpecularComponent * (1. - color.a);
    FragColor = vec4(color.xyz * LightFactor + rim, 1.);
}

