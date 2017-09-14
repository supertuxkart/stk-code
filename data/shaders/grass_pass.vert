uniform vec3 windDir;
uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;

#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;
#endif

out vec3 nor;
out vec2 uv;

void main()
{

    vec3 test = sin(windDir * (Position.y * 0.1)) * 1.;
    test += cos(windDir) * 0.7;

    mat4 new_model_matrix = ModelMatrix;
    mat4 new_inverse_model_matrix = InverseModelMatrix;
    new_model_matrix[3].xyz += test * Color.r;

    // FIXME doesn't seem to make too much difference in pass 2, because this
    // affects "nor" which is later only * 0.1 by scattering
    new_inverse_model_matrix[3].xyz -= test * Color.r;

    mat4 ModelViewProjectionMatrix = ProjectionViewMatrix * new_model_matrix;
    mat4 TransposeInverseModelView = transpose(InverseViewMatrix * new_inverse_model_matrix);
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
}
