uniform vec3 color_from;
uniform vec3 color_to;
uniform int flips;

#ifdef Explicit_Attrib_Location_Usable

layout(location = 0) in vec3 Position;
layout(location = 1) in float lifetime;
layout(location = 2) in float size;

layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 quadcorner;

layout(location = 5) in vec3 rotationvec;
layout(location = 6) in float anglespeed;
#else

in vec3 Position;
in float lifetime;
in float size;

in vec2 Texcoord;
in vec2 quadcorner;

in vec3 rotationvec;
float anglespeed;
#endif

out float lf;
out vec2 tc;
out vec4 pc;

void main(void)
{
    if (size == 0.0)
    {
        return;
    }
    tc = Texcoord;
    lf = lifetime;
    pc = vec4(vec3(color_from + (color_to - color_from) * lf), 1.0) * smoothstep(1., 0.8, lf);
#if !defined(sRGB_Framebuffer_Usable) && !defined(Advanced_Lighting_Enabled)
    pc.rgb = pow(pc.rgb, vec3(1. / 2.2));
#endif
    vec4 viewpos = vec4(0.);
    if (flips == 1)
    {
        // from http://jeux.developpez.com/faq/math
        float angle = lifetime * anglespeed;
        float sin_a = sin(angle / 2.);
        float cos_a = cos(angle / 2.);

        vec4 quaternion = normalize(vec4(rotationvec * sin_a, cos_a));
        float xx = quaternion.x * quaternion.x;
        float xy = quaternion.x * quaternion.y;
        float xz = quaternion.x * quaternion.z;
        float xw = quaternion.x * quaternion.w;
        float yy = quaternion.y * quaternion.y;
        float yz = quaternion.y * quaternion.z;
        float yw = quaternion.y * quaternion.w;
        float zz = quaternion.z * quaternion.z;
        float zw = quaternion.z * quaternion.w;

        vec4 col1 = vec4(
            1. - 2. * ( yy + zz ),
            2. * ( xy + zw ),
            2. * ( xz - yw ),
            0.);
        vec4 col2 = vec4(
            2. * ( xy - zw ),
            1. - 2. * ( xx + zz ),
            2. * ( yz + xw ),
            0.);
        vec4 col3 = vec4(
            2. * ( xz + yw ),
            2. * ( yz - xw ),
            1. - 2. * ( xx + yy ),
            0.);
        vec4 col4 = vec4(0., 0., 0., 1.);
        mat4 rotationMatrix = mat4(col1, col2, col3, col4);
        vec3 newquadcorner = size * vec3(quadcorner, 0.);
        newquadcorner = (rotationMatrix * vec4(newquadcorner, 0.)).xyz;
        viewpos = ViewMatrix * vec4(Position + newquadcorner, 1.0);
    }
    else
    {
        viewpos = ViewMatrix * vec4(Position, 1.0);
        viewpos += size * vec4(quadcorner, 0., 0.);
    }
    gl_Position = ProjectionMatrix * viewpos;
}
