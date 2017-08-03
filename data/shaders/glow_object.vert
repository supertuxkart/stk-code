#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
layout(location = 10) in vec4 misc_data;
#else
in vec3 Position;
in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
in vec4 misc_data;
#endif
flat out vec4 glowColor;

#stk_include "utils/getworldmatrix.vert"

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation, Scale) * InverseViewMatrix);
    gl_Position = ProjectionViewMatrix * ModelMatrix * vec4(Position, 1.);
    glowColor = misc_data;
}
