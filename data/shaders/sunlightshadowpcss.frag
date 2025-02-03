uniform sampler2D ntex;
#if defined(GL_ES) && defined(GL_FRAGMENT_PRECISION_HIGH)
uniform highp sampler2D dtex;
#else
uniform sampler2D dtex;
#endif
uniform sampler2DArray shadowtexdepth;
uniform sampler2DArrayShadow shadowtex;

uniform float split0;
uniform float split1;
uniform float split2;
uniform float splitmax;
uniform float shadow_res;
uniform float overlap_proportion;

uniform vec3 box0;
uniform vec3 box1;
uniform vec3 box2;
uniform vec3 box3;

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

// PCF with Vogel Disk Sampling
// From https://drdesten.github.io/web/tools/vogel_disk/
vec2 vogel_disk_16[16] = vec2[](
    vec2(0.18993645671348536, 0.027087114076591513),
    vec2(-0.21261242652069953, 0.23391293246949066),
    vec2(0.04771781344140756, -0.3666840644525993),
    vec2(0.297730981239584, 0.398259878229082),
    vec2(-0.509063425827436, -0.06528681462854097),
    vec2(0.507855152944665, -0.2875976005206389),
    vec2(-0.15230616564632418, 0.6426121151781916),
    vec2(-0.30240170651828074, -0.5805072900736001),
    vec2(0.6978019230005561, 0.2771173334141519),
    vec2(-0.6990963248129052, 0.3210960724922725),
    vec2(0.3565142601623699, -0.7066415061851589),
    vec2(0.266890002328106, 0.8360191043249159),
    vec2(-0.7515861305520581, -0.41609876195815027),
    vec2(0.9102937449894895, -0.17014527555321657),
    vec2(-0.5343471434373126, 0.8058593459499529),
    vec2(-0.1133270115046468, -0.9490025827627441)
);

vec2 vogel_disk_4[4] = vec2[](
	vec2(0.21848650099008202, -0.09211370200809937),
	vec2(-0.5866112654782878, 0.32153793477769893),
	vec2(-0.06595078555407359, -0.879656059066481),
	vec2(0.43407555004227927, 0.6502318262968816)
);

float interleavedGradientNoise(vec2 w)
{
    vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}

vec2 computeReceiverPlaneDepthBias(vec3 position)
{
    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    // Chain rule to compute dz/du and dz/dv
    // |dz/du|   |du/dx du/dy|^-T   |dz/dx|
    // |dz/dv| = |dv/dx dv/dy|    * |dz/dy|
    vec3 duvz_dx = dFdx(position);
    vec3 duvz_dy = dFdy(position);
    vec2 dz_duv = inverse(transpose(mat2(duvz_dx.xy, duvz_dy.xy))) * vec2(duvz_dx.z, duvz_dy.z);
    return dz_duv;
}

mat2 getRandomRotationMatrix(vec2 fragCoord)
{
    // rotate the vogel disk randomly
    float randomAngle = interleavedGradientNoise(fragCoord) * 2.0 * 3.14159;
    vec2 randomBase = vec2(cos(randomAngle), sin(randomAngle));
    mat2 R = mat2(randomBase.x, randomBase.y, -randomBase.y, randomBase.x);
    return R;
}

void blockerSearchAndFilter(out float occludedCount, out float z_occSum,
        vec2 uv, float z_rec, uint layer, vec2 filterRadii, mat2 R, vec2 dz_duv)
{
    occludedCount = 0.0;
    z_occSum = 0.0;
    for (uint i = 0u; i < 4u; i++)
    {
        vec2 duv = R * (vogel_disk_4[i] * filterRadii);
        vec2 tc = clamp(uv + duv, vec2(0.), vec2(1.));

        float z_occ = texture(shadowtexdepth, vec3(tc, float(layer))).r;

        // receiver plane depth bias
        float z_bias = dot(dz_duv, duv);
        float dz = z_rec - z_occ; // dz>0 when blocker is between receiver and light
        float occluded = step(z_bias, dz);
        occludedCount += occluded;
        z_occSum += z_occ * occluded;
    }
    float z_occ = texture(shadowtexdepth, vec3(uv, float(layer))).r;
    float dz = z_rec - z_occ;
    float occluded = step(0., dz);
    occludedCount += occluded;
    z_occSum += z_occ * occluded;
}

