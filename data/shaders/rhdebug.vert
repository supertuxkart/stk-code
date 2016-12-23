uniform vec3 extents;

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
    gl_Position = ProjectionViewMatrix * RHMatrix * vec4(WorldPos, 1.);
    gl_PointSize = 500. / gl_Position.w;

}