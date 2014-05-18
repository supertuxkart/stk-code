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


void main() {
    const vec3 forward = vec3(0., 0., 1.);
    vec3 normal_x = normalize(vec3(nor.x, 0., nor.z));
    float sin_theta_x = length(cross(forward, normal_x)) * sign(nor.x);
    vec3 normal_y = normalize(vec3(0., nor.y, nor.z));
    float sin_theta_y = length(cross(forward, normal_y)) * sign(nor.y);
    vec4 detail0 = texture(tex, .5 * vec2(sin_theta_x, sin_theta_y) + .5);

    FragColor = vec4(detail0.xyz, 1.);
}
