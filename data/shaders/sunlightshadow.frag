uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2DArrayShadow shadowtex;

uniform float split0;
uniform float split1;
uniform float split2;
uniform float splitmax;
uniform float shadow_res;

in vec2 uv;
out vec4 Diff;
out vec4 Spec;

vec3 DecodeNormal(vec2 n);
vec3 SpecularBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);
vec3 DiffuseBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);
vec3 SunMRP(vec3 normal, vec3 eyedir);

float getShadowFactor(vec3 pos, int index)
{
    vec4 shadowcoord = (ShadowViewProjMatrixes[index] * InverseViewMatrix * vec4(pos, 1.0));
    shadowcoord.xy /= shadowcoord.w;
    vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
    float d = .5 * shadowcoord.z + .5;

    float result = 0.;

    for (float i = -1.; i <= 1.; i += 1.)
    {
        for (float j = -1.; j <= 1.; j += 1.)
            result += texture(shadowtex, vec4(shadowtexcoord + vec2(i,j) / shadow_res, float(index), d));
    }

    return result / 9.;
}

void main() {
    vec2 uv = gl_FragCoord.xy / screen;
    float z = texture(dtex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

    vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
    float roughness =texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

    vec3 Lightdir = SunMRP(norm, eyedir);
    float NdotL = clamp(dot(norm, Lightdir), 0., 1.);

    vec3 Specular = SpecularBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);
    vec3 Diffuse = DiffuseBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);

    // Shadows
    float factor;
    if (xpos.z < split0)
        factor = getShadowFactor(xpos.xyz, 0);
    else if (xpos.z < split1)
        factor = getShadowFactor(xpos.xyz, 1);
    else if (xpos.z < split2)
        factor = getShadowFactor(xpos.xyz, 2);
    else if (xpos.z < splitmax)
        factor = getShadowFactor(xpos.xyz, 3);
    else
        factor = 1.;

    Diff = vec4(factor * NdotL * Diffuse * sun_col, 1.);
    Spec = vec4(factor * NdotL * Specular * sun_col, 1.);
}
