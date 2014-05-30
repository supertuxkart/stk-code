uniform vec3 extents;

layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

uniform mat4 RHMatrix;

ivec3 resolution = ivec3(32, 16, 32);

out vec3 uvw;

void main(void)
{
    // Determine the RH center
    float   gx = int(gl_VertexID) & (resolution.x - 1);
    float   gy = int(gl_VertexID >> 5) & (resolution.y - 1);
    float   gz = int(gl_VertexID >> 9) & (resolution.z - 1);
    uvw = vec3(gx, gy, gz) / vec3(resolution);
    vec3 WorldPos = (2. * uvw - 1.) * extents;
    gl_Position = ProjectionMatrix * ViewMatrix * RHMatrix * vec4(WorldPos, 1.);
    gl_PointSize = 100. / gl_Position.w;

}