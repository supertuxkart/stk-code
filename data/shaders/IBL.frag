uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2DShadow stex;
uniform sampler2D albedo;
uniform sampler2D ssao;
uniform sampler2D ctex;

uniform int ssr;

#ifdef GL_ES
layout (location = 0) out vec4 Diff;
layout (location = 1) out vec4 Spec;
#else
out vec4 Diff;
out vec4 Spec;
#endif

#stk_include "utils/decodeNormal.frag"
#stk_include "utils/encode_normal.frag"
#stk_include "utils/getPosFromUVDepth.frag"
#stk_include "utils/DiffuseIBL.frag"
#stk_include "utils/SpecularIBL.frag"
#stk_include "utils/screen_space_reflection.frag"

vec3 gtaoMultiBounce(float visibility, vec3 albedo)
{
    // Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"
    vec3 a =  2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c =  2.7552 * albedo + 0.6903;

    return max(vec3(visibility), ((visibility * a + b) * visibility + c) * visibility);
}


// Main ===================================================================

void main(void)
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec3 normal = (u_view_matrix * vec4(DecodeNormal(texture(ntex, uv).xy), 0)).xyz;

    float z = texture(dtex, uv).x;

    vec4 xpos = getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos.xyz);
    // Extract roughness
    float specval = texture(ntex, uv).z;

    float ao = texture(ssao, uv).x;
    // Lagarde and de Rousiers 2014, "Moving Frostbite to PBR"
    float ao_spec = clamp(pow(max(dot(normal, eyedir), 0.) + ao, exp2(-16.0 * (1.0 - specval) - 1.0)) - 1.0 + ao, 0.0, 1.0);

    if (ssr == 0)
    {
        Diff = vec4(0.25 * DiffuseIBL(normal) * ao, 1.);
        Spec = vec4(.25 * SpecularIBL(normal, eyedir, specval) * ao_spec, 1.);
        return;
    }

    vec3 surface_color = texture(ctex, uv).xyz;
    vec3 ao_multi = gtaoMultiBounce(ao, surface_color);
    vec3 ao_spec_multi = gtaoMultiBounce(ao_spec, surface_color);

    // :::::::: Compute Space Screen Reflection ::::::::::::::::::::::::::::::::::::

    // Output color
    vec3 outColor;

    // Fallback (if the ray can't find an intersection we display the sky)
    vec3 fallback = .25 * SpecularIBL(normal, eyedir, specval);

    // Reflection vector
    vec3 reflected              = reflect(-eyedir, normal);
    // Disable raycasts towards camera
    float cosine                = dot(reflected, eyedir);

    // Only calculate reflections if the reflectivity value is high enough,
    // otherwise just use specular IBL
    if (specval < 0.5 || cosine > 0.2) {
        outColor = fallback;
    } else {
        vec2 viewport_scale = vec2(1.0);
        vec2 viewport_offset = vec2(0.0);
        vec2 coords = RayCast(reflected, xpos.xyz, u_projection_matrix,
            viewport_scale, viewport_offset, stex);

        if (coords.x < 0. || coords.x > 1. || coords.y < 0. || coords.y > 1.) {
            outColor = fallback;
        } else {
            // Disable raycasts onto another reflective surface
            float mirror = texture(ntex, coords).z;
            
            outColor = textureLod(albedo, coords, 0.f).rgb;
            outColor = mix(fallback, outColor, GetEdgeFade(coords,
                viewport_scale, viewport_offset));
            outColor = mix(fallback, outColor, 1. - max(cosine * 5., 0.));
            outColor = mix(fallback, outColor, 4. - max(mirror * 4., 3.));
            // TODO temporary measure the lack of mipmapping for RTT albedo
            // Implement it in proper way
            // Use (specval - 0.5) * 2.0 to bring specval from 0.5-1.0 range to 0.0-1.0 range
            outColor = mix(fallback, outColor, (specval - 0.5) * 2.0);
        }
    }

    Diff = vec4(0.25 * DiffuseIBL(normal) * ao_multi, 1.);
    Spec = vec4(outColor.rgb * ao_spec_multi, 1.0);

}
