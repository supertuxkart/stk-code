uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

in vec3 Position;
in float Energy;
in vec3 Color;

in vec2 Corner;

flat out vec3 center;
flat out float energy;
flat out vec3 col;

const float zNear = 1.;

void main(void)
{
    // Beyond that value, light is too attenuated
    float r = 100 * Energy;
    center = Position;
    energy = Energy;
    vec4 Center = ViewMatrix * vec4(Position, 1.);
    vec4 ProjectedCornerPosition = ProjectionMatrix * (Center + r * vec4(Corner, 0., 0.));
    float adjustedDepth = ProjectedCornerPosition.z;
    if (Center.z > zNear) // Light is in front of the cam
    {
        adjustedDepth = max(Center.z - r, zNear);
    }
    else if (Center.z + r > zNear) // Light is behind the cam but in range
    {
        adjustedDepth = zNear;
    }

    ProjectedCornerPosition /= ProjectedCornerPosition.w;
    ProjectedCornerPosition.zw = (ProjectionMatrix * vec4(0., 0., adjustedDepth, 1.)).zw;
    ProjectedCornerPosition.xy *= ProjectedCornerPosition.w;
    col = Color;
    gl_Position = ProjectedCornerPosition;
}
