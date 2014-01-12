#version 130
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

in vec3 Position;
in vec2 Texcoord;
in vec3 Normal;
out vec2 uv;
noperspective out vec3 nor;

void main(void)
{
    uv = Texcoord;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
}
