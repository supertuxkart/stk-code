in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;

in vec3 Position;
in vec2 Texcoord;

out vec2 tc;

mat4 getWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);
mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation, vec3 scale);

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    gl_Position = ModelMatrix * vec4(Position, 1.);
    tc = Texcoord;
}