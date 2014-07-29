uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;

#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
uniform vec2 screen;
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};
#endif

vec3 getLightFactor(float specMapValue)
{
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    float ao = texture(SSAO, tc).x;
    vec3 tmp = DiffuseComponent + SpecularComponent * specMapValue;
    return tmp * ao;
}