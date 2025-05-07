uniform sampler2D dtex;
uniform float density;
uniform vec3 fogcol;

flat in vec3 center;
flat in float energy;
flat in vec3 col;
flat in float radius;
flat in vec4 direction_scale_offset;

out vec4 Fog;

#stk_include "utils/getPosFromUVDepth.frag"

void main()
{
    vec4 pseudocenter = u_view_matrix * vec4(center.xyz, 1.0);
    pseudocenter /= pseudocenter.w;
    vec3 light_pos = pseudocenter.xyz;
    vec3 light_col = col.xyz;

    // Compute pixel position
    vec2 texc = 2. * gl_FragCoord.xy / u_screen;
    float z = texture(dtex, texc).x;
    vec4 pixelpos = getPosFromUVDepth(vec3(texc, z), u_inverse_projection_matrix);
    vec3 eyedir = -normalize(pixelpos.xyz);

    vec3 farthestpoint = - eyedir * (min(dot(-eyedir, light_pos) + radius, length(pixelpos.xyz)));
    vec3 closestpoint = - eyedir * (dot(-eyedir, light_pos) - radius);
    if (closestpoint.z < 1.) closestpoint = vec3(0.);

    float stepsize = length(farthestpoint - closestpoint) / 16.;
    vec3 fog = vec3(0.);
    vec3 xpos = farthestpoint;
    vec3 fog_factor = light_col * density * stepsize * energy * 20.;
    vec3 xpos_step = eyedir * stepsize;

    // Spotlight direction calculation
    float sscale = direction_scale_offset.z;
    vec3 sdir;
    if (sscale != 0.)
    {
        sdir = vec3(direction_scale_offset.xy, 0.);
        sdir.z = sqrt(1. - dot(sdir, sdir)) * sign(sscale);
        sdir = (u_view_matrix * vec4(sdir, 0.0)).xyz;
    }

    for (int i = 0; i < 16; i++)
    {
        vec3 light_to_pos = light_pos - xpos;
        float d = length(light_to_pos);
        float l = (16. - float(i)) * stepsize;
        vec3 base_att = fog_factor / (1. + d * d) * max((radius - d) / radius, 0.) * exp(- density * d) * exp(- density * l);

        // Apply spotlight attenuation
        if (sscale != 0.)
        {
            float offset = direction_scale_offset.w;
            float sattenuation = clamp(dot(-sdir, normalize(light_to_pos)) *
                                 abs(sscale) + offset, 0.0, 1.0);
            base_att *= sattenuation * sattenuation;
        }

        fog += base_att;
        xpos += xpos_step;
    }

    Fog = vec4(fogcol * fog, 0.);
}
