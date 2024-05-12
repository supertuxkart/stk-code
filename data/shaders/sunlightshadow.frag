uniform sampler2D ntex;
#if defined(GL_ES) && defined(GL_FRAGMENT_PRECISION_HIGH)
uniform highp sampler2D dtex;
#else
uniform sampler2D dtex;
#endif
uniform sampler2DArrayShadow shadowtex;

uniform float split0;
uniform float split1;
uniform float split2;
uniform float splitmax;
uniform float shadow_res;
uniform float overlap_proportion;

uniform vec3 sundirection;
uniform vec3 sun_color;

in vec2 uv;
#ifdef GL_ES
layout (location = 0) out vec4 Diff;
layout (location = 1) out vec4 Spec;
#else
out vec4 Diff;
out vec4 Spec;
#endif

#stk_include "utils/decodeNormal.frag"
#stk_include "utils/SpecularBRDF.frag"
#stk_include "utils/DiffuseBRDF.frag"
#stk_include "utils/getPosFromUVDepth.frag"
#stk_include "utils/SunMRP.frag"

// https://web.archive.org/web/20230210095515/http://the-witness.net/news/2013/09/shadow-mapping-summary-part-1
float getShadowFactor(vec3 pos, int index, float bias)
{
    vec4 shadowcoord = (u_shadow_projection_view_matrices[index] * u_inverse_view_matrix * vec4(pos, 1.0));
    shadowcoord.xy /= shadowcoord.w;
    vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
    float d = .5 * shadowcoord.z + .5 - bias;

    vec2 uv = shadowtexcoord * shadow_res;
    vec2 base_uv = floor(uv + 0.5);
    float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);
    base_uv -= 0.5;
    base_uv /= shadow_res;

    float uw0 = (4.0 - 3.0 * s);
    float uw1 = 7.0;
    float uw2 = (1.0 + 3.0 * s);

    float u0 = (3.0 - 2.0 * s) / uw0 - 2.0;
    float u1 = (3.0 + s) / uw1;
    float u2 = s / uw2 + 2.0;

    float vw0 = (4.0 - 3.0 * t);
    float vw1 = 7.0;
    float vw2 = (1.0 + 3.0 * t);

    float v0 = (3.0 - 2.0 * t) / vw0 - 2.0;
    float v1 = (3.0 + t) / vw1;
    float v2 = t / vw2 + 2.0;

    float sum = 0.0;

    sum += uw0 * vw0 * texture(shadowtex, vec4(base_uv + (vec2(u0, v0) / shadow_res), float(index), d));
    sum += uw1 * vw0 * texture(shadowtex, vec4(base_uv + (vec2(u1, v0) / shadow_res), float(index), d));
    sum += uw2 * vw0 * texture(shadowtex, vec4(base_uv + (vec2(u2, v0) / shadow_res), float(index), d));

    sum += uw0 * vw1 * texture(shadowtex, vec4(base_uv + (vec2(u0, v1) / shadow_res), float(index), d));
    sum += uw1 * vw1 * texture(shadowtex, vec4(base_uv + (vec2(u1, v1) / shadow_res), float(index), d));
    sum += uw2 * vw1 * texture(shadowtex, vec4(base_uv + (vec2(u2, v1) / shadow_res), float(index), d));

    sum += uw0 * vw2 * texture(shadowtex, vec4(base_uv + (vec2(u0, v2) / shadow_res), float(index), d));
    sum += uw1 * vw2 * texture(shadowtex, vec4(base_uv + (vec2(u1, v2) / shadow_res), float(index), d));
    sum += uw2 * vw2 * texture(shadowtex, vec4(base_uv + (vec2(u2, v2) / shadow_res), float(index), d));

    return sum / 144.0;
}

float blend_start(float x) {
    return x * (1.0 - overlap_proportion);
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_screen;
    float z = texture(dtex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix);

    vec3 norm = DecodeNormal(texture(ntex, uv).xy);
    float roughness =texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

    vec3 Lightdir = SunMRP(norm, eyedir);
    float NdotL = clamp(dot(norm, Lightdir), 0., 1.);

    vec3 Specular = SpecularBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);
    vec3 Diffuse = DiffuseBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);

    // Shadows
    float factor;
    float bias = max(1.0 - NdotL, .2) / shadow_res;
    if (xpos.z < split0) {
        factor = getShadowFactor(xpos.xyz, 0, bias);
        if (xpos.z > blend_start(split0)) {
            factor = mix(factor, getShadowFactor(xpos.xyz, 1, bias), (xpos.z - blend_start(split0)) / split0 / overlap_proportion);
        }
    } else if (xpos.z < split1) {
        factor = getShadowFactor(xpos.xyz, 1, bias);
        if (xpos.z > blend_start(split1)) {
            factor = mix(factor, getShadowFactor(xpos.xyz, 2, bias), (xpos.z - blend_start(split1)) / split1 / overlap_proportion);
        }
    } else if (xpos.z < split2) {
        factor = getShadowFactor(xpos.xyz, 2, bias);
        if (xpos.z > blend_start(split2)) {
            factor = mix(factor, getShadowFactor(xpos.xyz, 3, bias), (xpos.z - blend_start(split2)) / split2 / overlap_proportion);
        }
    } else if (xpos.z < splitmax) {
        factor = getShadowFactor(xpos.xyz, 3, bias);
        if (xpos.z > blend_start(splitmax)) {
            factor = mix(factor, 1.0, (xpos.z - blend_start(splitmax)) / splitmax / overlap_proportion);
        }
    } else {
        factor = 1.;
    }

    Diff = vec4(factor * NdotL * Diffuse * sun_color, 1.);
    Spec = vec4(factor * NdotL * Specular * sun_color, 1.);
}
