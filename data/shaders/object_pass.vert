uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform mat4 TextureMatrix;

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
    mat4 DefaultMatrix = mat4(1., 0., 0., 0.,
                              0., 1., 0., 0.,
                              0., 0., 1., 0.,
                              0., 0., 0., 1.);

    mat4 ModelMatrixCurr = ModelMatrix;
    mat4 InverseModelMatrixCurr = InverseModelMatrix;
    mat4 TextureMatrixCurr = TextureMatrix;

    if (ModelMatrixCurr == mat4(0.0))
        ModelMatrixCurr = DefaultMatrix;

    if (InverseModelMatrixCurr == mat4(0.0))
        InverseModelMatrixCurr = DefaultMatrix;

    if (TextureMatrixCurr == mat4(0.0))
        TextureMatrixCurr = DefaultMatrix;

    color = Color.zyxw;
    mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrixCurr;
    mat4 TransposeInverseModelView = transpose(InverseModelMatrixCurr * InverseViewMatrix);
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    // Keep orthogonality
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrixCurr * vec4(Tangent, 0.)).xyz;
    bitangent = (ViewMatrix * ModelMatrixCurr * vec4(Bitangent, 0.)).xyz;
    uv = (TextureMatrixCurr * vec4(Texcoord, 1., 1.)).xy;
    uv_bis = SecondTexcoord;
}
