uniform sampler2D dtex;
uniform float density;
uniform vec3 fogcol;

flat in vec3 center;
flat in float energy;
flat in vec3 col;
flat in float radius;

out vec4 Fog;

#stk_include "utils/getPosFromUVDepth.frag"

void main()
{
    vec4 pseudocenter = ViewMatrix * vec4(center.xyz, 1.0);
    pseudocenter /= pseudocenter.w;
    vec3 light_pos = pseudocenter.xyz;
    vec3 light_col = col.xyz;

    // Compute pixel position
    vec2 texc = 2. * gl_FragCoord.xy / screen;
    float z = texture(dtex, texc).x;
    vec4 pixelpos = getPosFromUVDepth(vec3(texc, z), InverseProjectionMatrix);
    vec3 eyedir = -normalize(pixelpos.xyz);

    vec3 farthestpoint = - eyedir * (min(dot(-eyedir, light_pos) + radius, length(pixelpos.xyz)));
    vec3 closestpoint = - eyedir * (dot(-eyedir, light_pos) - radius);
    if (closestpoint.z < 1.) closestpoint = vec3(0.);

    float stepsize = length(farthestpoint - closestpoint) / 16.;
    vec3 fog = vec3(0.);
    vec3 xpos = farthestpoint;

    for (int i = 0; i < 16; i++)
    {
        float d = distance(light_pos, xpos);
        float l = (16. - float(i)) * stepsize;
        float att = energy * 20. / (1. + d * d);
        att *= max((radius - d) / radius, 0.);
        fog += density * light_col * att * exp(- density * d) * exp(- density * l) * stepsize;
        xpos += stepsize * eyedir;
    }

    Fog = vec4(fogcol * fog, 0.);
}
