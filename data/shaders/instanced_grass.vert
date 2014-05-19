layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

uniform vec3 windDir;


in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;

out vec3 nor;
out vec2 uv;

mat4 getWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);
mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);

void main()
{
    mat4 ModelMatrix = getWorldMatrix(Origin + windDir * Color.r, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin + windDir * Color.r, Orientation, Scale) * InverseViewMatrix);
    gl_Position = ProjectionMatrix * ViewMatrix *  ModelMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
}
