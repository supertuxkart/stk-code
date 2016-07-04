#ifdef GL_ES
uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform mat4 TextureMatrix;
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

uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);
#endif

#if __VERSION__ >= 330
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


void main(void)
{
    color = Color.zyxw;
    mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    mat4 TransposeInverseModelView = transpose(InverseModelMatrix * InverseViewMatrix);
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    // Keep orthogonality
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * vec4(Tangent, 0.)).xyz;
    bitangent = (ViewMatrix * ModelMatrix * vec4(Bitangent, 0.)).xyz;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    uv_bis = SecondTexcoord;
}
