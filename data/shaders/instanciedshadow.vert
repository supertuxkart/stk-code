layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};

in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;

in vec3 Position;
in vec2 Texcoord;

out vec2 tc;
out int layerId;

mat4 getWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);
mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);

void main(void)
{
    layerId = gl_InstanceID & 3;
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position, 1.);
    tc = Texcoord;
}