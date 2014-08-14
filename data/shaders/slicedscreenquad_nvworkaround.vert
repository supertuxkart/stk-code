uniform int slice;

in vec2 Position;
in vec2 Texcoord;

out int layer;

void main() {
    layer = slice;
    gl_Position = vec4(Position, 0., 1.);
}
