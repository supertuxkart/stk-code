uniform mat4 ViewProjectionMatrix;
uniform mat4 InverseViewMatrix;

in vec3 Origin;
in vec3 Orientation;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;

out vec3 nor;
out vec2 uv;

mat4 getWorldMatrix(vec3 translation, vec3 rotation);
mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation);

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation) * InverseViewMatrix);
    gl_Position = ViewProjectionMatrix *  ModelMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
}
