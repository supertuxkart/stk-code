uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2DArrayShadow shadowtex;

uniform vec3 direction;
uniform vec3 col;

#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
uniform mat4 ShadowViewProjMatrixes[4];
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


in vec2 uv;
out vec4 Diff;
out vec4 Spec;

vec3 DecodeNormal(vec2 n);
vec3 getSpecular(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);

vec3 getShadowFactor(vec3 pos)
{
    vec3 cascadeColor[] = vec3[](
        vec3(1., 0., 0.),
        vec3(0., 1., 0.),
        vec3(0., 0., 1.),
        vec3(1., 1., 1.)
    );
    for (int i = 0; i < 4; i++)
    {
        vec4 shadowcoord = (ShadowViewProjMatrixes[i] * vec4(pos, 1.0));
        shadowcoord /= shadowcoord.w;
        vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
        if (shadowtexcoord.x < 0. || shadowtexcoord.x > 1. || shadowtexcoord.y < 0. || shadowtexcoord.y > 1.)
            continue;
        return cascadeColor[i] * texture(shadowtex, vec4(shadowtexcoord, float(i), 0.5 * shadowcoord.z + 0.5));
    }
    return vec3(1.);
}

void main() {
    float z = texture(dtex, uv).x;
    vec4 xpos = 2.0 * vec4(uv, z, 1.0) - 1.0;
    xpos = InverseViewMatrix * InverseProjectionMatrix * xpos;
    xpos.xyz /= xpos.w;

    vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
    float roughness =texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

    // Normalized on the cpu
    vec3 L = direction;

    float NdotL = max(0., dot(norm, L));

    vec3 Specular = getSpecular(norm, eyedir, L, col, roughness) * NdotL;
    vec3 outcol = NdotL * col;

    // Shadows
    vec3 factor = getShadowFactor(xpos.xyz);
    Diff = vec4(factor * NdotL * col, 1.);
    Spec = vec4(factor * Specular, 1.);
    return;
}
