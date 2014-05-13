#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};
#endif

uniform samplerCube tex;
uniform vec2 screen;

#if __VERSION__ >= 130
in vec3 nor;
out vec4 FragColor;
#else
varying vec3 nor;
#define FragColor gl_FragColor
#endif


void main() {
    vec3 fpos = gl_FragCoord.xyz / vec3(screen, 1.);
    vec4 xpos = 2.0 * vec4(fpos, 1.0) - 1.0;
    xpos = InverseProjectionMatrix * xpos;

    xpos.xyz /= xpos.w;
    vec3 viewSampleDir = reflect(xpos.xyz, nor);
    // Convert sampleDir in world space (where tex was generated)
    vec4 sampleDir = transpose(InverseViewMatrix) * vec4(viewSampleDir, 0.);
    vec4 detail0 = texture(tex, sampleDir.xyz);

    FragColor = vec4(detail0.xyz, 1.);
}