float filterPCSS(vec2 uv, float z_rec, uint layer,
        vec2 filterRadii, mat2 R, vec2 dz_duv)
{
    float occludedCount = 0.0; // must be to workaround a spirv-tools issue
    for (uint i = 0u; i < 16u; i++)
    {
        vec2 duv = R * (vogel_disk_16[i] * filterRadii);
        vec2 tc = clamp(uv + duv, vec2(0.), vec2(1.));
        
        // receiver plane depth bias
        float z_bias = dot(dz_duv, duv); 
        occludedCount += texture(shadowtex, vec4(tc, float(layer), z_rec + z_bias));
    }
    return occludedCount * (1.0 / 16.0);
}

float getShadowFactor(vec3 position, vec3 bbox, vec2 dz_duv, uint layer)
{
    float penumbra = tan(3.14 * sun_angle / 360.) * bbox.z * position.z;

    // rotate the poisson disk randomly
    mat2 R = getRandomRotationMatrix(gl_FragCoord.xy);

    float occludedCount = 0.0;
    float z_occSum = 0.0;

    blockerSearchAndFilter(occludedCount, z_occSum, position.xy, position.z, layer, penumbra / bbox.xy, R, dz_duv);

    // early exit if there is no occluders at all, also avoids a divide-by-zero below.
    if (z_occSum == 0.0) {
        return 1.0;
    }

    float penumbraRatio = 1.0 - z_occSum / occludedCount / position.z;

    float percentageOccluded = filterPCSS(position.xy, position.z, layer, penumbra / bbox.xy * penumbraRatio, R, dz_duv);

    return percentageOccluded;
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
    // Calculate all shadow positions to prevent bug of dFdx
    vec4 position = (u_shadow_projection_view_matrices[0] * u_inverse_view_matrix * vec4(xpos.xyz, 1.0));
    vec3 position1 = position.xyz * (1.0 / position.w) * 0.5 + 0.5;

         position = (u_shadow_projection_view_matrices[1] * u_inverse_view_matrix * vec4(xpos.xyz, 1.0));
    vec3 position2 = position.xyz * (1.0 / position.w) * 0.5 + 0.5;

         position = (u_shadow_projection_view_matrices[2] * u_inverse_view_matrix * vec4(xpos.xyz, 1.0));
    vec3 position3 = position.xyz * (1.0 / position.w) * 0.5 + 0.5;

         position = (u_shadow_projection_view_matrices[3] * u_inverse_view_matrix * vec4(xpos.xyz, 1.0));
    vec3 position4 = position.xyz * (1.0 / position.w) * 0.5 + 0.5;

    // We need to use the shadow receiver plane depth bias to combat shadow acne due to the
    // large kernel.
    vec2 dz_duv1 = computeReceiverPlaneDepthBias(position1);
    vec2 dz_duv2 = computeReceiverPlaneDepthBias(position2);
    vec2 dz_duv3 = computeReceiverPlaneDepthBias(position3);
    vec2 dz_duv4 = computeReceiverPlaneDepthBias(position4);

    float factor;
    if (xpos.z < split0) {
        float factor2 = getShadowFactor(position1, box0, dz_duv1, 0);
        factor = factor2;
    }
    if (blend_start(split0) < xpos.z && xpos.z < split1) {
        float factor2 = getShadowFactor(position2, box1, dz_duv2, 1);
        if (xpos.z < split0) {
            factor = mix(factor, factor2, (xpos.z - blend_start(split0)) / split0 / overlap_proportion);
        } else {
            factor = factor2;
        }
    }
    if (blend_start(split1) < xpos.z && xpos.z < split2) {
        float factor2 = getShadowFactor(position3, box2, dz_duv3, 2);
        if (xpos.z < split1) {
            factor = mix(factor, factor2, (xpos.z - blend_start(split1)) / split1 / overlap_proportion);
        } else {
            factor = factor2;
        }
    }
    if (blend_start(split2) < xpos.z && xpos.z < splitmax) {
        float factor2 = getShadowFactor(position4, box3, dz_duv4, 3);
        if (xpos.z < split2) {
            factor = mix(factor, factor2, (xpos.z - blend_start(split2)) / split2 / overlap_proportion);
        } else {
            factor = factor2;
        }
    } 
    if (blend_start(splitmax) < xpos.z) {
        float factor2 = 1.;
        if (xpos.z < splitmax) {
            factor = mix(factor, factor2, (xpos.z - blend_start(splitmax)) / splitmax / overlap_proportion);
        } else {
            factor = factor2;
        }
    }

    Diff = vec4(factor * NdotL * Diffuse * sun_color, 1.);
    Spec = vec4(factor * NdotL * Specular * sun_color, 1.);
}
