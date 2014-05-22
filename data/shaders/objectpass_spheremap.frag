// See http://www.ozone3d.net/tutorials/glsl_texturing_p04.php for ref

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

uniform sampler2D tex;

#if __VERSION__ >= 130
in vec3 nor;
out vec4 FragColor;
#else
varying vec3 nor;
#define FragColor gl_FragColor
#endif

vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);
vec3 getLightFactor(float specMapValue);

void main() {
    vec3 texc = gl_FragCoord.xyz / vec3(screen, 1.);
    vec3 u = getPosFromUVDepth(texc, InverseProjectionMatrix).xyz;
    vec3 r = reflect(u, nor);

    float m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z + 1.0) * (r.z + 1.0));
    r.y = - r.y;
    vec4 detail0 = texture(tex, r.xy / m + .5);
    vec3 LightFactor = getLightFactor(1.);

    FragColor = vec4(detail0.xyz * LightFactor, 1.);
}
