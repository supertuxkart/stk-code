#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 SecondTexcoord;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;
in vec2 SecondTexcoord;
in vec3 Tangent;
in vec3 Bitangent;
#endif

out vec2 uv;
out vec4 color;

void main(void)
{
    color = Color.zyxw;
    vec3 P = Position / vec3(screen, 1.);
    P = 2. * P - 1.;
    P.y *= -1.;
    gl_Position = vec4(P, 1.);
    uv = Texcoord;
}
