in vec2 Position;
in vec2 Texcoord;

#ifndef VSLayer
out int layer;
#else
flat out int slice;
#endif

void main() {
#ifdef VSLayer
    gl_Layer = gl_InstanceID;
    slice = gl_InstanceID;
#else
    layer = gl_InstanceID;
#endif
    gl_Position = vec4(Position, 0., 1.);
}
