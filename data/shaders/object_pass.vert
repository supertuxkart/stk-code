#ifdef GL_ES
uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform vec2 texture_trans;
#else
uniform mat4 ModelMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);
uniform mat4 InverseModelMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);

uniform vec2 texture_trans = vec2(0., 0.);
#endif

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

out vec3 nor;
out vec3 tangent;
out vec3 bitangent;
out vec2 uv;
out vec2 uv_bis;
out vec4 color;
out float camdist;

void main(void)
{
    color = Color.zyxw;
    mat4 ModelViewProjectionMatrix = ProjectionViewMatrix * ModelMatrix;
    mat4 TransposeInverseModelView = transpose(InverseModelMatrix * InverseViewMatrix);
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    // Keep orthogonality
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * vec4(Tangent, 0.)).xyz;
    bitangent = (ViewMatrix * ModelMatrix * vec4(Bitangent, 0.)).xyz;
    uv = vec2(Texcoord.x + texture_trans.x, Texcoord.y + texture_trans.y);
    uv_bis = SecondTexcoord;
    camdist = length(ViewMatrix * ModelMatrix * vec4(Position, 1.));
}
