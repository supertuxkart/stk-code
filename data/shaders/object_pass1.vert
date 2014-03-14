uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

#if __VERSION__ >= 130
in vec3 Position;
in vec3 Normal;
out vec3 nor;
#else
attribute vec3 Position;
attribute vec3 Normal;
varying vec3 nor;
#endif


void main(void)
{
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
}
