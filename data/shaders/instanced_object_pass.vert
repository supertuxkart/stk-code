#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 SecondTexcoord;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;

layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
#ifdef Use_Bindless_Texture
layout(location = 10) in sampler2D Handle;
layout(location = 11) in sampler2D SecondHandle;
layout(location = 13) in sampler2D ThirdHandle;
#endif

#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;
in vec2 SecondTexcoord;
in vec3 Tangent;
in vec3 Bitangent;

in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
#endif

out vec3 nor;
out vec3 tangent;
out vec3 bitangent;
out vec2 uv;
out vec2 uv_bis;
out vec4 color;
#ifdef Use_Bindless_Texture
flat out sampler2D handle;
flat out sampler2D secondhandle;
flat out sampler2D thirdhandle;
#endif

#stk_include "utils/getworldmatrix.vert"

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation, Scale) * InverseViewMatrix);
    gl_Position = ProjectionViewMatrix *  ModelMatrix * vec4(Position, 1.);
    // Keep orthogonality
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * vec4(Tangent, 0.)).xyz;
    bitangent = (ViewMatrix * ModelMatrix * vec4(Bitangent, 0.)).xyz;
    uv = Texcoord;
    uv_bis = SecondTexcoord;
    color = Color.zyxw;
#ifdef Use_Bindless_Texture
    handle = Handle;
    secondhandle = SecondHandle;
    thirdhandle = ThirdHandle;
#endif
}
