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

float quick_hash(vec2 pos) {
	const vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
	return fract(magic.z * fract(dot(pos, magic.xy)));
}

float getShadowFactor(vec3 pos, int index, float split)
{
    // PCF with Vogel Disk Sampling
    // From https://drdesten.github.io/web/tools/vogel_disk/
    const vec2 vogel_disk_16[16] = vec2[](
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

    vec4 shadowcoord = (u_shadow_projection_view_matrices[index] * u_inverse_view_matrix * vec4(pos, 1.0));
    shadowcoord.xy /= shadowcoord.w;
    vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
    //float d = .5 * shadowcoord.z + .5;
    float d = .5 * shadowcoord.z + .5 - .5 * (pos.z + 1) / shadow_res / split * (index + 2);
    float smooth_factor = (pos.z + 1) * 4 / shadow_res / split;

    mat2 disk_rotation;
	{
		float r = quick_hash(gl_FragCoord.xy) * 2.0 * 3.1415926;
		float sr = sin(r);
		float cr = cos(r);
		disk_rotation = mat2(vec2(cr, -sr), vec2(sr, cr));
	}

    float result = texture(shadowtex, vec4(shadowtexcoord, float(index), d));

    for (uint i = 0; i < 16; i++)
    {
        result += texture(shadowtex, vec4(shadowtexcoord + disk_rotation * vogel_disk_16[i] * smooth_factor, float(index), d));
    }
    return result / 16.;
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_screen;
    float z = texture(dtex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix);

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
        factor = getShadowFactor(xpos.xyz, 0, split0);
    else if (xpos.z < split1)
        factor = getShadowFactor(xpos.xyz, 1, split1);
    else if (xpos.z < split2)
        factor = getShadowFactor(xpos.xyz, 2, split2);
    else if (xpos.z < splitmax)
        factor = getShadowFactor(xpos.xyz, 3, splitmax);
    else
        factor = 1.;

    Diff = vec4(factor * NdotL * Diffuse * sun_color, 1.);
    Spec = vec4(factor * NdotL * Specular * sun_color, 1.);
}
