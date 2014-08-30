uniform vec3 color_from;
uniform vec3 color_to;

layout(location=0) in vec3 Position;
layout(location = 1) in float lifetime;
layout(location = 2) in float size;

layout(location=3) in vec2 Texcoord;
layout(location = 4) in vec2 quadcorner;

out float lf;
out vec2 tc;
out vec3 pc;

void main(void)
{
    tc = Texcoord;
    lf = lifetime;
    pc = color_from + (color_to - color_from) * lifetime;
    vec3 newposition = Position;

    vec4 viewpos = ViewMatrix * vec4(newposition, 1.0);
    viewpos += size * vec4(quadcorner, 0., 0.);
    gl_Position = ProjectionMatrix * viewpos;
}
