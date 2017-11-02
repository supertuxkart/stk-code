uniform int flips;

#ifdef Explicit_Attrib_Location_Usable

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 color_lifetime;
layout(location = 2) in vec2 size;

layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 quadcorner;

layout(location = 6) in float anglespeed;
#else

in vec3 Position;
in vec4 color_lifetime;
in vec2 size;

in vec2 Texcoord;
in vec2 quadcorner;

in float anglespeed;
#endif

out vec2 tc;
out vec4 pc;

void main(void)
{
    if (size.x == 0.0 && size.y == 0.0)
    {
        gl_Position = vec4(0.);
        pc = vec4(0.0);
        tc = vec2(0.0);
        return;
    }

    float lifetime = color_lifetime.w;
    vec4 particle_color = vec4(color_lifetime.zyx, 1.0);
    tc = Texcoord;

#if !defined(sRGB_Framebuffer_Usable) && !defined(Advanced_Lighting_Enabled)
    particle_color.rgb = pow(particle_color.rgb, vec3(1.0 / 2.2));
#endif
    pc = particle_color;

    vec4 viewpos = vec4(0.);
    if (flips == 1)
    {
        float angle = lifetime * anglespeed;
        float sin_a = sin(mod(angle / 2.0, 6.283185307179586));
        float cos_a = cos(mod(angle / 2.0, 6.283185307179586));
        vec4 quat = normalize(vec4(vec3(0.0, 1.0, 0.0) * sin_a, cos_a));
        vec3 newquadcorner = vec3(size * quadcorner, 0.0);
        newquadcorner = newquadcorner + 2.0 * cross(cross(newquadcorner,
            quat.xyz) + quat.w * newquadcorner, quat.xyz);
        viewpos = ViewMatrix * vec4(Position + newquadcorner, 1.0);
    }
    else
    {
        viewpos = ViewMatrix * vec4(Position, 1.0);
        viewpos += vec4(size * quadcorner, 0.0, 0.0);
    }
    gl_Position = ProjectionMatrix * viewpos;
}
