uniform int flips;
uniform int sky;
uniform vec3 view_position;
uniform float billboard;

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

vec4 getQuat(float half_sin, float half_cos)
{
    return normalize(vec4(vec3(0.0, 1.0, 0.0) * half_sin, half_cos));
}

void main(void)
{
    if (size.x == 0.0 && size.y == 0.0)
    {
        gl_Position = vec4(0.);
        pc = vec4(0.0);
        tc = vec2(0.0);
        return;
    }

    float lifetime = size.y;
    vec2 particle_size = mix(size.xx, size, billboard);
    tc = Texcoord;
    pc = color_lifetime.zyxw;

    vec4 viewpos = vec4(0.);
    if (flips == 1 || sky == 1)
    {
        vec4 quat = vec4(0.0);
        if (flips == 1)
        {
            float angle = lifetime * anglespeed;
            float sin_a = sin(mod(angle / 2.0, 6.283185307179586));
            float cos_a = cos(mod(angle / 2.0, 6.283185307179586));
            quat = getQuat(sin_a, cos_a);
        }
        else
        {
            vec3 diff = Position - view_position;
            float angle = atan(diff.x, diff.z);
            quat = getQuat(sin(angle / -2.0), cos(angle / -2.0));
        }
        vec3 newquadcorner = vec3(particle_size * quadcorner, 0.0);
        newquadcorner = newquadcorner + 2.0 * cross(cross(newquadcorner,
            quat.xyz) + quat.w * newquadcorner, quat.xyz);
        viewpos = u_view_matrix * vec4(Position + newquadcorner, 1.0);
    }
    else
    {
        viewpos = u_view_matrix * vec4(Position, 1.0);
        viewpos += vec4(particle_size * quadcorner, 0.0, 0.0);
    }
    gl_Position = u_projection_matrix * viewpos;
}
