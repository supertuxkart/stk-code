uniform vec3 windDir;

#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;

layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
#ifdef GL_ARB_bindless_texture
layout(location = 10) in sampler2D Handle;
layout(location = 11) in sampler2D SecondHandle;
#endif

#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;

in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
#endif

out vec3 nor;
out vec2 uv;
#ifdef GL_ARB_bindless_texture
flat out sampler2D handle;
flat out sampler2D secondhandle;
#endif

mat4 getWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);
mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);

void main()
{
    mat4 ModelMatrix = getWorldMatrix(Origin + windDir * Color.r, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin + windDir * Color.r, Orientation, Scale) * InverseViewMatrix);
    gl_Position = ProjectionViewMatrix *  ModelMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
#ifdef GL_ARB_bindless_texture
    handle = Handle;
    secondhandle = SecondHandle;
#endif
}
