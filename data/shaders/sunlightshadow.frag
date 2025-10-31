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

uniform vec3  sundirection;
uniform float shadow_res;
uniform vec3  sun_color;
uniform float overlap_proportion;

uniform float texel0;
uniform float texel1;
uniform float texel2;
uniform float texel3;

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
float getShadowFactor(vec3 pos, int index)
{
    vec4 shadowcoord = (u_shadow_projection_view_matrices[index] * u_inverse_view_matrix * vec4(pos, 1.0));
    shadowcoord.xy /= shadowcoord.w;
    vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
    float d = .5 * shadowcoord.z + .5;

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

vec3 getXcYcZc(int x, int y, float zC)
{
    // We use perspective symetric projection matrix hence P(0,2) = P(1, 2) = 0
    float xC= (2. * (float(x)) / u_screen.x - 1.) * zC / u_projection_matrix[0][0];
    float yC= (2. * (float(y)) / u_screen.y - 1.) * zC / u_projection_matrix[1][1];
    return vec3(xC, yC, zC);
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_screen;
    float z = texture(dtex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix);

    // get the normal of current fragment
    vec3 ddx = dFdx(xpos.xyz);
    vec3 ddy = dFdy(xpos.xyz);
    vec3 geo_norm = normalize(cross(ddy, ddx));

    vec3 norm = (u_view_matrix * vec4(DecodeNormal(texture(ntex, uv).xy), 0)).xyz;
    float roughness = texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

    vec3 Lightdir = SunMRP(norm, eyedir);
    float NdotL = clamp(dot(norm, Lightdir), 0., 1.);

    vec3 Specular = SpecularBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);
    vec3 Diffuse = DiffuseBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);

    // Shadows
    float factor;
    vec3 lbias = 40. * Lightdir / shadow_res; // Scale with blur kernel size
    vec3 nbias = geo_norm * (1.0 - max(dot(-geo_norm, Lightdir), 0.)) / shadow_res;
    nbias -= Lightdir * dot(nbias, Lightdir); // Slope-scaled normal bias

    if (xpos.z < split0) {
        factor = getShadowFactor(xpos.xyz + lbias + nbias * texel0, 0);
        if (xpos.z > blend_start(split0)) {
            factor = mix(factor, getShadowFactor(xpos.xyz + lbias + nbias * texel1, 1), 
                            (xpos.z - blend_start(split0)) / split0 / overlap_proportion);
        }
    } else if (xpos.z < split1) {
        factor = getShadowFactor(xpos.xyz + lbias + nbias * texel1, 1);
        if (xpos.z > blend_start(split1)) {
            factor = mix(factor, getShadowFactor(xpos.xyz + lbias + nbias * texel2, 2), 
                            (xpos.z - blend_start(split1)) / split1 / overlap_proportion);
        }
    } else if (xpos.z < split2) {
        factor = getShadowFactor(xpos.xyz + lbias + nbias * texel2, 2);
        if (xpos.z > blend_start(split2)) {
            factor = mix(factor, getShadowFactor(xpos.xyz + lbias + nbias * texel3, 3), 
                            (xpos.z - blend_start(split2)) / split2 / overlap_proportion);
        }
    } else if (xpos.z < splitmax) {
        factor = getShadowFactor(xpos.xyz + lbias + nbias * texel3, 3);
        if (xpos.z > blend_start(splitmax)) {
            factor = mix(factor, 1.0, (xpos.z - blend_start(splitmax)) / splitmax / overlap_proportion);
        }
    } else {
        factor = 1.;
    }

    Diff = vec4(factor * NdotL * Diffuse * sun_color, 1.);
    Spec = vec4(factor * NdotL * Specular * sun_color, 1.);
}
