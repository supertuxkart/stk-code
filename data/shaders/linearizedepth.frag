uniform sampler2D tex;
uniform float zn;
uniform float zf;

layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

out float Depth;

void main()
{
    vec2 uv = gl_FragCoord.xy / screen;
    float d = texture(tex, uv).x;
    float c0 = zn * zf, c1 = zn - zf, c2 = zf;
    Depth = c0 / (d * c1 + c2);
}
