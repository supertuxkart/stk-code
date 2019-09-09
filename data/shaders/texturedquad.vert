uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;
uniform float rotation;
#ifdef Explicit_Attrib_Location_Usable
layout(location=0) in vec2 Position;
layout(location=3) in vec2 Texcoord;
#else
in vec2 Position;
in vec2 Texcoord;
#endif

out vec2 uv;

void main()
{
    float s = sin(rotation);
    float c = cos(rotation);
    mat2 m = mat2(c, -s, s, c);
    uv = Texcoord * texsize + texcenter;
    gl_Position = vec4(m * Position * size + center, 0., 1.) ;
}
