uniform mat4 ModelMatrix;
uniform mat4 RSMMatrix;
uniform vec2 texture_trans = vec2(0., 0.);

#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 SecondTexcoord;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;
in vec2 SecondTexcoord;
#endif

out vec3 nor;
out vec2 uv;
out vec2 uv_bis;
out vec4 color;


void main(void)
{
    mat4 ModelViewProjectionMatrix = RSMMatrix * ModelMatrix;
    mat4 TransposeInverseModel = transpose(inverse(ModelMatrix));
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModel * vec4(Normal, 0.)).xyz;
    uv = vec2(Texcoord.x + texture_trans.x, Texcoord.y + texture_trans.y);
    uv_bis = SecondTexcoord;
    color = Color.zyxw;
}
